//
// Created by Laurent on 7/24/2025.
//

#ifndef CLEANUP_REL_H
#define CLEANUP_REL_H

#include <flecs.h>
#include "modules/engine/physics/components.h"
namespace physics::systems {
    inline void collision_cleanup_system(flecs::iter &it, size_t i) {
        it.world().remove_all<CollidedWith>(it.entity(i));
    }
}
#endif //CLEANUP_REL_H
