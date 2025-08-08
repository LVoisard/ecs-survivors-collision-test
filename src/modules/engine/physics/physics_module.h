//
// Created by Laurent Voisard on 12/22/2024.
//

#ifndef PHYSICS_MODULE_H
#define PHYSICS_MODULE_H


#include <cmath>
#include <functional>
#include <vector>

#include "modules/base_module.h"
#include "flecs.h"
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
        SPATIAL_HASH,
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

    inline std::vector<std::vector<flecs::entity>> collision_method_systems;
    inline flecs::entity m_physicsTick;

    class PhysicsModule : public BaseModule<PhysicsModule> {
        friend class BaseModule<PhysicsModule>;

    public:
        // do not add implementation to the constructor
        PhysicsModule(flecs::world &world): BaseModule(world) {
        };

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
}

#endif //PHYSICS_MODULE_H
