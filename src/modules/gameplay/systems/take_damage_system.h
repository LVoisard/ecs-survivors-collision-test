//
// Created by laurent on 29/05/25.
//

#ifndef TAKE_DAMAGE_SYSTEM_H
#define TAKE_DAMAGE_SYSTEM_H
#include <flecs.h>

#include "modules/engine/core/components.h"
#include "modules/gameplay/components.h"

namespace gameplay::systems {
    inline void take_damage_system(flecs::entity e, Health &health, TakeDamage &dmg) {
        health.value -= dmg.damage;
        if (health.value <= 0) {
            e.add<core::DestroyAfterFrame>();
            // TODO change magic value
            e.set<GiveExperience>({dmg.giver, 1});
        }
        e.remove<TakeDamage>();
    }
}
#endif //TAKE_DAMAGE_SYSTEM_H
