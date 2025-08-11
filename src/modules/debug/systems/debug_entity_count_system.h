//
// Created by laurent on 30/05/25.
//

#ifndef DEBUG_ENTITY_COUNT_SYSTEM_H
#define DEBUG_ENTITY_COUNT_SYSTEM_H

#include <flecs.h>

#include "modules/engine/rendering/queries.h"


namespace debug::systems {
    inline void debug_entity_count_system(flecs::iter &iter) {
        DrawRectangleRec({0, 30, 225, 40}, DARKGRAY);
        DrawText(std::string(std::to_string(iter.world().query<physics::Collider>().count()) + " entities").c_str(), 10,
                 30, 20,
                 GREEN);
        auto q = iter.world().query<rendering::Renderable>();
        DrawText(
            std::string(std::to_string(q.count()) + " visible entities").
            c_str(),
            10, 50, 20, GREEN);
    }
}
#endif //DEBUG_ENTITY_COUNT_SYSTEM_H
