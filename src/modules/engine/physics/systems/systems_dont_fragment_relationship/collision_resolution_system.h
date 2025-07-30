//
// Created by laurent on 29/05/25.
//

#ifndef COLLISION_RESOLUTION_RESOLUTION_DONT_FRAGMENT_SYSTEM_H
#define COLLISION_RESOLUTION_RESOLUTION_DONT_FRAGMENT_SYSTEM_H

#include <flecs.h>
#include "../../collision_helper.h"
#include "modules/engine/physics/components.h"
#include "modules/gameplay/components.h"

namespace physics::systems {


    inline void collision_resolution_relationship_dont_fragment_system(flecs::iter &it, size_t i) {
        // looping helps with stability
        flecs::entity a = it.entity(i);
        flecs::entity b = it.pair(0).second(); // Colliding entity

        const Collider a_col = *(a.get<Collider>());
        const Collider b_col = *(b.get<Collider>());

        // are the entities colliding?
        CollisionInfo a_info;
        CollisionInfo b_info;
        if (!collision_handler[a_col.type][b_col.type](a, a_col, a_info, b, b_col, b_info))
            return;

        // if the entities are of different types (player & enemy) we report it a significant collision
        // enemy vs environment should not be significant. (too many tables)
        // But player vs environment should count (because of projectiles, they might have behaviours specific to
        // obstacles)
        if (!((a_col.collision_type & b_col.collision_type) == none &&
            (a_col.collision_type | b_col.collision_type) != (enemy | environment))) {
            a.remove<NonFragmentingCollidedWith>(b);
            b.remove<NonFragmentingCollidedWith>(a);
        }
    }
} // namespace physics::systems
#endif // COLLISION_RESOLUTION_RESOLUTION_DONT_FRAGMENT_SYSTEM_H
