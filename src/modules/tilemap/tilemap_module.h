//
// Created by laurent on 03/06/25.
//

#ifndef TILEMAPMODULE_H
#define TILEMAPMODULE_H

#include <flecs.h>
#include "modules/base_module.h"

namespace tilemap {
    class TilemapModule : public BaseModule<TilemapModule> {
    public:
        TilemapModule(flecs::world &world): BaseModule(world) {
        }

    private:
        void register_components(flecs::world world);

        void register_systems(flecs::world world);

        friend class BaseModule<TilemapModule>;
    };
}


#endif //TILEMAPMODULE_H
