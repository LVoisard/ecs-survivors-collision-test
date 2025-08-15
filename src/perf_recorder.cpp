//
// Created by ubuntu on 2025-08-06.
//

#include "perf_recorder.h"

#include <fstream>
#include <iostream>
#include <utility>

#include "modules/engine/core/components.h"
#include "modules/engine/physics/components.h"

#include <flecs/addons/stats.h>

#include "modules/engine/physics/physics_module.h"
static std::vector<std::string> event_names_non_live = { "L1-dcache-loads", "L1-dcache-load-misses"};
static std::vector<std::string> event_names_live = {"L1-dcache-loads", "L1-dcache-load-misses"};
PerfRecorder::PerfRecorder(const perf::CounterDefinition &def) {
    m_event_counter = std::make_unique<perf::EventCounter>(def);
    m_event_counter->add(event_names_non_live);
    m_event_counter->add_live(static_cast<std::vector<std::string>>(event_names_live));

    m_live_event_counter = std::make_unique<perf::LiveEventCounter>(*m_event_counter);
}
PerfRecorder::~PerfRecorder() { m_counters.clear(); }

void PerfRecorder::init() {}
void PerfRecorder::close() {

    m_event_counter->close();

    // m_event_counter->release();
    // m_live_event_counter->release();
    // m_counters.clear();
}
void PerfRecorder::start_recording() { m_event_counter->start(); }
void PerfRecorder::stop_recording() { m_event_counter->stop(); }
void PerfRecorder::start_live_recording() {
    m_live_start = std::chrono::high_resolution_clock::now();
    m_live_event_counter->start();
}
void PerfRecorder::save_frame(flecs::world world, int fps) {
    double update, detection, resolution, event, cleanup, total;
    update = physics::get_update_time();
    detection = physics::get_detection_time();
    resolution = physics::get_resolution_time();
    event = physics::get_event_time();
    cleanup = physics::get_cleanup_time();
    total = update + detection + resolution + event + cleanup;


    double a = m_live_event_counter->get("L1-dcache-loads");
    double b = m_live_event_counter->get("L1-dcache-load-misses");
    auto map = std::vector<std::string>{std::to_string(m_counters.size()),
                                        std::to_string(world.query<core::Position2D, physics::Collider>().count()),
                                        std::to_string(fps),
                                        std::to_string(dt),
                                        std::to_string(total),
                                        std::to_string(a),
                                        std::to_string(b),
                                        std::to_string(b / a)};

    m_counters.push_back(map);
}
void PerfRecorder::stop_live_recording() {
    m_live_event_counter->stop();
    auto now = std::chrono::high_resolution_clock::now();

    dt = std::chrono::duration<double>(now - m_live_start).count();
}
void PerfRecorder::dump_data(std::string file_dir, std::string file_name) {
    std::cout << m_event_counter->result().to_string() << std::endl;
    std::cout << m_event_counter->result().get("L1-dcache-load-misses").value() / m_event_counter->result().get("L1-dcache-loads").value() << std::endl;

    try {
        if (std::ofstream file(file_dir + file_name); file.is_open()) {
            file << "frame" << "," << "nb of entities" << "," << "FPS" << "," << "frame length" << ","
                 << "physics length" << "," << "L1-dcache-loads" << ","
                 << "L1-dcache-load-misses" << "," << "L1-dcache-load-miss-ratio" << "\n";
            for (int i = 0; i < m_counters.size(); i++) {
                for (int j = 0; j < m_counters[i].size(); j++) {
                    file << m_counters[i][j];
                    if (j < m_counters[i].size() - 1) {
                        file << ",";
                    }
                }
                file << "\n";
            }
            file.close();
        } else {
            printf("Failed to open file %s\n", std::string(file_dir + file_name).c_str());
        }
    } catch (std::exception &e) {
        std::cout << "could not write results" << e.what() << std::endl;
    }
}
