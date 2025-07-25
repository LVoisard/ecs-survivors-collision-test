//
// Created by Laurent on 7/24/2025.
//

#ifndef COLLISION_DETECTION_RECORD_LIST_SYSTEM_H
#define COLLISION_DETECTION_RECORD_LIST_SYSTEM_H

#include <flecs.h>
#include <mutex>
#include <vector>
#include "modules\engine\physics\components.h"
#include "modules\engine\core\components.h"

namespace physics::systems {
    inline void collision_detection_non_static_record_list_system(flecs::iter &self_it, size_t self_id, CollisionRecordList &list,
                                                      const core::Position2D &pos, const Collider &collider) {
        std::vector<CollisionRecord> collisions;
        std::vector<CollisionRecord> events;
        flecs::world stage_world = self_it.world();

        // Build a staged query, and filter
        auto visible_query =
                stage_world.query_builder<const core::Position2D, const Collider>().with<rendering::Visible>().filter();
        flecs::entity self = self_it.entity(self_id);

        visible_query.each([&](flecs::iter &other_it, size_t other_id, const core::Position2D &other_pos,
                               const Collider &other_collider) {
            flecs::entity other = other_it.entity(other_id);
            if (other.id() <= self.id())
                return;

            if ((collider.collision_filter & other_collider.collision_type) == none)
                return;

            // get ready for next step
            collisions.push_back({self, other});

        });


        // not ideal, there is a bit of loss of time because of the lock
        list_mutex.lock();
        list.records.insert(list.records.end(), collisions.begin(), collisions.end());
        list_mutex.unlock();
    }
}
#endif //COLLISION_DETECTION_RECORD_LIST_SYSTEM_H
