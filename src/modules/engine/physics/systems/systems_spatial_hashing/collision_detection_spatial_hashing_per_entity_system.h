//
// Created by laurent on 07/07/25.
//

#ifndef COLLISION_DETECTION_SPATIAL_HASHING_PER_ENTITY_SYSTEM_H
#define COLLISION_DETECTION_SPATIAL_HASHING_PER_ENTITY_SYSTEM_H

#include <flecs.h>

#include "modules/engine/core/components.h"
#include "modules/engine/physics/components.h"

namespace physics::systems {

    inline void collision_detection_spatial_hashing_per_entity_system(flecs::entity e, CollisionRecordList &list,
                                                                      SpatialHashingGrid &grid,
                                                                      const core::Position2D &pos,
                                                                      const Collider collider) {

        int cell_pos_x = std::floor((pos.value.x - grid.offset.x) / grid.cell_size);
        int cell_pos_y = std::floor((pos.value.y - grid.offset.y) / grid.cell_size);

        if (!grid.cells.contains(std::make_pair(cell_pos_x, cell_pos_y))) {
            return;
        }
        flecs::entity cell = grid.cells[std::make_pair(cell_pos_x, cell_pos_y)];
        for (int offset_y = -1; offset_y <= 1; offset_y++) {
            for (int offset_x = -1; offset_x <= 1; offset_x++) {
                int x = cell.get<GridCell>().x + offset_x;
                int y = cell.get<GridCell>().y + offset_y;
                if (!grid.cells.contains(std::make_pair(x, y)))
                    continue;

                const GridCell neighbour = grid.cells[std::make_pair(x, y)].get<GridCell>();

                    for (int j = 0; j < neighbour.entities.size(); j++) {
                        flecs::entity other = neighbour.entities[j];
                        if (e.id() <= other.id())
                            continue;

                        const Collider other_collider = neighbour.entities[j].get<Collider>();
                        if ((collider.collision_filter & other_collider.collision_type) == none)
                            continue;

                        CollisionInfo a_info;
                        CollisionInfo b_info;
                        if (collision_handler[collider.type][other_collider.type](e, collider, a_info, other,
                                                                                  other_collider, b_info)) {
                            list.records.push_back({e, other, a_info, b_info});
                        }
                    }

            }
        }
    }
} // namespace physics::systems
#endif // COLLISION_DETECTION_SPATIAL_HASHING_PER_ENTITY_SYSTEM_H
