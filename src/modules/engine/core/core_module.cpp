//
// Created by Laurent Voisard on 12/21/2024.
//

// ReSharper disable CppMemberFunctionMayBeStatic
#include "core_module.h"

#include "flecs.h"
#include "components.h"
#include "modules/engine/input/components.h"
#include "modules/engine/physics/components.h"
#include "modules/engine/physics/physics_module.h"

#include <raymath.h>

#include "queries.h"
#include "systems/destroy_entity_after_frame_system.h"
#include "systems/destroy_entity_after_time_system.h"
#include "systems/remove_empty_tables_system.h"
#include "systems/reset_enabled_menus_system.h"
#include "systems/set_time_scale_on_pause_system.h"
#include "systems/set_paused_on_entity_disable_system.h"
#include "systems/set_paused_on_entity_enabled_system.h"
#include "systems/disable_entity_on_close_system.h"
#include "systems/enable_entity_on_open_system.h"

namespace core {
    CoreModule::~CoreModule() {
        //queries::position_and_tag_query.destruct();
    }
    void CoreModule::register_components(flecs::world &world) {
        world.component<Position2D>();
        world.component<Speed>();
        world.component<GameSettings>();
        world.component<Tag>();
        world.component<DestroyAfterTime>();
        world.component<DestroyAfterFrame>();
    }

    void CoreModule::register_queries(flecs::world &world) {
        queries::position_and_tag_query = world.query<Position2D, Tag>();
    }

    void CoreModule::register_systems(flecs::world &world) {
        std::cout << "Registering core systems" << std::endl;

        world.system<EnabledMenus>()
            .kind(flecs::OnStart)
            .term_at(0).singleton()
            .each(systems::reset_enabled_menus_system);

        world.observer<const Paused>()
                .event(flecs::OnSet)
                .each(systems::set_time_scale_on_pause_system);

        world.observer<EnabledMenus>()
                .term_at(0).singleton()
                .with<PauseOnEnabled>().filter()
                .event(flecs::OnAdd)
                .with(flecs::Disabled)
                .each(systems::set_paused_on_entity_disable_system);

        world.observer<EnabledMenus>()
                .term_at(0).singleton()
                .with<PauseOnEnabled>().filter()
                .event(flecs::OnRemove)
                .with(flecs::Disabled)
                .each(systems::set_paused_on_entity_enabled_system);

        world.observer()
                .event(flecs::OnAdd)
                .with<Close>()
                .each(systems::disable_entity_on_close_system);

        world.observer()
                .event(flecs::OnAdd)
                .with<Open>()
                .with(flecs::Disabled).filter()
                .each(systems::enable_entity_on_open_system);

        world.system<DestroyAfterTime>("Destroy entities after time")
                .kind(flecs::PostFrame)
                .write<DestroyAfterFrame>()
                .multi_threaded()
                .each(systems::destroy_entity_after_time_system);

        world.system("Destroy entities after frame")
                .with<DestroyAfterFrame>()
                .kind(flecs::PostFrame)
                .multi_threaded()
                .each(systems::destroy_entity_after_frame_system);

        world.system(
                    "Remove empty tables to avoid fragmentation in collision (CHANGE TO DONTFRAGMENT WHEN FEATURE IS OUT)")
                //.interval(0.25f)
                .kind(flecs::PostFrame)
                .run([world](flecs::iter &it) { systems::remove_empty_tables_system(world); });
    }
}
