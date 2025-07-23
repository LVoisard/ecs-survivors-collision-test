#include "game.h"
#include <iostream>
#ifdef __linux__
#include <perfcpp/event_counter.h>
#endif
int main() {
#ifdef EMSCRIPTEN
    const int screenWidth = 1920;
    const int screenHeight = 1080;
#else
    const int screenWidth = 1920;
    const int screenHeight = 1080;
#endif

    std::string titles[6] = {
        "collision-relationship",
        "collision-relationship-dontfragment",
        "collision-entity",
        "record-list",
        "spatial-hash",
        "spatial-hash-relationship",
    };
    for (int i = 0; i < 1; i++) {
        for (int strategy = 0; strategy < 6; strategy++) {
#ifdef __linux__
            const auto counter_def = perf::CounterDefinition();
            auto event_counter = perf::EventCounter{ counter_def };
            event_counter.add({"seconds", "instructions", "cycles", "cache-misses"});
            event_counter.start();
#endif
            Game game = Game(titles[strategy].c_str(), screenWidth, screenHeight);
            game.init();
            game.set_collision_strategy(static_cast<physics::PHYSICS_COLLISION_STRATEGY>(strategy));
            game.run();
#ifdef __linux__
            event_counter.stop();

            std::cout << "============================== EVENTS ==============================";
            const auto result = event_counter.result();
            for (const auto [event_name, value] : result)
            {
                std::cout << event_name << ": " << value << std::endl;
            }
            std::cout << "========================== END OF EVENTS ===========================";
#endif
        }
    }
    return 0;
}
