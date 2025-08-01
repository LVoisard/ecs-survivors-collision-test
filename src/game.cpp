//
// Created by Laurent Voisard on 12/20/2024.
//

#include "game.h"
#include <iostream>
#include <ostream>
#ifdef __linux__
#include <perfcpp/hardware_info.h>
#include <perfcpp/event_counter.h>
#include <perfcpp/config.h>
#endif

#if defined(EMSCRIPTEN)
#include <emscripten/emscripten.h>
#endif

#include <filesystem>
#include <fstream>
#include <thread>

#include "modules/ai/ai_module.h"
#include "modules/ai/components.h"
#include "modules/debug/debug_module.h"
#include "modules/engine/core/components.h"
#include "modules/engine/core/core_module.h"
#include "modules/engine/input/components.h"
#include "modules/engine/input/input_module.h"
#include "modules/engine/physics/components.h"
#include "modules/engine/physics/physics_module.h"
#include "modules/player/player_module.h"
#include "raylib.h"

#include "modules/engine/rendering/components.h"
#include "modules/engine/rendering/rendering_module.h"
#include "modules/gameplay/components.h"
#include "modules/gameplay/gameplay_module.h"

#include <tmxlite/Layer.hpp>
#include <tmxlite/Map.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/TileLayer.hpp>

#include "modules/engine/rendering/gui/components.h"
#include "modules/engine/rendering/gui/gui_module.h"
#include "modules/engine/rendering/gui/prefabs.h"
#include "modules/tilemap/components.h"
#include "modules/tilemap/tilemap_module.h"

Game::Game(const char *windowName, int windowWidth, int windowHeight) :
    m_windowName(windowName), m_windowHeight(windowHeight), m_windowWidth(windowWidth) {}


void Game::init() {
    m_world = flecs::world();
    // Raylib window
    // #ifndef EMSCRIPTEN
    // web has an scaling issue with the cursor
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    // #endif
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(m_windowWidth, m_windowHeight, m_windowName.c_str());

    SetExitKey(KEY_F4);
#ifndef EMSCRIPTEN
    // use the flecs explorer when not on browser
    m_world.import <flecs::stats>();
    m_world.set<flecs::Rest>({});
    //m_world.set_threads(static_cast<int>(std::thread::hardware_concurrency()));
#endif
    physics::PhysicsModule::reset_systems_list();
    modules.push_back(m_world.import <core::CoreModule>());
    modules.push_back(m_world.import <input::InputModule>());
    modules.push_back(m_world.import <rendering::RenderingModule>());
    modules.push_back(m_world.import <physics::PhysicsModule>());
    modules.push_back(m_world.import <player::PlayerModule>());
    modules.push_back(m_world.import <ai::AIModule>());
    modules.push_back(m_world.import <gameplay::GameplayModule>());
    modules.push_back(m_world.import <debug::DebugModule>());
    modules.push_back(m_world.import <tilemap::TilemapModule>());


    m_world.set<core::GameSettings>({m_windowName, m_windowWidth, m_windowHeight, m_windowWidth, m_windowHeight});
    m_world.add<physics::CollisionRecordList>();
    m_world.set<physics::SpatialHashingGrid>({48, {0, 0}});
    m_world.set<core::Paused>({false});
    m_world.set<core::EnabledMenus>({0});
    flecs::entity player = m_world.entity("player")
                                   .set<core::Tag>({"player"})
                                   .set<core::Position2D>({2300.0f, 1300.0f})
                                   .set<core::Speed>({300})
                                   .set<physics::Velocity2D>({0, 0})
                                   .set<physics::DesiredVelocity2D>({0, 0})
                                   .set<physics::AccelerationSpeed>({15.0})
                                   .set<physics::Collider>({
                                           true,
                                           false,
                                           {-24, -24, 48, 48},
                                           physics::CollisionFilter::player,
                                           physics::player_filter,
                                           physics::ColliderType::Circle,
                                   })
                                   .set<physics::CircleCollider>({24})
                                   .set<rendering::Priority>({2})
                                   .set<rendering::Renderable>({LoadTexture("../resources/player.png"), // 8x8
                                                                {0, 0},
                                                                3.f,
                                                                WHITE})
                                   .set<gameplay::Experience>({1, 0, 100000});

    m_world.entity("dagger attack")
            .child_of(player)
            .add<gameplay::Projectile>()
            .set<gameplay::Attack>({"projectile", "enemy"})
            .set<gameplay::Cooldown>({1.0f, 1})
            .add<gameplay::CooldownCompleted>()
            .set<core::Speed>({150.f});

    m_world.prefab("projectile")
            .add<gameplay::Projectile>()
            .set<gameplay::Attack>({"projectile", "enemy"})
            .set<gameplay::Damage>({2})
            .set<physics::Velocity2D>({0, 0})
            .set<physics::Collider>({
                    false,
                    false,
                    {-18, -18, 36, 36},
                    physics::CollisionFilter::player,
                    physics::player_filter,
                    physics::ColliderType::Circle,
            })
            .set<physics::CircleCollider>({18})
            .set<rendering::Priority>({1})
            .set<rendering::Renderable>({LoadTexture("../resources/dagger.png"), // 8x8
                                         {0, 0},
                                         3.f,
                                         WHITE})
            .set<core::DestroyAfterTime>({15});

    auto hori = m_world.entity("player_horizontal_input").child_of(player).set<input::InputHorizontal>({});
    m_world.entity().child_of(hori).set<input::KeyBinding>({KEY_A, -1});
    m_world.entity().child_of(hori).set<input::KeyBinding>({KEY_D, 1});
    m_world.entity().child_of(hori).set<input::KeyBinding>({KEY_LEFT, -1});
    m_world.entity().child_of(hori).set<input::KeyBinding>({KEY_RIGHT, 1});

    auto vert = m_world.entity("player_vertical_input").child_of(player).set<input::InputVertical>({});
    m_world.entity().child_of(vert).set<input::KeyBinding>({KEY_W, -1});
    m_world.entity().child_of(vert).set<input::KeyBinding>({KEY_S, 1});
    m_world.entity().child_of(vert).set<input::KeyBinding>({KEY_UP, -1});
    m_world.entity().child_of(vert).set<input::KeyBinding>({KEY_DOWN, 1});

    flecs::entity enemy = m_world.prefab("enemy")
                                  .set<core::Tag>({"enemy"})
                                  .set<core::Position2D>({800, 400})
                                  .set<core::Speed>({25})
                                  .set<gameplay::Health>({10, 10})
                                  .set<gameplay::Damage>({1})
                                  .set<gameplay::GiveExperience, gameplay::OnDeathEffect>({player, 2})
                                  .add<ai::Target>(player)
                                  .add<ai::FollowTarget>()
                                  .set<ai::StoppingDistance>({16.0})
                                  .set<physics::Velocity2D>({0, 0})
                                  .set<physics::DesiredVelocity2D>({0, 0})
                                  .set<physics::AccelerationSpeed>({5.0})
                                  .set<physics::Collider>({true,
                                                           false,
                                                           {-24, -24, 48, 48},
                                                           physics::CollisionFilter::enemy,
                                                           physics::enemy_filter,
                                                           physics::ColliderType::Circle})
                                  .set<physics::CircleCollider>({24})
                                  .set<rendering::Renderable>({LoadTexture("../resources/ghost.png"), // 8x8
                                                               {0, 0},
                                                               3.f,
                                                               WHITE})
                                  .set<rendering::Priority>({0});

    auto spawner = m_world.entity("enemy_spawner").set<gameplay::Spawner>({enemy, 1});

    // m_world.entity("tilemap_1")
    //         .set<tilemap::Tilemap>({
    //             "../resources/tiled/maps/sampleMap.tmx",
    //             3.0f
    //         });

    m_world.set<rendering::TrackingCamera>({player, Camera2D{0}});

    auto exp_panel = m_world.entity("exp_panel")
                             .child_of(rendering::gui::get_gui_canvas(m_world))
                             .is_a<rendering::gui::prefabs::Panel>()
                             .set<Rectangle>({-250, -65, 500, 60})
                             .set<rendering::gui::Anchor>({rendering::gui::CENTER, rendering::gui::BOTTOM});

    auto exp_bar = m_world.entity("exp_bar")
                           .child_of(exp_panel)
                           .set<rendering::gui::ProgressBar>({0, 10, 0})
                           .set<Rectangle>({-200, -30, 400, 20})
                           .set<rendering::gui::Anchor>({rendering::gui::CENTER, rendering::gui::BOTTOM});

    auto exp_level_txt =
            m_world.entity("exp_level_txt")
                    .child_of(exp_panel)
                    .set<rendering::gui::Text>({"Level: 1", rendering::gui::FONT_SIZE_32, TEXT_ALIGN_CENTER,
                                                rendering::gui::GUIModule::font_color()})
                    .set<Rectangle>({-37.5, 10, 75, 20})
                    .set<rendering::gui::Anchor>({rendering::gui::CENTER, rendering::gui::TOP});

    auto pause_menu = m_world.entity("pause_menu")
                              .child_of(rendering::gui::get_gui_canvas(m_world))
                              .is_a<rendering::gui::prefabs::Panel>()
                              .set<Rectangle>({-150, -200, 300, 400})
                              .set<rendering::gui::Anchor>({rendering::gui::CENTER, rendering::gui::MIDDLE})
                              .add<core::PauseOnEnabled>()
                              .disable();

    pause_menu.child()
            .set<Rectangle>({-150, 5, 300, 50})
            .set<rendering::gui::Anchor>({rendering::gui::CENTER, rendering::gui::TOP})
            .set<rendering::gui::Text>({"Paused", rendering::gui::FONT_SIZE_48, TEXT_ALIGN_CENTER,
                                        rendering::gui::GUIModule::font_color()})
            .disable();

    flecs::entity resume_btn =
            pause_menu.child()
                    .is_a<rendering::gui::prefabs::Button>()
                    .set<Rectangle>({-125, -135, 250, 50})
                    .set<rendering::gui::Anchor>(
                            {rendering::gui::HORIZONTAL_ANCHOR::CENTER, rendering::gui::VERTICAL_ANCHOR::BOTTOM})
                    .set<rendering::gui::ButtonCallback>({[pause_menu] { pause_menu.add<core::Close>(); }})
                    .disable();

    resume_btn.get_mut<rendering::gui::Text>().text = "Resume Game";

#ifndef EMSCRIPTEN
    flecs::entity close_btn =
            pause_menu.child()
                    .is_a<rendering::gui::prefabs::Button>()
                    .set<Rectangle>({-125, -75, 250, 50})
                    .set<rendering::gui::Anchor>(
                            {rendering::gui::HORIZONTAL_ANCHOR::CENTER, rendering::gui::VERTICAL_ANCHOR::BOTTOM})
                    .set<rendering::gui::ButtonCallback>({[&] { m_world.add<core::ExitConfirmed>(); }})
                    .disable();

    close_btn.get_mut<rendering::gui::Text>().text = "Close Game";
#else
    resume_btn.set<Rectangle>({-125, -75, 250, 50});
#endif


    auto input_toggle = pause_menu.child().add<input::InputToggleEnable>();
    input_toggle.child().set<input::KeyBinding>({KEY_ESCAPE, 0});

    auto level_up_menu = m_world.entity("level_up_menu")
                                 .child_of(rendering::gui::get_gui_canvas(m_world))
                                 .is_a<rendering::gui::prefabs::Panel>()
                                 .set<Rectangle>({-300, -200, 600, 400})
                                 .set<rendering::gui::Anchor>({rendering::gui::CENTER, rendering::gui::MIDDLE})
                                 .add<core::PauseOnEnabled>();


    m_world.entity()
            .child_of(level_up_menu)
            .set_name("level up menu text")
            .set<Rectangle>({-150, 5, 300, 50})
            .set<rendering::gui::Anchor>({rendering::gui::CENTER, rendering::gui::TOP})
            .set<rendering::gui::Text>({"You Leveled Up, Pick an upgrade", rendering::gui::FONT_SIZE_48,
                                        TEXT_ALIGN_CENTER, rendering::gui::GUIModule::font_color()});


    level_up_menu.disable();

    player.observe<gameplay::LevelUpEvent>([exp_bar, exp_level_txt, level_up_menu](gameplay::LevelUpEvent &event) {
        exp_bar.get_mut<rendering::gui::ProgressBar>().max_val = event.threshold;
        exp_level_txt.get_mut<rendering::gui::Text>().text = "Level: " + std::to_string(event.level);
        level_up_menu.add<core::Open>();
    });

    player.observe<gameplay::LevelUpEvent>([&, spawner](gameplay::LevelUpEvent &event) {
        spawner.get_mut<gameplay::Spawner>().difficulty = event.level;
        gameplay::spawner_interval = std::max(0.0167f, gameplay::BASE_SPAWNER_INTERVAL - 2 * (event.level / 100.f));
        gameplay::m_spawner_tick.destruct();
        gameplay::m_spawner_tick = m_world.timer().interval(gameplay::spawner_interval);
        gameplay::spawn_system.set_tick_source(gameplay::m_spawner_tick);
    });

    player.observe<gameplay::ExpGainedEvent>([exp_bar](gameplay::ExpGainedEvent &event) {
        exp_bar.get_mut<rendering::gui::ProgressBar>().current_val = event.cur;
    });

    auto container = m_world.entity()
                             .child_of(level_up_menu)
                             .set_name("level up options container")
                             .set<Rectangle>({-175, -150, 350, 300})
                             .set<rendering::gui::Anchor>({rendering::gui::CENTER, rendering::gui::MIDDLE})
                             .set<rendering::gui::Outline>({1, GRAY, Fade(WHITE, 0)});

    container.child()
            .is_a<rendering::gui::prefabs::Button>()
            .set_name("Option 1")
            .set<Rectangle>({-162.5, 5, 325, 40})
            .set<rendering::gui::Anchor>({rendering::gui::CENTER, rendering::gui::TOP})
            .set<rendering::gui::Text>({"+1 Pierce", rendering::gui::FONT_SIZE_32, TEXT_ALIGN_CENTER, BLACK})
            .set<rendering::gui::ButtonCallback>({[level_up_menu] {
                gameplay::add_pierce.run();
                gameplay::add_pierce_amt.run();
                level_up_menu.add<core::Close>();
            }});

    container.child()
            .is_a<rendering::gui::prefabs::Button>()
            .set_name("Option 2")
            .set<Rectangle>({-162.5, 50, 325, 40})
            .set<rendering::gui::Anchor>({rendering::gui::CENTER, rendering::gui::TOP})
            .set<rendering::gui::Text>({"+1 Chain", rendering::gui::FONT_SIZE_32, TEXT_ALIGN_CENTER, BLACK})
            .set<rendering::gui::ButtonCallback>({[level_up_menu] {
                gameplay::add_chain.run();
                gameplay::add_chain_amt.run();
                level_up_menu.add<core::Close>();
            }});

    container.child()
            .is_a<rendering::gui::prefabs::Button>()
            .set_name("Option 3")
            .set<Rectangle>({-162.5, 95, 325, 40})
            .set<rendering::gui::Anchor>({rendering::gui::CENTER, rendering::gui::TOP})
            .set<rendering::gui::Text>({"+1 Projectile", rendering::gui::FONT_SIZE_32, TEXT_ALIGN_CENTER, BLACK})
            .set<rendering::gui::ButtonCallback>({[level_up_menu] {
                gameplay::add_multiproj.run();
                gameplay::add_proj.run();
                level_up_menu.add<core::Close>();
            }});

    container.child()
            .is_a<rendering::gui::prefabs::Button>()
            .set_name("Option 4")
            .set<Rectangle>({-162.5, 140, 325, 40})
            .set<rendering::gui::Anchor>({rendering::gui::CENTER, rendering::gui::TOP})
            .set<rendering::gui::Text>({"+1 Bounce", rendering::gui::FONT_SIZE_32, TEXT_ALIGN_CENTER, BLACK})
            .set<rendering::gui::ButtonCallback>({[level_up_menu] {
                gameplay::add_bounce.run();
                gameplay::add_bounce_amt.run();
                level_up_menu.add<core::Close>();
            }});

    flecs::entity split_level_up =
            container.child()
                    .is_a<rendering::gui::prefabs::Button>()
                    .set_name("Option 5")
                    .set<Rectangle>({-162.5, 185, 325, 40})
                    .set<rendering::gui::Anchor>({rendering::gui::CENTER, rendering::gui::TOP})
                    .set<rendering::gui::Text>({"Projectiles can split", 32, TEXT_ALIGN_CENTER, BLACK});

    split_level_up.set<rendering::gui::ButtonCallback>({[level_up_menu, split_level_up] {
        gameplay::add_split.run();
        level_up_menu.add<core::Close>();
        split_level_up.destruct();
    }});

    container.add<core::Close>();
}

void Game::run() {

    // ON START
    m_world.progress();
    // m_world.progress(GetFrameTime());

#if defined(EMSCRIPTEN)
    emscripten_set_main_loop_arg(m_world.progress(), this, 0, 1);
#else
    std::vector<std::chrono::microseconds> delta_times;
    std::vector<int> frameRates;
    std::vector<int> entities;
#ifdef __linux__ 
    std::vector<double> cache_refs;
    std::vector<double> cache_miss;
#endif
    int frames = 0;
    // Main game loop
    // Main game loop
#ifdef __linux__

    auto config = perf::Config{};
    config.max_groups(12U);             /// Only two hardware counters
    config.max_counters_per_group(1U); /// Only one event per counter.
    const auto counter_definition = perf::CounterDefinition{};
    auto event_counter = perf::EventCounter{ counter_definition, config };
    try {
        event_counter.add(std::vector<std::string>{"cpu_core/cache-references","cpu_core/cache-misses"});
        event_counter.add_live(std::vector<std::string>{"cache-references", "cache-misses" });

    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
    }

    auto live_events = perf::LiveEventCounter{ event_counter };

    try {
        event_counter.start();
    } catch (std::runtime_error& exception) {
        std::cout << "wtf" << std::endl;
        std::cerr << exception.what() << std::endl;
        return;
    }
#endif
    while (!WindowShouldClose() && !m_world.has<core::ExitConfirmed>()) // Detect window close button or ESC key
    {
        auto start = std::chrono::high_resolution_clock::now();
        #ifdef __linux__
    live_events.start();
#endif
        UpdateDrawFrameDesktop();
#ifdef __linux__ 
       
        live_events.stop();
#endif
        auto end = std::chrono::high_resolution_clock::now();
        delta_times.push_back(std::chrono::duration_cast<std::chrono::microseconds>(end - start));
#ifdef __linux__
        cache_refs.push_back(live_events.get("cache-references"));
        cache_miss.push_back(live_events.get("cache-misses"));
#endif
        entities.push_back(m_world.query<core::Position2D>().count());
        frames++;
        // std::cout << live_events.get("cache-references") << " cache references, " <<  live_events.get("cache-misses") << " cache-misses, " << live_events.get("cache-misses") / live_events.get("cache-references") << " cache-miss-ratio" << std::endl;


        auto time = 0L;
        int count = 0;
        if (frames < 2) {
            frameRates.push_back(0);
            continue;
        }
        for (int i = 1; i < frames; i++) {
            time += delta_times[frames - i].count();
            count++;
            if (time >= 1000000)
                break;
        }

        frameRates.push_back(count / (time / 1000000.0f));

        if (time >= 1000000 && count <= 30) {
            //std::cout << time << std::endl;
            //std::cout << count << std::endl;
            //std::string target = std::format("../../results/{}/{}-{}.png", m_windowName, m_windowName, rep);
            //std::string name = std::format("{}-{}.png", m_windowName, rep);
            //TakeScreenshot(name.c_str());
            //try {
            //    if (std::filesystem::exists(target))
            //        std::filesystem::remove(target);
            //    std::filesystem::copy(name, target);
            //    std::filesystem::remove(name);
            //} catch (std::exception &e) {
            //    std::cout << "couldn't write png" << e.what() << std::endl;
            //}
            break;
        }
    }
#ifdef __linux__ 
    event_counter.stop();
    std::cout << event_counter.result().to_string() << std::endl;
#endif

    m_world.quit();
    m_world.progress();

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    physics::PhysicsModule::reset_systems_list();
    //--------------------------------------------------------------------------------------
    for (auto module: modules) {
        module.destruct();
    }
    modules.clear();
    std::cout << "inner reset: refcount = " << flecs_poly_refcount(m_world) << std::endl;
    m_world.reset();
    #ifdef __linux__
    try {
    //--------------------------------------------------------------------------------------
    std::stringstream filename_stream;
    filename_stream << "../../results/" << m_windowName << "/" << m_windowName << "-" << rep << ".txt";
        //std::cout << event_counter.result().to_string() << std::endl;
    if (std::ofstream file(filename_stream.str()); file.is_open()) {
        file << "frame" << "," << "nb of entities" << "," << "fps" << "," << "frame length" << "," << "cache-references" << "," << "cache-misses" << "," << "cache-miss-ratio" << "\n";
        for (int i = 0; i < frames; i++) {
            file << i << "," << entities[i] << "," << frameRates[i] << "," << delta_times[i].count() << "," << cache_refs[i] << "," << cache_miss[i] << "," << cache_miss[i] / cache_refs[i] << "\n";
        }
        file.close();
    } else {
        printf("Failed to open file %s\n", filename_stream.str().c_str());
    }
}
catch(std::exception& e) {
    std::cout << "could not write results" << e.what() << std::endl;
}
#endif 
#endif
}
void Game::set_collision_strategy(physics::PHYSICS_COLLISION_STRATEGY strategy) {
    physics::PhysicsModule::set_collision_strategy(strategy);
}

void Game::UpdateDrawFrameDesktop() { m_world.progress(GetFrameTime()); }

void Game::UpdateDrawFrameWeb(void *game) {
    Game *instance = static_cast<Game *>(game);
    instance->m_world.progress(GetFrameTime());
}
