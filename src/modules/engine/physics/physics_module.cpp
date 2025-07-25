//
// Created by Laurent Voisard on 12/22/2024.
//

#include "physics_module.h"
#include "pipeline_steps.h"

#include <modules/engine/rendering/pipeline_steps.h>
#include <raygui.h>
#include <raymath.h>

#include "components.h"
#include "modules/engine/core/components.h"
#include "queries.h"

#include "modules/engine/core/core_module.h"
#include "modules/engine/rendering/components.h"
#include "systems/add_collided_with_system.h"
#include "systems/collision_detection_system.h"
#include "systems/collision_resolution_system.h"
#include "systems/reset_desired_velocity_system.h"
#include "systems/update_position_system.h"
#include "systems/update_velocity_system.h"

#include "systems/systems_relationship/collision_detection_relationship_system.h"
#include "systems/systems_relationship/cleanup.h"

#include "systems/systems_dont_fragment_relationship/collision_detection_dont_fragment_relationship_system.h"
#include "systems/systems_dont_fragment_relationship/cleanup.h"

#include "systems/systems_record_list/collision_detection_record_list_system.h"
#include "systems/systems_record_list/cleanup.h"

#include "systems/systems_create_entity/collision_detection_create_entity_system.h"
#include "systems/systems_create_entity/cleanup.h"

#include "systems/systems_spatial_hashing/collision_detection_spatial_hashing_system.h"
#include "systems/systems_spatial_hashing/init_spatial_hashing_grid_system.h"
#include "systems/systems_spatial_hashing/update_cell_entities_system.h"
#include "systems/systems_spatial_hashing/update_grid_on_window_resized_system.h"
#include "systems/systems_spatial_hashing/update_grid_system.h"

#include "systems/systems_spatial_hashing_relationship/collision_detection_relationship_spatial_hashing_system.h"

namespace physics {

    void enable_system(flecs::entity &e, bool enabled) { enabled ? e.enable() : e.disable(); }

    PhysicsModule::~PhysicsModule() {
        m_collision_detection_spatial_hashing_system.destruct();
        m_collision_detection_spatial_ecs.destruct();
        m_collision_detection_naive_system.destruct();
        //queries::visible_collision_bodies_query.destruct();
        //queries::box_collider_query.destruct();

    }

    void PhysicsModule::set_collision_strategy(PHYSICS_COLLISION_STRATEGY strategy) {
        for(int i = 0; i < collision_method_systems.size(); i++) {
            for (auto e: collision_method_systems[i]) {
                const auto s = static_cast<PHYSICS_COLLISION_STRATEGY>(i);
                enable_system(e, strategy == s);
            }
        }
    }


    void PhysicsModule::reset_systems_list() {
        for(auto s: collision_method_systems) {
            for(auto e: s) {
               //e.destruct();
            }
            s.clear();
        }
        collision_method_systems.clear();
        collision_method_systems = std::vector<std::vector<flecs::entity>>(PHYSICS_COLLISION_STRATEGY::COUNT);
    }

    void PhysicsModule::register_components(flecs::world &world) {
        world.component<Velocity2D>();
        world.component<AccelerationSpeed>();
        world.component<CollidedWith>();
        world.component<NonFragmentingCollidedWith>().add(flecs::DontFragment);
        world.component<ContainedIn>().add(flecs::Exclusive);
    }

    void PhysicsModule::register_queries(flecs::world &world) {
        queries::visible_collision_bodies_query =
                world.query_builder<core::Position2D, Collider>().with<rendering::Visible>().build();

        queries::box_collider_query = world.query_builder<core::Position2D, Collider>().with<BoxCollider>().build();
    }

    void PhysicsModule::register_systems(flecs::world &world) {
        m_physicsTick = world.timer().interval(PHYSICS_TICK_LENGTH);


        collision_method_systems[SPATIAL_HASH].push_back(world.system<SpatialHashingGrid, core::GameSettings>("init grid normal")
                                                  .term_at(0).singleton()
                                                  .term_at(1).singleton()
                                                  .kind(flecs::OnStart)
                                                  .each(systems::init_spatial_hashing_grid_system));

        collision_method_systems[SPATIAL_HASH_RELATIONSHIP].push_back(world.system<SpatialHashingGrid, core::GameSettings>("init grid relationship")
                                                  .term_at(0).singleton()
                                                  .term_at(1).singleton()
                                                  .kind(flecs::OnStart)
                                                  .each(systems::init_spatial_hashing_grid_system));

        collision_method_systems[SPATIAL_HASH].push_back(
                world.system<SpatialHashingGrid, core::GameSettings>("update grid on window resized")
                        .term_at(0).singleton()
                        .term_at(1).singleton()
                        .kind(flecs::OnUpdate)
                        .each(systems::update_grid_on_window_resized_system));

        collision_method_systems[SPATIAL_HASH_RELATIONSHIP].push_back(
                world.system<SpatialHashingGrid, core::GameSettings>("update grid on window resized relationship")
                        .term_at(0).singleton()
                        .term_at(1).singleton()
                        .kind(flecs::OnUpdate)
                        .each(systems::update_grid_on_window_resized_system));

        collision_method_systems[SPATIAL_HASH].push_back(
                world.observer<SpatialHashingGrid, core::GameSettings>("update grid on grid set")
                        .term_at(1).singleton()
                        .event(flecs::OnSet)
                        .each(systems::reset_grid));

        collision_method_systems[SPATIAL_HASH_RELATIONSHIP].push_back(
                world.observer<SpatialHashingGrid, core::GameSettings>("update grid on grid set relationship")
                        .term_at(1).singleton()
                        .event(flecs::OnSet)
                        .each(systems::reset_grid));

        collision_method_systems[SPATIAL_HASH].push_back(
                world.system<SpatialHashingGrid, rendering::TrackingCamera, core::GameSettings, GridCell>("update grid")
                        .term_at(0).singleton()
                        .term_at(1).singleton()
                        .term_at(2).singleton()
                        .kind(flecs::PreUpdate)
                        .each(systems::update_grid_system));

        collision_method_systems[SPATIAL_HASH_RELATIONSHIP].push_back(
                world.system<SpatialHashingGrid, rendering::TrackingCamera, core::GameSettings, GridCell>("update grid relationship")
                        .term_at(0).singleton()
                        .term_at(1).singleton()
                        .term_at(2).singleton()
                        .kind(flecs::PreUpdate)
                        .each(systems::update_grid_system));


        world.system<const Velocity2D, DesiredVelocity2D>("reset desired vel")
                .kind(flecs::PreUpdate)
                .multi_threaded()
                .tick_source(m_physicsTick)
                .each(systems::reset_desired_velocity_system);

        world.system<Velocity2D, const DesiredVelocity2D, const AccelerationSpeed>("Lerp Current to Desired Velocity")
                .kind<UpdateBodies>()
                .multi_threaded()
                .tick_source(m_physicsTick)
                .each(systems::update_velocity_system);

        world.system<core::Position2D, const Velocity2D>("Update Position")
                .kind<UpdateBodies>()
                .multi_threaded()
                .tick_source(m_physicsTick)
                .each(systems::update_position_system);

        collision_method_systems[SPATIAL_HASH].push_back(
                world.system<SpatialHashingGrid, Collider, core::Position2D>("update entity cells")
                        .term_at(0).singleton()
                        .without<StaticCollider>()
                        .kind<UpdateBodies>()
                        .each(systems::update_cell_entities_system));

        collision_method_systems[SPATIAL_HASH_RELATIONSHIP].push_back(
                world.system<SpatialHashingGrid, Collider, core::Position2D>("update entity cells relationship")
                        .term_at(0).singleton()
                        .without<StaticCollider>()
                        .kind<UpdateBodies>()
                        .each(systems::update_cell_entities_system));


        m_collision_detection_naive_system = world.system<CollisionRecordList, const core::Position2D, const Collider>(
                                                          "Detect Collisions ECS (Naive Record List) non-static")
                                                     .term_at(0).singleton()
                                                     .with<rendering::Visible>()
                                                     .kind<Detection>()
                                                     .multi_threaded()
                                                     .tick_source(m_physicsTick)
                                                     .each(systems::collision_detection_non_static_record_list_system);
        m_collision_detection_naive_system.disable();
        collision_method_systems[RECORD_LIST].push_back(m_collision_detection_naive_system);

        collision_method_systems[COLLISION_RELATIONSHIP].push_back(
                world.system<const core::Position2D, const Collider>("Detect Collisions ECS (Relationship)")
                        .with<rendering::Visible>()
                        .kind<Detection>()
                        .multi_threaded()
                        .tick_source(m_physicsTick)
                        .each(systems::collision_detection_non_static_relationship_system));

        collision_method_systems[COLLISION_RELATIONSHIP_DONTFRAGMENT].push_back(
                world.system<const core::Position2D, const Collider>("Detect Collisions ECS (Relationship non-frag)")
                        .with<rendering::Visible>()
                        .kind<Detection>()
                        .multi_threaded()
                        .tick_source(m_physicsTick)
                        .each(systems::collision_detection_non_static_relationship_non_fragmenting_system));

        collision_method_systems[COLLISION_ENTITY].push_back(
            world.system<const core::Position2D, const Collider>("Detect Collisions ECS (entity)")
                .with<rendering::Visible>()
                .kind<Detection>()
                .tick_source(m_physicsTick)
                .immediate()
                .each([world] (flecs::iter& it, size_t i, const core::Position2D& pos, const Collider& col) {
                    systems::collision_detection_non_static_entity_system(world, it, i, pos, col );
                }));

        m_collision_detection_spatial_hashing_system =
                world.system<CollisionRecordList, SpatialHashingGrid, GridCell>(
                             "Detect Collisions ECS non-static with spatial hashing")
                        .term_at(0).singleton()
                        .term_at(1).singleton()
                        .kind<Detection>()
                        .multi_threaded()
                        .tick_source(m_physicsTick)
                        .each(systems::collision_detection_spatial_hashing_system);
        // m_collision_detection_spatial_hashing_system.disable();
        collision_method_systems[SPATIAL_HASH].push_back(m_collision_detection_spatial_hashing_system);

        m_collision_detection_spatial_ecs =
                world.system<CollisionRecordList, SpatialHashingGrid, GridCell>("test collision with relationship")
                        .term_at(0).singleton()
                        .term_at(1).singleton()
                        .kind<Detection>()
                        .multi_threaded()
                        .tick_source(m_physicsTick)
                        .each(systems::collision_detection_relationship_spatial_hashing_system);
        m_collision_detection_spatial_ecs.disable();
        collision_method_systems[SPATIAL_HASH_RELATIONSHIP].push_back(m_collision_detection_spatial_ecs);

        collision_method_systems[RECORD_LIST].push_back(
                world.system<CollisionRecordList>("Collision Resolution ECS (Naive Record List) 1")
                        .term_at(0).singleton()
                        .kind<Resolution>()
                        .tick_source(m_physicsTick)
                        .each(systems::collision_resolution_rec_list_system));

        collision_method_systems[SPATIAL_HASH].push_back(
                world.system<CollisionRecordList>("Collision Resolution ECS (spatial hash) ")
                        .term_at(0).singleton()
                        .kind<Resolution>()
                        .tick_source(m_physicsTick)
                        .each(systems::collision_resolution_rec_list_system));

        collision_method_systems[RECORD_LIST].push_back(world.system<CollisionRecordList>("Add CollidedWith Component 1")
                                                 .term_at(0).singleton()
                                                 .kind<Resolution>()
                                                 .tick_source(m_physicsTick)
                                                 .each(systems::add_collided_with_system));
        collision_method_systems[SPATIAL_HASH].push_back(world.system<CollisionRecordList>("Add CollidedWith Component 2")
                                                  .term_at(0).singleton()
                                                  .kind<Resolution>()
                                                  .tick_source(m_physicsTick)
                                                  .each(systems::add_collided_with_system));

        collision_method_systems[COLLISION_RELATIONSHIP].push_back(world.system("Collision Cleanup (relationship)")
                .with<Collider>()
                .kind<CollisionCleanup>()
                .immediate()
                .tick_source(m_physicsTick)
                .each(systems::collision_cleanup_system));

        collision_method_systems[COLLISION_RELATIONSHIP_DONTFRAGMENT].push_back(world.system("Collision Cleanup (dont fragment relationship)")
                .with<NonFragmentingCollidedWith>(flecs::Wildcard)
                .with<Collider>()
                .kind<CollisionCleanup>()
                .immediate()
                .tick_source(m_physicsTick)
                .each(systems::collision_cleanup_non_frag_system));

        collision_method_systems[COLLISION_ENTITY].push_back(world.system("Collision Cleanup (entity)")
                .kind<CollisionCleanup>()
                .immediate()
                .tick_source(m_physicsTick)
                .run(systems::collision_cleanup_entity));

        collision_method_systems[RECORD_LIST].push_back(world.system<CollisionRecordList>("Collision Cleanup List 1")
                                                 .term_at(0).singleton()
                                                 .kind<CollisionCleanup>()
                                                 .tick_source(m_physicsTick)
                                                 .each(systems::collision_cleanup_list_system));

        collision_method_systems[SPATIAL_HASH].push_back(world.system<CollisionRecordList>("Collision Cleanup List 2")
                                                  .term_at(0).singleton()
                                                  .kind<CollisionCleanup>()
                                                  .tick_source(m_physicsTick)
                                                  .each(systems::collision_cleanup_list_system));
    }

    void PhysicsModule::register_pipeline(flecs::world &world) {
        world.component<UpdateBodies>().add(flecs::Phase).depends_on(flecs::OnUpdate);
        world.component<Detection>().add(flecs::Phase).depends_on(flecs::OnValidate);
        world.component<Resolution>().add(flecs::Phase).depends_on(flecs::PostUpdate); // to use from external modules
        world.component<CollisionCleanup>().add(flecs::Phase).depends_on<Resolution>();
    }
} // namespace physics
