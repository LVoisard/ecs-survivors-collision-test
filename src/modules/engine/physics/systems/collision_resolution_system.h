//
// Created by laurent on 29/05/25.
//

#ifndef COLLISION_RESOLUTION_SYSTEM_H
#define COLLISION_RESOLUTION_SYSTEM_H

#include <flecs.h>
#include "modules/engine/physics/components.h"
#include "modules/gameplay/components.h"
#include "../collision_helper.h"

namespace physics::systems {


    inline void collision_resolution_rec_list_system(CollisionRecordList &rec) {
        // looping helps with stability
        for (int i = 0; i < 3; i++) {
            for (auto &record: rec.records) {
                flecs::entity a = record.a; // Current entity
                flecs::entity b = record.b; // Colliding entity

                const Collider a_col = a.get<Collider>();
                const Collider b_col = b.get<Collider>();

                // are the entities colliding?
                CollisionInfo a_info;
                CollisionInfo b_info;
                if (!collision_handler[a_col.type][b_col.type](a, a_col, a_info, b, b_col, b_info)) continue;

                // if the entities are of different types (player & enemy) we report it a significant collision
                // enemy vs environment should not be significant. (too many tables)
                // But player vs environment should count (because of projectiles, they might have behaviours specific to obstacles)
                if (i > 0) continue;
                if ((a_col.collision_type & b_col.collision_type) == none &&
                    (a_col.collision_type | b_col.collision_type) != (enemy | environment)) {
                    rec.significant_collisions.push_back({a, b, a_info, b_info});
                }
            }
        }
    }
}
#endif //COLLISION_RESOLUTION_SYSTEM_H
