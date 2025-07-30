//
// Created by laurent on 30/05/25.
//

#ifndef CREATE_HEALTH_BAR_SYSTEM_H
#define CREATE_HEALTH_BAR_SYSTEM_H
#include <flecs.h>

#include "modules/engine/rendering/components.h"
#include "modules/gameplay/components.h"

namespace gameplay::systems {
    inline void create_health_bar_system(flecs::iter &it, size_t i, const gameplay::Health health) {
        it.entity(i).add<HealthBar>();
        it.world()
                .entity()
                .child_of(it.entity(i))
                .set<Rectangle>({0, 0, 50, 10})
                .set<rendering::ProgressBar>({0, health.max, health.value});
    }
} // namespace gameplay::systems
#endif // CREATE_HEALTH_BAR_SYSTEM_H
