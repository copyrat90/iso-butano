#include "ibn_sprite_palette_swap_action.h"
#include "ibn_variable_wait_sprite_animate_action.h"

#include <bn_array.h>
#include <bn_bg_palettes.h>
#include <bn_core.h>
#include <bn_fixed.h>
#include <bn_format.h>
#include <bn_keypad.h>
#include <bn_sprite_ptr.h>
#include <bn_sprite_text_generator.h>
#include <bn_string_view.h>
#include <bn_vector.h>

#include "bn_sprite_items_cavegirl.h"
#include "bn_sprite_items_ninja.h"
#include "bn_sprite_palette_items_cavegirl_alt.h"

#include "common_info.h"
#include "common_variable_8x16_sprite_font.h"

namespace
{

void animation_once_scene(bn::sprite_text_generator& text_generator)
{
    constexpr bn::string_view info_text_lines[] = {
        "PAD: change sprite's direction",
        "A: update action",
        "START: go to next scene",
    };

    common::info info("Variable wait animation once", info_text_lines, text_generator);

    bn::sprite_ptr ninja_sprite = bn::sprite_items::ninja.create_sprite(0, 0);
    ibn::variable_wait_sprite_animate_action<4> action = ibn::variable_wait_sprite_animate_action<4>::once(
        ninja_sprite, bn::sprite_items::ninja.tiles_item(), bn::array<uint16_t, 4>{0, 1, 2, 3},
        bn::array<uint16_t, 4>{3, 4, 0, 2});

    bn::vector<bn::sprite_ptr, 64> action_info;
    auto update_action_info = [&] {
        action_info.clear();
        text_generator.generate(0, 15, bn::format<32>("wait_updates(): {}", action.wait_updates()), action_info);
        text_generator.generate(0, 25, bn::format<32>("current_wait_updates(): {}", action.current_wait_updates()),
                                action_info);
        text_generator.generate(0, 35, bn::format<32>("current_index(): {}", action.current_index()), action_info);
        text_generator.generate(0, 45, bn::format<32>("done(): {}", action.done()), action_info);
    };
    update_action_info();

    while (!bn::keypad::start_pressed())
    {
        if (bn::keypad::a_pressed() && !action.done())
        {
            action.update();
            update_action_info();
        }

        if (bn::keypad::left_pressed())
        {
            action = ibn::variable_wait_sprite_animate_action<4>::once(
                ninja_sprite, bn::sprite_items::ninja.tiles_item(), bn::array<uint16_t, 4>{8, 9, 10, 11},
                bn::array<uint16_t, 4>{3, 4, 0, 2});
            update_action_info();
        }
        else if (bn::keypad::right_pressed())
        {
            action = ibn::variable_wait_sprite_animate_action<4>::once(
                ninja_sprite, bn::sprite_items::ninja.tiles_item(), bn::array<uint16_t, 4>{12, 13, 14, 15},
                bn::array<uint16_t, 4>{3, 4, 0, 2});
            update_action_info();
        }

        if (bn::keypad::up_pressed())
        {
            action = ibn::variable_wait_sprite_animate_action<4>::once(
                ninja_sprite, bn::sprite_items::ninja.tiles_item(), bn::array<uint16_t, 4>{4, 5, 6, 7},
                bn::array<uint16_t, 4>{3, 4, 0, 2});
            update_action_info();
        }
        else if (bn::keypad::down_pressed())
        {
            action = ibn::variable_wait_sprite_animate_action<4>::once(
                ninja_sprite, bn::sprite_items::ninja.tiles_item(), bn::array<uint16_t, 4>{0, 1, 2, 3},
                bn::array<uint16_t, 4>{3, 4, 0, 2});
            update_action_info();
        }

        info.update();
        bn::core::update();
    }
}

void animation_forever_scene(bn::sprite_text_generator& text_generator)
{
    constexpr bn::string_view info_text_lines[] = {
        "PAD: change sprite's direction",
        "A: update action",
        "START: go to next scene",
    };

    common::info info("Variable wait animation forever", info_text_lines, text_generator);

    bn::sprite_ptr ninja_sprite = bn::sprite_items::ninja.create_sprite(0, 0);
    ibn::variable_wait_sprite_animate_action<4> action = ibn::variable_wait_sprite_animate_action<4>::forever(
        ninja_sprite, bn::sprite_items::ninja.tiles_item(), bn::array<uint16_t, 4>{0, 1, 2, 3},
        bn::array<uint16_t, 4>{3, 4, 0, 2});

    bn::vector<bn::sprite_ptr, 64> action_info;
    auto update_action_info = [&] {
        action_info.clear();
        text_generator.generate(0, 15, bn::format<32>("wait_updates(): {}", action.wait_updates()), action_info);
        text_generator.generate(0, 25, bn::format<32>("current_wait_updates(): {}", action.current_wait_updates()),
                                action_info);
        text_generator.generate(0, 35, bn::format<32>("current_index(): {}", action.current_index()), action_info);
        text_generator.generate(0, 45, bn::format<32>("done(): {}", action.done()), action_info);
    };
    update_action_info();

    while (!bn::keypad::start_pressed())
    {
        if (bn::keypad::a_pressed())
        {
            action.update();
            update_action_info();
        }

        if (bn::keypad::left_pressed())
        {
            action = ibn::variable_wait_sprite_animate_action<4>::forever(
                ninja_sprite, bn::sprite_items::ninja.tiles_item(), bn::array<uint16_t, 4>{8, 9, 10, 11},
                bn::array<uint16_t, 4>{3, 4, 0, 2});
            update_action_info();
        }
        else if (bn::keypad::right_pressed())
        {
            action = ibn::variable_wait_sprite_animate_action<4>::forever(
                ninja_sprite, bn::sprite_items::ninja.tiles_item(), bn::array<uint16_t, 4>{12, 13, 14, 15},
                bn::array<uint16_t, 4>{3, 4, 0, 2});
            update_action_info();
        }

        if (bn::keypad::up_pressed())
        {
            action = ibn::variable_wait_sprite_animate_action<4>::forever(
                ninja_sprite, bn::sprite_items::ninja.tiles_item(), bn::array<uint16_t, 4>{4, 5, 6, 7},
                bn::array<uint16_t, 4>{3, 4, 0, 2});
            update_action_info();
        }
        else if (bn::keypad::down_pressed())
        {
            action = ibn::variable_wait_sprite_animate_action<4>::forever(
                ninja_sprite, bn::sprite_items::ninja.tiles_item(), bn::array<uint16_t, 4>{0, 1, 2, 3},
                bn::array<uint16_t, 4>{3, 4, 0, 2});
            update_action_info();
        }

        info.update();
        bn::core::update();
    }
}

void palette_swap_action_scene(bn::sprite_text_generator& text_generator)
{
    constexpr bn::string_view info_text_lines[] = {
        "START: go to next scene",
    };

    common::info info("Palette swap action", info_text_lines, text_generator);

    bn::sprite_ptr cavegirl_sprite = bn::sprite_items::cavegirl.create_sprite(0, 0);
    const bn::sprite_palette_item& palette_item = bn::sprite_items::cavegirl.palette_item();
    const bn::sprite_palette_item& alt_palette_item = bn::sprite_palette_items::cavegirl_alt;
    bn::sprite_palette_ptr cavegirl_palette = cavegirl_sprite.palette();

    ibn::sprite_palette_swap_toggle_action action(cavegirl_palette, palette_item, alt_palette_item, 60);

    while (!bn::keypad::start_pressed())
    {
        action.update();

        info.update();
        bn::core::update();
    }
}

} // namespace

int main()
{
    bn::core::init();

    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    bn::bg_palettes::set_transparent_color(bn::color(16, 16, 16));

    while (true)
    {
        animation_once_scene(text_generator);
        bn::core::update();

        animation_forever_scene(text_generator);
        bn::core::update();

        palette_swap_action_scene(text_generator);
        bn::core::update();
    }
}
