//
// Created by Laurent Voisard on 12/22/2024.
//

#include "gui_module.h"

#include <flecs.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "../../rendering/pipeline_steps.h"
#include "components.h"
#include "prefabs.h"
#include "systems/check_window_resized_system.h"
#include "systems/draw_interactable_textured_element_system.h"
#include "systems/draw_menu_bar_system.h"
#include "systems/draw_menu_bar_tab_item_system.h"
#include "systems/draw_menu_bar_tab_system.h"
#include "systems/draw_outline_system.h"
#include "systems/draw_panel_system.h"
#include "systems/draw_text_system.h"
#include "systems/load_style_system.h"
#include "systems/parent_rectangle_changed_disabled_observer.h"
#include "systems/parent_rectangle_changed_observer.h"
#include "systems/set_anchored_position_system.h"
#include "systems/set_gui_canvas_size_system.h"

#include "systems/disable_children_on_disable_system.h"
#include "systems/enable_children_on_enable_system.h"
#include "systems/invoke_button_callback_system.h"
#include "systems/draw_progress_bar_system.h"

#include "systems/interactable_transition_to_normal_system.h"
#include "systems/interactable_transition_to_hovered_system.h"
#include "systems/interactable_transition_to_pressed_system.h"
#include "systems/interactable_transition_to_released_system.h"

namespace rendering::gui {
    void GUIModule::register_components(flecs::world &world) {
        world.component<Text>();
        world.component<Outline>();
        world.component<Font>();
        world.component<prefabs::Panel>().add(flecs::Inheritable);
        world.component<FontAtlas>().add(flecs::Singleton);
        world.set<FontAtlas>({
            std::unordered_map<int, Font>{
                {FONT_SIZE_16, LoadFontEx("../resources/Spectral-SemiBold.ttf", FONT_SIZE_16, nullptr, 0)},
                {FONT_SIZE_32, LoadFontEx("../resources/Spectral-SemiBold.ttf", FONT_SIZE_32, nullptr, 0)},
                {FONT_SIZE_48, LoadFontEx("../resources/Spectral-SemiBold.ttf", FONT_SIZE_48, nullptr, 0)},
                {FONT_SIZE_64, LoadFontEx("../resources/Spectral-SemiBold.ttf", FONT_SIZE_64, nullptr, 0)},
            }
        });
    }

    void GUIModule::register_systems(flecs::world &world) {
        world.system().kind(flecs::OnStart).run(systems::load_style_system);

        world.system<core::GameSettings>("On start set move gui elements to match anchors")
                .kind(flecs::OnStart)
                .with(flecs::Disabled).optional()
                .each(systems::set_gui_canvas_size_system);

        world.system<const Rectangle, Anchor>("on start, set anchored position")
                .kind(flecs::OnStart)
                .with(flecs::Disabled).optional()
                .each(systems::set_anchored_position_system);

        world.observer<const Rectangle>("parent rectangle changed enabled")
                .term_at(0).up()
                .event(flecs::OnSet)
                .each(systems::on_parent_rectangle_changed_observer);

        world.observer<const Rectangle>("parent rectangle changed disabled")
                .term_at(0).parent()
                .event(flecs::OnSet)
                .with(flecs::Disabled).filter()
                .each(systems::on_parent_rectangle_changed_disabled_observer);


        world.observer()
                .event(flecs::OnAdd)
                .with(flecs::Disabled)
                .each(systems::disable_children_on_disable_system);

        world.observer()
                .event(flecs::OnRemove)
                .with(flecs::Disabled)
                .each(systems::enable_children_on_enable_system);

        world.system<const Rectangle>()
                .with<InteractableElementState>(Normal)
                .kind(flecs::PreFrame)
                .each(systems::interactable_transition_to_hovered_system);

        world.system<const Rectangle>()
                .with<InteractableElementState>(Hovered)
                .kind(flecs::PreFrame)
                .each(systems::interactable_transition_to_pressed_system);

        world.system<const Rectangle>()
                .with<InteractableElementState>(Pressed)
                .kind(flecs::PreFrame)
                .each(systems::interactable_transition_to_released_system);

        world.system<core::GameSettings>("Window Resized")
                .kind(flecs::PreFrame)
                .each(systems::check_window_resized_system);

        world.system<const TexturedElement, const Rectangle>("Draw Panel")
                .without<InteractableElement>()
                .kind<RenderGUI>()
                .each(systems::draw_panel_system);

        world.system<const TexturedElement, const InteractableElement, const Rectangle>("Draw textured interactable")
                .with<InteractableElementState>(flecs::Wildcard)
                .kind<RenderGUI>()
                .each(systems::draw_interactable_textured_element_system);

        world.system<const Text, const Rectangle, const InteractableElement*, const FontAtlas>("Draw Text")
                .kind<RenderGUI>()
                .each(systems::draw_text_system);

        world.system<const Rectangle, const Outline>("Draw Outline")
                .kind<RenderGUI>()
                .each(systems::draw_outline_system);

        world.system<const Rectangle, ProgressBar>("Draw Progress bar")
                .kind<RenderGUI>()
                .each(systems::draw_progress_bar_system);

        world.system<MenuBar>("Draw Menu Bar")
                .kind<RenderGUI>()
                .each(systems::draw_menu_bar_system);

        world.system<MenuBarTab, MenuBar>("Draw Tabs")
                .term_at(1).parent()
                .kind<RenderGUI>()
                .each(systems::draw_menu_bar_tab_system);

        world.system<MenuBarTabItem, MenuBarTab, Rectangle>("Draw Tab Items")
                .term_at(1).parent()
                .term_at(2).parent()
                .kind<RenderGUI>()
                .each(systems::draw_menu_bar_tab_item_system);

        world.system<const ButtonCallback>("On Button clicked")
                .with<InteractableElementState>(Released)
                .kind<RenderGUI>()
                .each(systems::invoke_button_callback_system);

        world.system<const Rectangle>()
                .with<InteractableElementState>(Released)
                .kind(flecs::PostFrame)
                .each(systems::interactable_transition_to_normal_system);
    }

    void GUIModule::register_entities(flecs::world &world) {
        auto panel_texture = LoadTexture("../resources/panel-010.png");
        auto button_texture = LoadTexture("../resources/panel-009.png");
        world.prefab<prefabs::Panel>().set<TexturedElement>({
            panel_texture,
            {{0, 0, (float) panel_texture.width, (float) panel_texture.height}, 16, 16, 16, 16, NPATCH_NINE_PATCH}
        });

        world.prefab<prefabs::Button>()
                .set<TexturedElement>({
                    button_texture,
                    {
                        {0, 0, (float) panel_texture.width, (float) panel_texture.height}, 16, 16, 16, 16,
                        NPATCH_NINE_PATCH
                    },
                    ColorAlpha(WHITE, 0.8)
                })
                .set<InteractableElement>({
                    ColorAlpha(BLACK, 1),
                    ColorAlpha(WHITE, 1)
                })
                .set<ButtonCallback>({
                    [] { std::cout << "You clicked me" << std::endl; }
                })
                .set<Text>({"Click me", FONT_SIZE_32, TEXT_ALIGN_CENTER, BLACK})
                .add<InteractableElementState>(Normal);


        auto a = world.entity<prefabs::GuiCanvas>(gui_canvas).set<Rectangle>({
            0, 0, static_cast<float>(GetScreenWidth()), static_cast<float>(GetScreenHeight())
        });


        world.entity(menu_bar)
                .set<MenuBar>({
                    200,
                    1,
                    GetColor(GuiGetStyle(DEFAULT, LINE_COLOR)),
                    GetColor(GuiGetStyle(BUTTON, BACKGROUND_COLOR)),
                }).disable();


    }
} // namespace rendering::gui
