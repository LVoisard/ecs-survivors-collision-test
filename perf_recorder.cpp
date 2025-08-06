//
// Created by ubuntu on 2025-08-06.
//

#include "src/perf_recorder.h"

#include <fstream>
#include <iostream>
#include <utility>

#include "src/modules/engine/core/components.h"
#include "src/modules/engine/physics/components.h"

PerfRecorder::PerfRecorder(const perf::Config &config, const perf::CounterDefinition &def) :
    m_event_counter(def, config), m_live_event_counter(m_event_counter) {
    m_event_counter.add(std::vector<std::string>{"cache-references", "cache-misses"});
    //m_event_counter.add_live(std::vector<std::string>{"cache-references", "cache-misses"});
}
PerfRecorder::~PerfRecorder() {
    m_counters.clear();
}

void PerfRecorder::init() {}
void PerfRecorder::close() {

    m_event_counter.close();

    // m_event_counter.release();
    // m_live_event_counter.release();
    // m_counters.clear();
}
void PerfRecorder::start_recording() { m_event_counter.start(); }
void PerfRecorder::stop_recording() { m_event_counter.stop(); }
void PerfRecorder::start_live_recording() {
    m_live_start = std::chrono::high_resolution_clock::now();
    m_live_event_counter.start();
}
void PerfRecorder::stop_live_recording(flecs::world world) {
    m_live_event_counter.stop();
    //std::cout << m_live_event_counter.get("cache-references") << std::endl;
    auto now = std::chrono::high_resolution_clock::now();
    auto map = std::vector<std::string>{
            std::to_string(m_counters.size()),
            std::to_string(world.query<core::Position2D, physics::Collider>().count()),
            std::to_string((now - m_live_start).count()),
            std::to_string(m_live_event_counter.get("cache-references")),
            std::to_string(m_live_event_counter.get("cache-misses")),
    };

    m_counters.push_back(map);
}
void PerfRecorder::dump_data(std::string file_dir, std::string file_name) {
    std::cout << m_event_counter.result().to_json() << std::endl;
    try {
        if (std::ofstream file(file_dir + file_name); file.is_open()) {
            file << "frame" << "," << "nb of entities" << "," << "frame length" << "," << "cache-references" << ","
                 << "cache-misses" << "," << "cache-miss-ratio" << "\n";
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
