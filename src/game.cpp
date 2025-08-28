//
// Created by Laurent Voisard on 12/20/2024.
//

#include "game.h"
#include <iostream>
#include <ostream>

#include "perf_recorder.h"
#ifdef __linux__

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

Game::Game(const char *windowName, int windowWidth, int windowHeight, int rep) :
    m_windowName(windowName), m_windowHeight(windowHeight), m_windowWidth(windowWidth), rep(rep) {}


void Game::init() {
    m_world = flecs::world();
    // Raylib window
    // #ifndef EMSCRIPTEN
    // web has an scaling issue with the cursor
    // SetConfigFlags();
    // #endif
    SetTraceLogLevel(LOG_ERROR);
    InitWindow(m_windowWidth, m_windowHeight, (m_windowName+"-"+std::to_string(rep)).c_str());

    SetExitKey(KEY_F4);
    SetWindowFocused();
    SetTargetFPS(300);
#ifndef EMSCRIPTEN
    // use the flecs explorer when not on browser
    m_world.import <flecs::stats>();
    m_world.set<flecs::Rest>({});
    // m_world.set_threads(static_cast<int>(std::thread::hardware_concurrency()));
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
    m_world.set<physics::SpatialHashingGrid>({32, {0, 0}});
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
                                           false,
                                           true,
                                           {-16, -16, 32, 32},
                                           physics::CollisionFilter::player,
                                           physics::player_filter,
                                           physics::ColliderType::Circle,
                                   })
                                   .set<physics::CircleCollider>({16})
                                   .set<rendering::Priority>({2})
                                   .set<rendering::Renderable>({LoadTexture("../resources/player.png"), // 8x8
                                                                {0, 0},
                                                                2.f,
                                                                WHITE})
                                   .set<gameplay::Experience>({1, 0, 100000});


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
                                                           {-16, -16, 32, 32},
                                                           physics::CollisionFilter::enemy,
                                                           physics::enemy_filter,
                                                           physics::ColliderType::Circle})
                                  .set<physics::CircleCollider>({16})
                                  .set<rendering::Renderable>({LoadTexture("../resources/ghost.png"), // 8x8
                                                               {0, 0},
                                                               2.f,
                                                               WHITE})
                                  .set<rendering::Priority>({0});

    auto spawner = m_world.entity("enemy_spawner").set<gameplay::Spawner>({enemy, 1});

    m_world.set<rendering::TrackingCamera>({player, Camera2D{0}});
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
#endif
    // Main game loop
#ifdef __linux__
    {
        int frames = 0;
        const int frame_capture_count = 60;
        float frame_times_history[frame_capture_count];
        std::fill(std::begin(frame_times_history), std::end(frame_times_history), (1.0 / 300.0) / 60.0);

        float average_frame = 1.0 / 300.0;

        const auto counter_definition = perf::CounterDefinition{};
        auto recorder = PerfRecorder{counter_definition};

        recorder.init();
        recorder.start_recording();
#endif
        while (!WindowShouldClose() && !m_world.has<core::ExitConfirmed>()) // Detect window close button or ESC key
        {
            frames++;
#ifdef __linux__
            recorder.start_live_recording();
#endif
            UpdateDrawFrameDesktop();
#ifdef __linux__


            recorder.stop_live_recording();

            int index = (frames + 1) % frame_capture_count;
            average_frame -= frame_times_history[index];
            frame_times_history[index] = recorder.get_dt() / frame_capture_count;
            average_frame += frame_times_history[index];
            int fps = (1.f / average_frame);

            recorder.save_frame(m_world, (int) (1.f / average_frame));

            if (fps  < frame_capture_count) { // greater than 33 ms
                if (frames > 60) {
                    // std::cout << "fps dropped below 60: " << frames << "," << 1.f / average_frame << std::endl;
                    break;
                }
            }


#endif
            // std::cout << 1.f / average_frame << std::endl;
            // std::cout << recorder.get_dt() << std::endl;
            DrawText(std::to_string((int) (1.0f / average_frame)).c_str(), 10, 30, 30, LIME);
        }
#ifdef __linux__
        recorder.stop_recording();
        recorder.close();
        std::stringstream filepath_stream;
        filepath_stream << "../../results/" << m_windowName << "/";
        std::stringstream filename_stream;
        filename_stream << m_windowName << "-" << rep << ".txt";
        //recorder.dump_data(filepath_stream.str(), filename_stream.str());


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
        // recorder.close();
        modules.clear();
        // std::cout << "inner reset: refcount = " << flecs_poly_refcount(m_world) << std::endl;
        m_world.reset();
        frames = 0;
    }
}

#endif

    void Game::set_collision_strategy(physics::PHYSICS_COLLISION_STRATEGY strategy) {
        physics::PhysicsModule::set_collision_strategy(strategy);
    }

    void Game::UpdateDrawFrameDesktop() { m_world.progress(GetFrameTime()); }

    void Game::UpdateDrawFrameWeb(void *game) {
        Game *instance = static_cast<Game *>(game);
        instance->m_world.progress(GetFrameTime());
    }
