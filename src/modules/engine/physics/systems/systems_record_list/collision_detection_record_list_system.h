//
// Created by Laurent on 7/24/2025.
//

#ifndef COLLISION_DETECTION_RECORD_LIST_SYSTEM_H
#define COLLISION_DETECTION_RECORD_LIST_SYSTEM_H

#include <flecs.h>
#include <mutex>
#include <vector>
#include "modules/engine/core/components.h"
#include "modules/engine/physics/components.h"

namespace physics::systems {
    inline void collision_detection_non_static_record_list_system(flecs::iter &it, size_t id,
                                                                  CollisionRecordList &list) {
        std::vector<CollisionRecord> collisions;
        std::vector<CollisionRecord> events;
        flecs::world stage_world = it.world();

        // Build a staged query, and filter
        auto visible_query =
                stage_world.query_builder<const core::Position2D, const Collider>().with<rendering::Visible>().filter();
        auto visible_query_1 =
                stage_world.query_builder<const core::Position2D, const Collider>().with<rendering::Visible>().filter();
        visible_query_1.each(
                [&](flecs::iter &self_it, size_t self_id, const core::Position2D &other_pos, const Collider &collider) {
                    flecs::entity self = self_it.entity(self_id);

                    visible_query.each([&](flecs::iter &other_it, size_t other_id, const core::Position2D &other_pos,
                                           const Collider &other_collider) {
                        flecs::entity other = other_it.entity(other_id);
                        if (other.id() <= self.id())
                            return;

                        if ((collider.collision_filter & other_collider.collision_type) == none)
                            return;

                        CollisionInfo a_info;
                        CollisionInfo b_info;
                        if (collision_handler[collider.type][other_collider.type](self, collider, a_info, other,
                                                                                  other_collider, b_info)) {
                            list.records.push_back({self, other, a_info, b_info});
                        }
                    });
                });


        // not ideal, there is a bit of loss of time because of the lock
        // list_mutex.lock();
        // list.records.insert(list.records.end(), collisions.begin(), collisions.end());
        // list_mutex.unlock();
    }
} // namespace physics::systems
#endif // COLLISION_DETECTION_RECORD_LIST_SYSTEM_H
