//
// Created by laurent on 29/05/25.
//

#ifndef COLLISION_RESOLUTION_ENTITY_SYSTEM_H
#define COLLISION_RESOLUTION_ENTITY_SYSTEM_H

#include <flecs.h>
#include "../../collision_helper.h"
#include "modules/engine/physics/components.h"
#include "modules/gameplay/components.h"

namespace physics::systems {


    inline void collision_resolution_entity_system(CollisionRecord& rec) {
        // looping helps with stability
        flecs::entity a = rec.a;
        flecs::entity b = rec.b; // Colliding entity

        const Collider a_col = *(a.get<Collider>());
        const Collider b_col = *(b.get<Collider>());

        // are the entities colliding?
        correct_positions(a, a_col, a_info, )

        // if the entities are of different types (player & enemy) we report it a significant collision
        // enemy vs environment should not be significant. (too many tables)
        // But player vs environment should count (because of projectiles, they might have behaviours specific to
        // obstacles)
        if (((a_col.collision_type & b_col.collision_type) == none &&
            (a_col.collision_type | b_col.collision_type) != (enemy | environment))) {
            a.add<CollidedWith>(b);
            b.add<CollidedWith>(a);
        }
    }
} // namespace physics::systems
#endif // COLLISION_RESOLUTION_ENTITY_SYSTEM_H
