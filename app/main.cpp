#include <iostream>
#include "game.h"
#ifdef __linux__
#include <perfcpp/event_counter.h>
#endif
#include <flecs.h>
struct NonFragmentingCollidedWith {};
struct Collider {};
struct RemoveAfterFrame {};
int main() {
    // flecs::world world;
    // world.component<NonFragmentingCollidedWith>().add(flecs::DontFragment);
    //
    // flecs::entity entity_prefab = world.prefab().add<Collider>();
    //
    // world.system().immediate().run([world, entity_prefab](flecs::iter &it) {
    //     if (world.query<Collider>().count() < 3) {
    //         it.world().entity().is_a(entity_prefab);
    //     }
    // });
    //
    // world.system<Collider>().immediate().each([world](flecs::entity a, Collider) {
    //     world.query<Collider>().each([&](flecs::entity b, Collider) {
    //         // assume they collided
    //         if (a.id() < b.id()) {
    //             std::cout << "collided";
    //             std::cout << " " << a.id();
    //             std::cout << " " << b.id() << std::endl;
    //             a.add<NonFragmentingCollidedWith>(b);
    //             b.add<NonFragmentingCollidedWith>(a);
    //         }
    //     });
    // });
    //
    // world.system()
    //         .immediate()
    //         .with<NonFragmentingCollidedWith>(flecs::Wildcard)
    //         .each([world](flecs::iter &it, size_t i) {
    //             flecs::entity a = it.entity(i);
    //             a.add<RemoveAfterFrame>();
    //         });
    //
    //
    // world.system().with<NonFragmentingCollidedWith>(flecs::Wildcard).immediate().each([world](flecs::iter &it, size_t
    // i) {
    //     world.remove<NonFragmentingCollidedWith>(it.entity(i));
    //     it.entity(i).remove<NonFragmentingCollidedWith>(it.pair(0));
    // });
    //
    // world.system().with<RemoveAfterFrame>().immediate().each([](flecs::iter &it, size_t i) {
    //     std::cout << "destroying " << it.entity(i).id() << std::endl;
    //
    //     it.entity(i).destruct();
    // });
    // for (int i = 0; i < 10; i++) {
    //     world.progress();
    // }
    // return 0;

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
#ifdef __linux__
            const auto counter_def = perf::CounterDefinition();
            auto event_counter = perf::EventCounter{counter_def};
            event_counter.add({"seconds", "instructions", "cycles", "cache-misses", "cache-references"});
            event_counter.start();
#endif
            Game game = Game(titles[strategy].c_str(), screenWidth, screenHeight);
            game.init();
            game.set_collision_strategy(static_cast<physics::PHYSICS_COLLISION_STRATEGY>(strategy));
            game.run();
#ifdef __linux__
            event_counter.stop();

            std::cout << "============================== EVENTS ==============================" << std::endl;
            const auto result = event_counter.result();
            for (const auto [event_name, value]: result) {
                std::cout << event_name << ": " << std::setprecision(16) << value << std::endl;
            }
            std::cout << "cache miss ratio: " << std::setprecision(4)
                      << (result["cache-misses"].value() / result["cache-references"].value()) * 100.f << "%" << std::endl;
            std::cout << "========================== END OF EVENTS ===========================";
#endif
        }
    }
    return 0;
}
