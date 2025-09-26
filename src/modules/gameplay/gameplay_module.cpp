//
// Created by laure on 3/12/2025.
//

#include "gameplay_module.h"
#include "components.h"
#include "modules/engine/core/components.h"
#include "modules/engine/physics/components.h"
#include <raymath.h>
#include <set>

#include "modules/engine/rendering/components.h"
#include <unordered_set>

#include "modules/engine/physics/physics_module.h"
#include "modules/engine/physics/pipeline_steps.h"
#include "modules/engine/rendering/gui/gui_module.h"
#include "modules/engine/rendering/gui/prefabs.h"
#include "pipeline_steps.h"
#include "systems/add_bounce_system.h"
#include "systems/add_chain_system.h"
#include "systems/add_multiproj_system.h"
#include "systems/add_pierce_system.h"
#include "systems/add_split_system.h"
#include "systems/check_if_dead_system.h"
#include "systems/create_health_bar_system.h"
#include "systems/deal_damage_on_collision_system.h"
#include "systems/decrement_bounce_system.h"
#include "systems/decrement_chain_system.h"
#include "systems/decrement_multiproj_system.h"
#include "systems/decrement_pierce_system.h"
#include "systems/fire_projectile_system.h"
#include "systems/give_experience_system.h"
#include "systems/increment_bounce_system.h"
#include "systems/increment_chain_system.h"
#include "systems/increment_multiproj_system.h"
#include "systems/increment_pierce_system.h"
#include "systems/projectile_bounce_collided_system.h"
#include "systems/projectile_chain_collided_system.h"
#include "systems/projectile_no_bounce_collided_system.h"
#include "systems/projectile_no_effect_collided_system.h"
#include "systems/projectile_pierce_collided_system.h"
#include "systems/projectile_split_collision_system.h"
#include "systems/regen_health_system.h"
#include "systems/remove_bounce_system.h"
#include "systems/remove_chain_system.h"
#include "systems/remove_multproj_system.h"
#include "systems/remove_pierce_system.h"
#include "systems/remove_split_system.h"
#include "systems/restart_cooldown_system.h"
#include "systems/spawn_enemies_around_screen_system.h"
#include "systems/take_damage_system.h"
#include "systems/update_cooldown_system.h"
#include "systems/update_health_bar_system.h"

namespace gameplay {
    void GameplayModule::register_components(flecs::world world) {
        world.component<Spawner>();
    }

    void GameplayModule::register_systems(flecs::world world) {
        m_spawner_tick = world.timer().interval(spawner_interval);

        spawn_system = world.system<const Spawner, const core::GameSettings, const rendering::TrackingCamera>("Spawn Enemies")
                //.tick_source(m_spawner_tick)
                .each(systems::spawn_enemies_around_screen_system);

        // world.system<Cooldown>("Update Cooldown")
        //         .without<CooldownCompleted>()
        //         .each(systems::update_cooldown_system);
        //
        // world.observer("Restart Cooldown")
        //         .with<CooldownCompleted>()
        //         .event(flecs::OnRemove)
        //         .each(systems::restart_cooldown_system);
        //
        // world.system<core::Position2D, Attack, core::Speed, MultiProj *>("Fire Projectile")
        //         .with<Projectile>()
        //         .with<CooldownCompleted>()
        //         .write<CooldownCompleted>()
        //         .term_at(0).parent()
        //         .each(systems::fire_projectile_system);

        // world.system("delete if hit wall and no bounce")
        //         .with<physics::CollidedWith>(flecs::Wildcard)
        //         .with<Projectile>()
        //         .without<Bounce>()
        //         .kind<OnCollisionDetected>()
        //         .immediate()
        //         .each(systems::projectile_no_bounce_collided_system);
        //
        // world.system<physics::CollisionRecordList, Bounce, physics::Velocity2D, rendering::Rotation>(
        //             "Bounce if hit wall")
        //         .with<physics::CollidedWith>(flecs::Wildcard)
        //         .with<Projectile>()
        //         .kind<OnCollisionDetected>()
        //         .term_at(0).singleton()
        //         .immediate()
        //         .each(systems::projectile_bounce_collided_system);

        world.system("no pierce or chain")
                .with<Projectile>()
                .without<Pierce>().without<Chain>()
                .with<physics::CollidedWith>(flecs::Wildcard)
                .kind<OnCollisionDetected>()
                .immediate()
                .each(systems::project_no_effect_collided_system);

        world.system("no pierce or chain (non-frag)")
                .with<Projectile>()
                .without<Pierce>().without<Chain>()
                .with<physics::NonFragmentingCollidedWith>(flecs::Wildcard)
                .kind<OnCollisionDetected>()
                .immediate()
                .each(systems::project_no_effect_collided_system);

        // world.system<Pierce>("apply pierce mod")
        //         .with<physics::CollidedWith>(flecs::Wildcard)
        //         .kind<OnCollisionDetected>()
        //         .immediate()
        //         .each(systems::projectile_pierce_collided_system);
        //
        // world.system<Chain, physics::Velocity2D, core::Position2D, rendering::Rotation, Attack>("apply chain mod")
        //         .with<physics::CollidedWith>(flecs::Wildcard)
        //         .kind<OnCollisionDetected>()
        //         .immediate()
        //         .each(systems::projectile_chain_collided_system);
        //
        // world.system<Split, physics::Velocity2D, core::Position2D, rendering::Rotation, Attack>("apply split mod")
        //         .with<physics::CollidedWith>(flecs::Wildcard)
        //         .kind<OnCollisionDetected>()
        //         .immediate()
        //         .each(systems::projectile_split_collision_system);

        // world.system<Damage>("collision detected, deal damage to target")
        //         .with<physics::CollidedWith>(flecs::Wildcard)
        //         .immediate()
        //         .kind<OnCollisionDetected>()
        //         .tick_source(physics::m_physicsTick)
        //         .each(systems::deal_damage_on_collision_system);
        //
        // world.system<Damage>("collision detected, deal damage to target (non-frag)")
        //         .with<physics::NonFragmentingCollidedWith>(flecs::Wildcard)
        //         .immediate()
        //         .kind<OnCollisionDetected>()
        //         .tick_source(physics::m_physicsTick)
        //         .each(systems::deal_damage_on_collision_system);
        //
        // world.system<const Health>("create health bar")
        //         .with<TakeDamage>()
        //         .without<HealthBar>()
        //         .without<rendering::ProgressBar>()
        //         .kind<PostCollisionDetected>()
        //         .each(systems::create_health_bar_system);
        //
        // world.system<Health, TakeDamage>("take damage")
        //         .kind<PostCollisionDetected>()
        //         .each(systems::take_damage_system);
        //
        // world.system<const Health>("check if dead")
        //         .kind<PostCollisionDetected>()
        //         .immediate()
        //         .each(systems::check_if_dead_system);
        //
        //
        // world.system<const Health, rendering::ProgressBar>("update health bar on take damage")
        //         .kind<PostCollisionDetected>()
        //         .term_at(0).parent()
        //         .each(systems::update_health_bar_system);
        //
        // world.system<Health, RegenHealth>("regen health")
        //         .kind(flecs::OnUpdate)
        //         .each(systems::regen_health_system);
        //
        // world.system<GiveExperience>()
        //         .with<core::DestroyAfterFrame>()
        //         .term_at(0).second<OnDeathEffect>()
        //         .kind<PostCollisionDetected>()
        //         .each(systems::give_experience_system);

        add_multiproj = world.system("add multi proj")
                .kind(0)
                .with<Attack>()
                .without<MultiProj>()
                .with(flecs::ChildOf, "player")
                .immediate()
                .each(systems::add_multiproj_system);

        remove_multiproj = world.system("remove multi proj")
                .kind(0)
                .with<Attack>()
                .with<MultiProj>()
                .with(flecs::ChildOf, "player")
                .immediate()
                .each(systems::remove_multiproj_system);

        add_pierce = world.system("add pierce")
                .kind(0)
                .with<Projectile>()
                .without<Pierce>()
                .with(flecs::Prefab)
                .immediate()
                .each([world](flecs::entity e) {
                    systems::add_pierce_system(world, e);
                });

        remove_pierce = world.system("remove pierce")
                .kind(0)
                .with<Projectile>()
                .with<Pierce>()
                .with(flecs::Prefab)
                .immediate()
                .each([world](flecs::entity e) {
                    systems::remove_pierce_system(world, e);
                });

        add_chain = world.system("add chain")
                .kind(0)
                .with<Projectile>()
                .without<Chain>()
                .with(flecs::Prefab)
                .immediate()
                .each([world](flecs::entity e) {
                    systems::add_chain_system(world, e);
                });

        remove_chain = world.system("remove chain")
                .kind(0)
                .with<Projectile>()
                .with<Chain>()
                .with(flecs::Prefab)
                .immediate()
                .each([world](flecs::entity e) {
                    systems::remove_chain_system(world, e);
                });

        add_split = world.system("add split")
                .kind(0)
                .with<Projectile>()
                .without<Split>()
                .with(flecs::Prefab)
                .immediate()
                .each([world](flecs::entity e) {
                    systems::add_split_system(world, e);
                });

        remove_split = world.system("remove split")
                .kind(0)
                .with<Projectile>()
                .with<Split>()
                .with(flecs::Prefab)
                .immediate()
                .each([world](flecs::entity e) {
                    systems::remove_split_system(world, e);
                });

        add_proj = world.system<MultiProj>("+1 proj")
                .kind(0)
                .with<Attack>()
                .with(flecs::ChildOf, "player")
                .each(systems::increment_multiproj_system);

        remove_proj = world.system<MultiProj>("-1 proj")
                .kind(0)
                .with<Attack>()
                .with(flecs::ChildOf, "player")
                .each(systems::decrement_multiproj_system);

        add_pierce_amt = world.system<Pierce>("+1 Pierce")
                .kind(0)
                .with<Projectile>()
                .with(flecs::Prefab)
                .each(systems::increment_pierce_system);

        remove_pierce_amt = world.system<Pierce>("-1 Pierce")
                .kind(0)
                .with<Projectile>()
                .with(flecs::Prefab)
                .each(systems::decrement_pierce_system);

        add_chain_amt = world.system<Chain>("+1 Chain")
                .kind(0)
                .with<Projectile>()
                .with(flecs::Prefab)
                .each(systems::increment_chain_system);

        remove_chain_amt = world.system<Chain>("-1 Chain")
                .kind(0)
                .with<Projectile>()
                .with(flecs::Prefab)
                .each(systems::decrement_chain_system);

        add_bounce = world.system("add bounce")
                .kind(0)
                .with<Projectile>()
                .without<Bounce>()
                .with(flecs::Prefab)
                .immediate()
                .each(systems::add_bounce_system);
        remove_bounce = world.system("remove bounce")
                .kind(0)
                .with<Projectile>()
                .with<Bounce>()
                .with(flecs::Prefab)
                .immediate()
                .each(systems::remove_bounce_system);

        add_bounce_amt = world.system<Bounce>("+1 Bounce")
                .kind(0)
                .with<Projectile>()
                .with(flecs::Prefab)
                .immediate()
                .each(systems::increment_bounce_system);

        remove_bounce_amt = world.system<Bounce>("-1 Bounce")
                .kind(0)
                .with<Projectile>()
                .with(flecs::Prefab)
                .immediate()
                .each(systems::decrement_bounce_system);
    }

    void GameplayModule::register_entities(flecs::world world) {
        auto dropdown = world.entity("gameplay_dropdown").child_of<rendering::gui::prefabs::MenuBar>()
                .set<rendering::gui::MenuBarTab>({"Gameplay Tools", 25});

        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({
                    "Add Multi Proj", add_multiproj, rendering::gui::MenuBarTabItemType::RUN
                });
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({
                    "Remove Multi Proj", remove_multiproj, rendering::gui::MenuBarTabItemType::RUN
                });
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({"+1 Proj", add_proj, rendering::gui::MenuBarTabItemType::RUN});
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({"-1 Proj", remove_proj, rendering::gui::MenuBarTabItemType::RUN});
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>(
                    {"Add Pierce", add_pierce, rendering::gui::MenuBarTabItemType::RUN});
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({
                    "Remove Pierce", remove_pierce, rendering::gui::MenuBarTabItemType::RUN
                });
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({
                    "+1 Pierce", add_pierce_amt, rendering::gui::MenuBarTabItemType::RUN
                });
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({
                    "-1 Pierce", remove_pierce_amt, rendering::gui::MenuBarTabItemType::RUN
                });
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({"Add Chain", add_chain, rendering::gui::MenuBarTabItemType::RUN});
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({
                    "Remove Chain", remove_chain, rendering::gui::MenuBarTabItemType::RUN
                });
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({
                    "+1 Chain", add_chain_amt, rendering::gui::MenuBarTabItemType::RUN
                });
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({
                    "-1 Chain", remove_chain_amt, rendering::gui::MenuBarTabItemType::RUN
                });
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({"Add Split", add_split, rendering::gui::MenuBarTabItemType::RUN});
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({
                    "Remove Split", remove_split, rendering::gui::MenuBarTabItemType::RUN
                });
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({
                    "Add Bounce", add_bounce, rendering::gui::MenuBarTabItemType::RUN
                });
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({
                    "Remove Bounce", remove_bounce, rendering::gui::MenuBarTabItemType::RUN
                });
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({
                    "+1 Bounce", add_bounce_amt, rendering::gui::MenuBarTabItemType::RUN
                });
        world.entity().child_of(dropdown)
                .set<rendering::gui::MenuBarTabItem>({
                    "-1 Bounce", remove_bounce_amt, rendering::gui::MenuBarTabItemType::RUN
                });
    }

    void GameplayModule::register_pipeline(flecs::world world) {
        world.component<OnCollisionDetected>().add(flecs::Phase).depends_on<physics::Detection>();
        world.component<PostCollisionDetected>().add(flecs::Phase).depends_on<OnCollisionDetected>();
    }
}
