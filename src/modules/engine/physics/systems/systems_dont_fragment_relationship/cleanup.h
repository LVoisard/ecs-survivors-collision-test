//
// Created by Laurent on 7/24/2025.
//

#ifndef CLEANUP_DONT_FRAG_REL_H
#define CLEANUP_DONT_FRAG_REL_H

#include <flecs.h>
#include "modules/engine/physics/components.h"
namespace physics::systems {
    inline void collision_cleanup_non_frag_system(flecs::iter& it, size_t i) {
        it.entity(i).remove<NonFragmentingCollidedWith>(it.pair(0));
    }
}
#endif //CLEANUP_DONT_FRAG_REL_H
