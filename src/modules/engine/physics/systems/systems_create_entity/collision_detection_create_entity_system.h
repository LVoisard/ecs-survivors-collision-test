//
// Created by Laurent on 7/24/2025.
//

#ifndef COLLISION_DETECTION_CREATE_ENTITY_SYSTEM_H
#define COLLISION_DETECTION_CREATE_ENTITY_SYSTEM_H
#include <flecs.h>
#include <raylib.h>
#include <vector>

#include "modules/engine/core/components.h"
#include "modules/engine/physics/collision_helper.h"
#include "modules/engine/physics/components.h"

namespace physics::systems {
    inline void collision_detection_non_static_entity_system(flecs::world world, flecs::iter &self_it, size_t self_id,
                                                             const core::Position2D &pos, const Collider &collider) {

        // Build a staged query, and filter
        auto visible_query =
                world.query_builder<const core::Position2D, const Collider>().with<rendering::Visible>().filter();
        flecs::entity self = self_it.entity(self_id);

        visible_query.each([&](flecs::iter &other_it, size_t other_id, const core::Position2D &other_pos,
                               const Collider &other_collider) {
            flecs::entity other = other_it.entity(other_id);
            if (other.id() <= self.id())
                return;

            if ((collider.collision_filter & other_collider.collision_type) == none)
                return;

            CollisionInfo a;
            CollisionInfo b;
            if (collision_handler[collider.type][other_collider.type](self, collider, a, other, collider, b)) {
                world.entity().set<CollisionRecord>({self, other});
            }
        });
    }
} // namespace physics::systems

#endif // COLLISION_DETECTION_CREATE_ENTITY_SYSTEM_H
