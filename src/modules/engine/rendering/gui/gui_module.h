//
// Created by Laurent Voisard on 12/22/2024.
//

#ifndef GUI_MODULE_H
#define GUI_MODULE_H

#include <raylib.h>
#include <raygui.h>

#include "components.h"
#include "modules/base_module.h"
#include "flecs.h"
#include "modules/engine/core/core_module.h"

namespace rendering::gui {
        static auto gui_canvas = "gui_canvas";
        static auto menu_bar = "menu_bar";
        static flecs::entity get_gui_canvas(flecs::world world) {return world.lookup(gui_canvas);}
        static flecs::entity get_menu_bar(flecs::world world) {return world.lookup(menu_bar);}
    class GUIModule : public BaseModule<GUIModule> {
    public:
        // do not add implementation to the constructor
        GUIModule(flecs::world &world): BaseModule(world) {
        };

        ~GUIModule() {
        }

        static Color font_color() { return LIGHTGRAY; }

    private:
        void register_components(flecs::world &world);

        void register_systems(flecs::world &world);

        void register_entities(flecs::world &world);


        friend class BaseModule<GUIModule>;
    };
}

#endif //GUI_MODULE_H
