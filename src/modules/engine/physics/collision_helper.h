//
// Created by Laurent on 7/24/2025.
//

#ifndef COLLISION_HELPER_H
#define COLLISION_HELPER_H

#include <flecs.h>
#include <raylib.h>
#include <raymath.h>
#include <functional>

#include "components.h"
#include "modules/engine/core/components.h"


namespace physics {
    static std::mutex list_mutex;
    static Vector2 collide_circles(const CircleCollider &a, const core::Position2D &a_pos, CollisionInfo& a_info,
                                       const CircleCollider &b, const core::Position2D &b_pos, CollisionInfo& b_info) {
            float combinedRadius = a.radius + b.radius;

            // Find the distance and adjust to resolve the overlap
            Vector2 direction = b_pos.value - a_pos.value;
            Vector2 moveDirection = Vector2Normalize(direction);
            float overlap = combinedRadius - Vector2Length(direction);

            a_info.normal = Vector2Negate(moveDirection);
            b_info.normal = moveDirection;

            return moveDirection * overlap;
        }

        static Vector2 collide_circle_rec(const CircleCollider &a, core::Position2D &a_pos, CollisionInfo& a_info,
                                          const Collider &b, core::Position2D &b_pos, CollisionInfo& b_info) {
            float recCenterX = b_pos.value.x + b.bounds.x + b.bounds.width / 2.0f;
            float recCenterY = b_pos.value.y + b.bounds.y + b.bounds.height / 2.0f;

            float halfWidth = b.bounds.width / 2.0f;
            float halfHeight = b.bounds.height / 2.0f;

            float dx = a_pos.value.x - recCenterX;
            float dy = a_pos.value.y - recCenterY;

            float absDx = fabsf(dx);
            float absDy = fabsf(dy);

            Vector2 overlap = {0, 0};

            if (absDx > (halfWidth + a.radius)) return overlap;
            if (absDy > (halfHeight + a.radius)) return overlap;

            if (absDx <= halfWidth || absDy <= halfHeight) {
                // Side collision — resolve with axis-aligned MTV
                float overlapX = (halfWidth + a.radius) - absDx;
                float overlapY = (halfHeight + a.radius) - absDy;


                if (overlapX < overlapY) {
                    overlap.x = dx < 0 ? overlapX : -overlapX;
                } else {
                    overlap.y = dy < 0 ? overlapY : -overlapY;
                }
                a_info.normal = Vector2Normalize(Vector2Negate(overlap));
                b_info.normal = Vector2Normalize(overlap);
                return overlap;
            }

            // Corner collision
            float cornerDx = absDx - halfWidth;
            float cornerDy = absDy - halfHeight;

            float cornerDistSq = cornerDx * cornerDx + cornerDy * cornerDy;
            float radius = a.radius;

            if (cornerDistSq < radius * radius) {
                float dist = sqrtf(cornerDistSq);

                if (dist == 0.0f) dist = 0.01f; // Avoid divide by zero

                float overlap_length = radius - dist;
                float nx = cornerDx / dist;
                float ny = cornerDy / dist;

                overlap = {
                    nx * overlap_length * ((dx < 0) ? 1.0f : -1.0f),
                    ny * overlap_length * ((dy < 0) ? 1.0f : -1.0f)
                };

                a_info.normal = Vector2Normalize(Vector2Negate(overlap));
                b_info.normal = Vector2Normalize(overlap);
            }

            return overlap;
        }
        /**
     * Correct the positions of the entity (if they are not static nor "triggers") by the overlap amount
     * @param a entity 1
     * @param a_col entity 1 mutable position
     * @param b entity 2
     * @param b_col entity 2 mutable position
     * @param overlap amount of overlap of the two entities
     */
    static void correct_positions(flecs::entity &a, const Collider &a_col, CollisionInfo &a_info, flecs::entity &b,
                                  const Collider &b_col, CollisionInfo &b_info, Vector2 overlap) {
        const core::Position2D& a_pos = a.get<core::Position2D>();
        const core::Position2D& b_pos = b.get<core::Position2D>();

        float a_move_ratio = 0.5f;
        float b_move_ratio = 0.5f;

        if (a_col.static_body) {
            a_move_ratio = 0;
            b_move_ratio = 1.0f;
        }
        if (b_col.static_body) {
            a_move_ratio = 1.0f;
            b_move_ratio = 0;
        }

        if ((!a_col.correct_position && !a_col.static_body) || (!b_col.correct_position && !b_col.static_body)) {
            a_move_ratio = 0.0f;
            b_move_ratio = 0.0f;
        }

        a.set<core::Position2D>({a_pos.value - overlap * a_move_ratio * 0.75});
        b.set<core::Position2D>({b_pos.value + overlap * b_move_ratio * 0.75});
    }

    /**
     * Resolve Circle vs Circle collision
     * @param a circle 1 entity
     * @param base_col circle 1 collider
     * @param b circle 2 entity
     * @param other_base_col circle 2 entity
     * @return if the circles collided
     */
    static bool handle_circle_circle(flecs::entity &a, const Collider &a_col, CollisionInfo &a_info, flecs::entity &b,
                                     const Collider &b_col, CollisionInfo &b_info) {
        Vector2 a_pos = a.get<core::Position2D>().value;
        Vector2 b_pos = b.get<core::Position2D>().value;

        const CircleCollider col = a.get<CircleCollider>();
        const CircleCollider other_col = b.get<CircleCollider>();

        if (!CheckCollisionCircles(a_pos, col.radius, b_pos, other_col.radius)) {
            return false;
        }

        Vector2 overlap = collide_circles(col, a.get<core::Position2D>(), a_info, other_col,
                                                         b.get<core::Position2D>(), b_info);

        correct_positions(a, a_col, a_info, b, b_col, b_info, overlap);

        Vector2 contact_point = a_pos + b_info.normal * col.radius;
        a_info.contact_point = contact_point;
        b_info.contact_point = contact_point;

        return true;
    }

    /**
     * Resolve circle vs rectangle collision
     * @param a circle entity
     * @param a_col circle base collider
     * @param b box entity
     * @param b_col box base collider
     * @return if there was a collision
     */
    static bool handle_circle_rec_collision(flecs::entity &a, const Collider &a_col, CollisionInfo &a_info,
                                            flecs::entity &b, const Collider &b_col, CollisionInfo &b_info) {
        Vector2 a_pos = a.get<core::Position2D>().value;
        Vector2 b_pos = b.get<core::Position2D>().value;
        const CircleCollider circle_col = a.get<CircleCollider>();
        if (!CheckCollisionCircleRec(
            a_pos, circle_col.radius, {
                b_pos.x + b_col.bounds.x, b_pos.y + b_col.bounds.y, b_col.bounds.width,
                b_col.bounds.height
            })) {
            return false;
        }

        Vector2 overlap = collide_circle_rec(
            circle_col, a.get_mut<core::Position2D>(), a_info,
            b_col, b.get_mut<core::Position2D>(), b_info);

        correct_positions(a, a_col, a_info, b, b_col, b_info, overlap);

        Vector2 contact_point = a_pos + b_info.normal * circle_col.radius;
        a_info.contact_point = contact_point;
        b_info.contact_point = contact_point;

        return true;
    }

    /**
     * Resolve rectangle vs rectangle collision
     * @param a box 1 entity
     * @param a_col box 1 collider
     * @param b box 2 entity
     * @param b_col box 2 collider
     * @return if the boxes collided or not
     */
    static bool handle_boxes_collision(flecs::entity &a, const Collider &a_col, flecs::entity &b,
                                       const Collider &b_col) {
        Vector2 a_pos = a.get<core::Position2D>().value;
        Vector2 b_pos = b.get<core::Position2D>().value;
        if (!CheckCollisionRecs(
            {
                a_pos.x + a_col.bounds.x, a_pos.y + a_col.bounds.y, a_col.bounds.width,
                a_col.bounds.height
            }, {
                b_pos.x + b_col.bounds.x, b_pos.y + b_col.bounds.y, b_col.bounds.width,
                b_col.bounds.height
            })) {
            return false;
        }

        // TODO need to figure overlap for boxes and update the positions properly
        return true;
    }

    using CollisionHandler = std::function<bool(flecs::entity &, const Collider &, CollisionInfo &, flecs::entity &,
                                                const Collider &, CollisionInfo &)>;

    /**
     * Map for collision type handling, we point to the correct position in the array with the Collider type enum
     */
    static CollisionHandler collision_handler[ColliderType::SIZE][ColliderType::SIZE] = {
        // Circle = 0
        {
            // Circle vs Circle
            [](flecs::entity &a, const Collider &a_col, CollisionInfo &a_info, flecs::entity &b,
               const Collider &b_col, CollisionInfo &b_info) {
                return handle_circle_circle(a, a_col, a_info, b, b_col, b_info);
            },
            // Circle vs Box
            [](flecs::entity &a, const Collider &a_col, CollisionInfo &a_info, flecs::entity &b,
               const Collider &b_col, CollisionInfo &b_info) {
                return handle_circle_rec_collision(a, a_col, a_info, b, b_col, b_info);
            },
        },
        // Box = 1
        {
            // Box vs Circle
            [](flecs::entity &a, const Collider &a_col, CollisionInfo &a_info, flecs::entity &b,
               const Collider &b_col, CollisionInfo &b_info) {
                return handle_circle_rec_collision(b, b_col, b_info, a, a_col, a_info);
            },
            // Box vs Box
            [](flecs::entity &a, const Collider &a_col, CollisionInfo &a_info, flecs::entity &b,
               const Collider &b_col, CollisionInfo &b_info) {
                return handle_boxes_collision(a, a_col, b, b_col);
            },
        }
        // more collisions
    };
}
#endif //COLLISION_HELPER_H
