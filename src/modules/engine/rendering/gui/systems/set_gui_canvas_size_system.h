//
// Created by laurent on 29/05/25.
//

#ifndef SET_GUI_CANVAS_SIZE_SYSTEM_H
#define SET_GUI_CANVAS_SIZE_SYSTEM_H

#include "modules/engine/core/components.h"
#include <flecs.h>
#include <raylib.h>

#include "modules/engine/rendering/gui/gui_module.h"

namespace rendering::gui::systems {
    inline void set_gui_canvas_size_system(flecs::iter& it, size_t, core::GameSettings& settings) {
        get_gui_canvas(it.world()).set<Rectangle>({
            0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())
        });
        settings.window_height = GetScreenHeight();
        settings.window_width = GetScreenWidth();
    }
}
#endif //SET_GUI_CANVAS_SIZE_SYSTEM_H
