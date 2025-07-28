//
// Created by Laurent on 7/24/2025.
//

#ifndef CLEANUP_REC_LIST_H
#define CLEANUP_REC_LIST_H

#include <flecs.h>
#include "modules/engine/physics/components.h"
namespace physics::systems {




    inline void collision_cleanup_list_system(flecs::iter& it, size_t, CollisionRecordList& list) {
        it.world().remove_all<CollidedWith>(flecs::Wildcard);
        list.records.clear();
        list.significant_collisions.clear();
        list.collisions_info.clear();
    }
}
#endif //CLEANUP_REC_LIST_H
