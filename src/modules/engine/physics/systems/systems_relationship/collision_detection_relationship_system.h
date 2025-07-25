//
// Created by Laurent on 7/24/2025.
//

#ifndef COLLISION_DETECTION_RELATIONSHIP_H
#define COLLISION_DETECTION_RELATIONSHIP_H

#include <flecs.h>
#include <raylib.h>
#include <vector>
#include "../../components.h"
#include "modules/engine/core/components.h"
#include "modules/engine/rendering/components.h"

namespace physics::systems {
    inline void collision_detection_non_static_relationship_system(flecs::iter &self_it, size_t self_id,
                                                                       const core::Position2D &pos,
                                                                       const Collider &collider) {
        std::vector<CollisionRecord> collisions;
        std::vector<CollisionRecord> events;
        flecs::world stage_world = self_it.world();

        // Build a staged query, and filter
        auto visible_query =
                stage_world.query_builder<const core::Position2D, const Collider>().with<rendering::Visible>().filter();
        flecs::entity self = self_it.entity(self_id);

        visible_query.each([&](flecs::iter &other_it, size_t other_id, const core::Position2D &other_pos,
                               const Collider &other_collider) {
            flecs::entity other = other_it.entity(other_id);
            if (other.id() <= self.id())
                return;

            if ((collider.collision_filter & other_collider.collision_type) == none)
                return;

            Rectangle self_rec = {pos.value.x + collider.bounds.x, pos.value.y + collider.bounds.y,
                                  collider.bounds.width, collider.bounds.height};
            Rectangle other_rec = {other_pos.value.x + other_collider.bounds.x,
                                   other_pos.value.y + other_collider.bounds.y, other_collider.bounds.width,
                                   other_collider.bounds.height};

            if (CheckCollisionRecs(self_rec, other_rec)) {
                self.add<CollidedWith>(other);
                other.add<CollidedWith>(self);
            }
        });
    }
}
#endif //COLLISION_DETECTION_RELATIONSHIP_H
