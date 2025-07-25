//
// Created by laurent on 29/05/25.
//

#ifndef DRAW_PANEL_SYSTEM_H
#define DRAW_PANEL_SYSTEM_H
#include "modules/engine/rendering/gui/components.h"

namespace rendering::gui::systems {
    inline void draw_panel_system(flecs::entity e, const TexturedElement &panel, const Rectangle &rect) {
        DrawTextureNPatch(panel.texture, panel.info, rect, {0,0}, 0, ColorAlpha(BLACK, 0.8));
    }
}

#endif //DRAW_PANEL_SYSTEM_H
