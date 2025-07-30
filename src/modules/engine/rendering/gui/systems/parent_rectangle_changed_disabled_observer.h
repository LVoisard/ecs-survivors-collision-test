﻿//
// Created by Laurent on 7/21/2025.
//

#ifndef ON_PARENT_RECTANGLE_CHANGED_DISABLED_OBSERVER_H
#define ON_PARENT_RECTANGLE_CHANGED_DISABLED_OBSERVER_H

#include <flecs.h>
#include <raylib.h>

#include "parent_rectangle_changed_observer.h"

namespace rendering::gui::systems {
    inline void on_parent_rectangle_changed_disabled_observer(flecs::entity e, const Rectangle &rec) {
        // some rly wierd shenanigans with disabled entities and observers i think
        systems::on_parent_rectangle_changed_observer(e, rec);
        e.children([&](flecs::entity c) { systems::on_parent_rectangle_changed_observer(c, *(e.get<Rectangle>()));
        });
    }
}
#endif //ON_PARENT_RECTANGLE_CHANGED_DISABLED_OBSERVER_H
