// SPDX-FileCopyrightText: Copyright 2021-2025 Guyeon Yu <copyrat90@gmail.com>
// SPDX-License-Identifier: Zlib

#include "ibn_sprite_text_typewriter.h"

#include <bn_array.h>
#include <bn_core.h>
#include <bn_display.h>
#include <bn_keypad.h>
#include <bn_log.h>
#include <bn_sprite_actions.h>
#include <bn_sprite_builder.h>
#include <bn_sprite_ptr.h>
#include <bn_sprite_text_generator.h>
#include <bn_vector.h>

#include "common_variable_8x16_sprite_font.h"

#include "bn_sound_items.h"
#include "bn_sprite_items_cursor.h"
#include "bn_sprite_items_width_guideline.h"
#include "bn_sprite_palette_items_blue.h"
#include "bn_sprite_palette_items_red.h"

namespace
{

constexpr auto RESUME_KEY = bn::keypad::key_type::A;
constexpr auto SKIP_KEY = (bn::keypad::key_type)((int)bn::keypad::key_type::SELECT | (int)bn::keypad::key_type::B);

constexpr bn::string_view STR = R"(* ➊Hello!⏯
⓿* And.⓵.⓹.⓾ ❷good-bye!⓿
the quick brown fox jumps over a lazy dog,
THE QUICK BROWN FOX JUMPS OVER A LAZY DOG?
zlib License⓪
Copyright 2021-2025 Guyeon Yu <copyrat90@gmail.com>➀
This software is provided 'as-is', without any express or implied②
warranty.  In no event will the authors be held liable for any damages➂
arising from the use of this software.④
Permission is granted to anyone to use this software for any purpose,⑤
including commercial applications, and to alter it and redistribute it➅
freely, subject to the following restrictions:⑦
1. The origin of this software must not be misrepresented; you must not➇
   claim that you wrote the original software. If you use this software⑨
   in a product, an acknowledgment in the product documentation would be➉
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.)";

constexpr int WAIT_UPDATES = 3;
constexpr int LINE_WIDTH = 220;
constexpr bn::fixed LINE_SPACING = 12;
constexpr int MAX_LINES = 4;

constexpr bn::array<const bn::sprite_palette_item*, 3> PALETTES = {
    &bn::sprite_items::common_variable_8x16_font.palette_item(), // 🄌
    &bn::sprite_palette_items::blue,                             // ➊
    &bn::sprite_palette_items::red,                              // ➋
};

constexpr bn::fixed CENTER_X = bn::display::width() / 2;
constexpr bn::fixed LEFT_X = CENTER_X - LINE_WIDTH / 2;
constexpr bn::fixed RIGHT_X = CENTER_X + LINE_WIDTH / 2;

constexpr bn::fixed PARA_SPACING = 55;

using out_vec_t = bn::vector<bn::sprite_ptr, 32>;

void custom_delegate_callback(int dele_idx)
{
    BN_LOG("custom_delegate_callback(", dele_idx, ")");
}

const bn::array<const ibn::function<void(int)>, 11> DELEGATES = {
    custom_delegate_callback, custom_delegate_callback, custom_delegate_callback, custom_delegate_callback,
    custom_delegate_callback, custom_delegate_callback, custom_delegate_callback, custom_delegate_callback,
    custom_delegate_callback, custom_delegate_callback, custom_delegate_callback,
};

} // namespace

int main()
{
    bn::core::init();

    // Show yellow guidelines
    bn::sprite_builder guideline_builder(bn::sprite_items::width_guideline);
    bn::vector<bn::sprite_ptr, 20> guidelines;
    for (int y = 0; y < 5; ++y)
    {
        guideline_builder.set_top_left_position(LEFT_X, y * guideline_builder.shape_size().height());
        guidelines.push_back(guideline_builder.build());
    }
    for (int y = 0; y < 5; ++y)
    {
        guideline_builder.set_top_left_position(RIGHT_X, y * guideline_builder.shape_size().height());
        guidelines.push_back(guideline_builder.build());
    }
    for (int y = 0; y < 5; ++y)
    {
        guideline_builder.set_top_left_position(CENTER_X, y * guideline_builder.shape_size().height());
        guidelines.push_back(guideline_builder.build());
    }

    // Show cursors
    bn::sprite_builder cursor_builder(bn::sprite_items::cursor);
    cursor_builder.set_visible(false);
    bn::vector<bn::sprite_visible_toggle_action, 3> cursors;
    for (int i = 0; i < 3; ++i)
    {
        cursors.emplace_back(cursor_builder.build(), 10);
        const_cast<bn::sprite_ptr&>(cursors.back().sprite()).set_visible(false);
    }

    // Create left, center, right typewriters
    bn::sprite_text_generator text_generator(common::variable_8x16_sprite_font);
    ibn::sprite_text_typewriter left_writer(text_generator, RESUME_KEY, SKIP_KEY,
                                            bn::span(PALETTES.cbegin(), PALETTES.cend()),
                                            bn::span(DELEGATES.cbegin(), DELEGATES.cend()));
    ibn::sprite_text_typewriter center_writer(text_generator, RESUME_KEY, SKIP_KEY,
                                              bn::span(PALETTES.cbegin(), PALETTES.cend()),
                                              bn::span(DELEGATES.cbegin(), DELEGATES.cend()));
    ibn::sprite_text_typewriter right_writer(text_generator, RESUME_KEY, SKIP_KEY,
                                             bn::span(PALETTES.cbegin(), PALETTES.cend()),
                                             bn::span(DELEGATES.cbegin(), DELEGATES.cend()));

    out_vec_t out_left, out_center, out_right;

    // Start typewriters
    // (Initial alignment is stored and used throughout typewritting)
    text_generator.set_alignment(bn::sprite_text_generator::alignment_type::LEFT);
    left_writer.start(LEFT_X, 0 * PARA_SPACING, STR, out_left, WAIT_UPDATES, nullptr, LINE_WIDTH, LINE_SPACING,
                      MAX_LINES);
    text_generator.set_alignment(bn::sprite_text_generator::alignment_type::CENTER);
    center_writer.start(CENTER_X, 1 * PARA_SPACING, STR, out_center, WAIT_UPDATES, nullptr, LINE_WIDTH, LINE_SPACING,
                        MAX_LINES);
    text_generator.set_alignment(bn::sprite_text_generator::alignment_type::RIGHT);
    right_writer.start(RIGHT_X, 2 * PARA_SPACING, STR, out_right, WAIT_UPDATES, &bn::sound_items::type, LINE_WIDTH,
                       LINE_SPACING, MAX_LINES);

    while (true)
    {
        if (!left_writer.done())
            left_writer.update();
        if (!center_writer.done())
            center_writer.update();
        if (!right_writer.done())
            right_writer.update();

        if (left_writer.paused())
        {
            const_cast<bn::sprite_ptr&>(cursors[0].sprite())
                .set_top_left_position(left_writer.next_character_position());
            cursors[0].update();
        }
        else
            const_cast<bn::sprite_ptr&>(cursors[0].sprite()).set_visible(false);

        if (center_writer.paused())
        {
            const_cast<bn::sprite_ptr&>(cursors[1].sprite())
                .set_top_left_position(center_writer.next_character_position());
            cursors[1].update();
        }
        else
            const_cast<bn::sprite_ptr&>(cursors[1].sprite()).set_visible(false);

        if (right_writer.paused())
        {
            const_cast<bn::sprite_ptr&>(cursors[2].sprite())
                .set_top_left_position(right_writer.next_character_position());
            cursors[2].update();
        }
        else
            const_cast<bn::sprite_ptr&>(cursors[2].sprite()).set_visible(false);

        bn::core::update();
    }
}
