//
// Created by ubuntu on 2025-08-06.
//

#ifndef ECS_SURVIVORS_PERF_RECORDER_H
#define ECS_SURVIVORS_PERF_RECORDER_H
#include <string>
#include <perfcpp/event_counter.h>
#include <unordered_map>
#include <vector>

#include <filesystem>
#include <fstream>

#include "flecs.h"

class PerfRecorder {
public:
    PerfRecorder(const perf::Config &config, const perf::CounterDefinition &def);
    ~PerfRecorder();

    void init();
    void close();

    void start_recording();

    void stop_recording();

    void start_live_recording();
    void stop_live_recording(flecs::world);

    void dump_data(std::string file_dir, std::string file_name);
private:

    std::vector<std::vector<std::string>> m_counters;

    perf::EventCounter m_event_counter;
    perf::LiveEventCounter m_live_event_counter;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_live_start;
};


#endif // ECS_SURVIVORS_PERF_RECORDER_H
