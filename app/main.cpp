#include <iostream>
#include "game.h"
#ifdef __linux__
#include <perfcpp/event_counter.h>
#endif
#include <errno.h>
#include <flecs.h>
#include <fstream>
#include <linux/perf_event.h>
#include <perfcpp/event_counter.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>
int main() {


#ifdef EMSCRIPTEN
    const int screenWidth = 1920;
    const int screenHeight = 1080;
#else
    const int screenWidth = 1920;
    const int screenHeight = 1080;
#endif

#ifdef __linux__

#endif

    std::string titles[7] = {
            "collision-relationship", "collision-relationship-dontfragment", "collision-entity", "record-list",
            "spatial-hash-per-cell", "spatial-hash-per-entity" ,         "spatial-hash-relationship",
    };
    for (int i = 0; i < 30; i++) {
        for (int strategy = 0; strategy < 7; strategy++) {

            if (strategy == 1 || strategy == 6)
                continue;

            Game game = Game(titles[strategy].c_str(), screenWidth, screenHeight, i);
            game.init();
            game.set_collision_strategy(static_cast<physics::PHYSICS_COLLISION_STRATEGY>(strategy));
            game.run();
        }
    }
    return 0;
}
