//
// Created by Laurent on 7/24/2025.
//

#ifndef COLLISION_DETECTION_DONT_FRAGMENT_RELATIONSHIP_H
#define COLLISION_DETECTION_DONT_FRAGMENT_RELATIONSHIP_H

#include <flecs.h>
#include <raylib.h>
#include <vector>
#include "modules/engine/physics/components.h"
#include "modules/engine/core/components.h"
#include "modules/engine/rendering/components.h"
#include "../../collision_helper.h"

namespace physics::systems {
    inline void collision_detection_non_static_relationship_non_fragmenting_system(flecs::iter &self_it, size_t self_id,
                                                                                   const core::Position2D &pos,
                                                                                   const Collider &collider) {
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

            CollisionInfo a_info;
            CollisionInfo b_info;
            if (collision_handler[collider.type][other_collider.type](self, collider, a_info, other, other_collider,
                                                                       b_info)) {
                correct_positions(self, collider, a_info, other, other_collider, b_info);
                self.add<NonFragmentingCollidedWith>(other);
                other.add<NonFragmentingCollidedWith>(self);
            }
        });
    }
}
#endif //COLLISION_DETECTION_DONT_FRAGMENT_RELATIONSHIP_H
