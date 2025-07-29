#include <iostream>
#include "game.h"
#ifdef __linux__
#include <perfcpp/event_counter.h>
#endif
#include <flecs.h>
#include <fstream>
struct NonFragmentingCollidedWith {};
struct Collider {};
struct RemoveAfterFrame {};
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

    std::string titles[6] = {
            "collision-relationship", "collision-relationship-dontfragment", "collision-entity", "record-list",
            "spatial-hash",           "spatial-hash-relationship",
    };
    for (int i = 0; i < 1; i++) {
        for (int strategy = 0; strategy < 6; strategy++) {

            if (strategy == 1) continue;
            Game game = Game(titles[strategy].c_str(), screenWidth, screenHeight);
            game.init();
            game.set_collision_strategy(static_cast<physics::PHYSICS_COLLISION_STRATEGY>(strategy));
            game.set_rep(i);
            game.run();
        }
    }
    return 0;
}
