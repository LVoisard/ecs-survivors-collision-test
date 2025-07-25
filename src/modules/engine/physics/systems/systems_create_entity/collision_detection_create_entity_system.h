//
// Created by Laurent on 7/24/2025.
//

#ifndef COLLISION_DETECTION_CREATE_ENTITY_SYSTEM_H
#define COLLISION_DETECTION_CREATE_ENTITY_SYSTEM_H
#include <flecs.h>
#include <raylib.h>
#include <vector>

#include "modules/engine/physics/components.h"
#include "modules/engine/core/components.h"
#include "modules/engine/physics/collision_helper.h"

namespace physics::systems {
    inline void collision_detection_non_static_entity_system(flecs::world world, flecs::iter &self_it, size_t self_id,
                                                             const core::Position2D &pos, const Collider &collider) {

        // Build a staged query, and filter
        auto visible_query = world.query_builder<const core::Position2D, const Collider>().with<rendering::Visible>().filter();
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

            CollisionInfo a_info;
            CollisionInfo b_info;
            if (!collision_handler[collider.type][other_collider.type](self, collider, a_info, other, other_collider,
                                                                       b_info)) return;

            world.entity().set<CollisionRecord>({self, other});

        });
    }
}

#endif //COLLISION_DETECTION_CREATE_ENTITY_SYSTEM_H
