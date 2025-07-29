//
// Created by laurent on 07/07/25.
//

#ifndef UPDATE_CELL_ENTITIES_RELATIONSHIP_SYSTEM_H
#define UPDATE_CELL_ENTITIES_RELATIONSHIP_SYSTEM_H

#include <cmath>
#include <flecs.h>

#include "modules/engine/core/components.h"
#include "modules/engine/physics/components.h"

namespace physics::systems {
    inline void update_cell_entities_relationship_system(flecs::entity e, SpatialHashingGrid &grid, Collider &col, core::Position2D &pos) {
        int cell_pos_x = std::floor((pos.value.x - grid.offset.x) / grid.cell_size);
        int cell_pos_y = std::floor((pos.value.y - grid.offset.y) / grid.cell_size);

        if (!grid.cells.contains(std::make_pair(cell_pos_x, cell_pos_y))) {
            return;
        }

        flecs::entity cell = grid.cells[std::make_pair(cell_pos_x, cell_pos_y)];
        e.add<ContainedIn>(cell);
    }
}
#endif //UPDATE_CELL_ENTITIES_SYSTEM_H
