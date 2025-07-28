//
// Created by Laurent on 7/24/2025.
//

#ifndef CLEANUP_ENTITY_H
#define CLEANUP_ENTITY_H

#include <flecs.h>
#include "modules/engine/physics/components.h"
namespace physics::systems {
    inline void collision_cleanup_entity(flecs::iter& it) {
        it.world().delete_with<CollisionRecord>();
        it.world().remove_all<CollidedWith>(flecs::Wildcard);
    }
}
#endif //CLEANUP_ENTITY_H
