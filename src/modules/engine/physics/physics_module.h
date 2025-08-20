//
// Created by Laurent Voisard on 12/22/2024.
//

#ifndef PHYSICS_MODULE_H
#define PHYSICS_MODULE_H


#include <cmath>
#include <functional>
#include <vector>

#include "flecs.h"
#include "modules/base_module.h"
#include "modules/engine/core/components.h"
#include "modules/engine/rendering/components.h"

#include <raymath.h>

#include "components.h"
#include "perf_recorder.h"


namespace physics {
    enum PHYSICS_COLLISION_STRATEGY {
        COLLISION_RELATIONSHIP,
        COLLISION_RELATIONSHIP_DONTFRAGMENT,
        COLLISION_ENTITY,
        RECORD_LIST,
        SPATIAL_HASH_PER_CELL,
        SPATIAL_HASH_PER_ENTITY,
        SPATIAL_HASH_RELATIONSHIP,
        COUNT
    };

    constexpr float PHYSICS_TICK_LENGTH = 0.016f;
    static CollisionFilter player_filter = static_cast<CollisionFilter>(enemy | environment);
    static CollisionFilter enemy_filter = static_cast<CollisionFilter>(player | enemy | environment);
    static CollisionFilter environment_filter = static_cast<CollisionFilter>(player | enemy);

    inline flecs::system m_collision_detection_spatial_hashing_system;
    inline flecs::system m_collision_detection_spatial_ecs;
    inline flecs::system m_collision_detection_naive_system;

    inline std::vector<std::vector<flecs::system>> collision_method_systems;
    inline std::vector<std::vector<flecs::entity>> collision_method_observers;
    inline flecs::entity m_physicsTick;

    inline std::chrono::system_clock::time_point start_update;
    inline std::chrono::system_clock::time_point end_update;
    inline std::chrono::system_clock::time_point start_detection;
    inline std::chrono::system_clock::time_point end_detection;
    inline std::chrono::system_clock::time_point start_resolution;
    inline std::chrono::system_clock::time_point end_resolution;
    inline std::chrono::system_clock::time_point start_event;
    inline std::chrono::system_clock::time_point end_event;
    inline std::chrono::system_clock::time_point start_cleanup;
    inline std::chrono::system_clock::time_point end_cleanup;

    inline double get_update_time() { return std::chrono::duration<double>(end_update - start_update).count(); }
    inline double get_detection_time() { return std::chrono::duration<double>(end_detection - start_detection).count(); }
    inline double get_resolution_time() { return std::chrono::duration<double>(end_resolution - start_resolution).count(); }
    inline double get_event_time() { return std::chrono::duration<double>(end_event - start_event).count(); }
    inline double get_cleanup_time() { return std::chrono::duration<double>(end_cleanup - start_cleanup).count(); }

    inline PHYSICS_COLLISION_STRATEGY strategy;

    inline void print_dt_test(flecs::world world) {
        double total = 0.0f;
        for (auto s: collision_method_systems[strategy]) {
            ecs_system_stats_t stats;
            if (ecs_system_stats_get(world, s, &stats)) {
                // std::cout << s.name() << " avg: " << stats.time_spent.gauge.avg[stats.query.t] << std::endl;
                // total += stats.time_spent.counter.value[stats.query.t];
                const ecs_system_t *sys = ecs_system_get(world, s);
                if (sys) {
                    total += sys->time_spent;


                } else {
                    std::cout << s.name() << std::endl;
                }
            }
        }
        std::cout << "Total collision system time: " << total << std::endl;
    }

    class PhysicsModule : public BaseModule<PhysicsModule> {
        friend class BaseModule<PhysicsModule>;

    public:
        // do not add implementation to the constructor
        PhysicsModule(flecs::world &world) : BaseModule(world) {};

        ~PhysicsModule();

        static PerfRecorder rec;
        static void set_collision_strategy(PHYSICS_COLLISION_STRATEGY strategy);
        static void reset_systems_list();


    private:
        void register_components(flecs::world &world);

        void register_queries(flecs::world &world);

        void register_systems(flecs::world &world);

        void register_pipeline(flecs::world &world);
    };
} // namespace physics

#endif // PHYSICS_MODULE_H
