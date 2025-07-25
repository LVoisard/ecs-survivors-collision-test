﻿//
// Created by Laurent on 12/29/2024.
//

#include "input_module.h"

#include "components.h"
#include "systems/reset_horizontal_input_system.h"
#include "systems/reset_vertical_input_system.h"
#include "systems/set_horizontal_input_system.h"
#include "systems/set_vertical_input_system.h"
#include "systems/toggle_element_on_input_system.h"

namespace input {
    void InputModule::register_components(flecs::world &world) {
        world.component<InputHorizontal>();
        world.component<InputVertical>();
        world.component<KeyBinding>();
    }

    void InputModule::register_systems(flecs::world &world) {

        world.system<const KeyBinding, InputHorizontal>("set horizontal input")
                .term_at(1).cascade()
                .kind(flecs::PreUpdate)
                .each(systems::set_horizontal_input_system);

        world.system<const KeyBinding, InputVertical>("set vertical input")
                .term_at(1).cascade()
                .kind(flecs::PreUpdate)
                .each(systems::set_vertical_input_system);

        world.system<const KeyBinding, InputToggleEnable>("toggle object enabled")
                .term_at(1).cascade()
                .kind(flecs::PreUpdate)
                .with(flecs::Disabled).optional()
                .each(systems::toggle_element_on_input_system);

        world.system<InputHorizontal>("Reset Input Horizontal")
                .kind(flecs::PostUpdate)
                .each(systems::reset_horizontal_input_system);

        world.system<InputVertical>("Reset Input Vertical")
                .kind(flecs::PostUpdate)
                .each(systems::reset_vertical_input_system);
    }
}
