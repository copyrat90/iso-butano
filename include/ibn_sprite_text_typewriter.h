// SPDX-FileCopyrightText: Copyright 2021-2025 Guyeon Yu <copyrat90@gmail.com>
// SPDX-License-Identifier: Zlib

#pragma once

#include <bn_fixed_point.h>
#include <bn_optional.h>
#include <bn_sound_handle.h>
#include <bn_span.h>
#include <bn_sprite_ptr.h>
#include <bn_sprite_text_generator.h>
#include <bn_string.h>
#include <bn_string_view.h>
#include <bn_vector.h>

#include <cstdint>

namespace bn
{

class sprite_font;

class sound_item;

namespace keypad
{
enum class key_type : std::uint16_t;
}

} // namespace bn

namespace ibn
{

/// @brief Typewrites sprites containing text.
///
/// Some characters are treated specially:
/// * `\n`: Newline.
/// * `⏯`: Pause until `resume_key` is pressed.
/// * `⓵`, `⓶`, ..., `⓾`: Pause for `N * wait_updates`.
/// * `⓿`, `❶`, ..., `❿`: Change the palette index.
class sprite_text_typewriter
{
public:
    static constexpr int PALETTES_MAX_SIZE = 11;

public:
    /// @brief Constructor.
    /// @note `text_generator` is not copied but referenced, so it should outlive
    /// `sprite_text_typewriter` to avoid dangling reference.
    /// @param text_generator Sprite text generator to reference.
    /// @param resume_key Key to press to resume the manual pause.
    /// @param skip_key Key to press to skip the typewritting, and render all right away.
    /// @param palettes Palettes to be used for the text. Index `0` is the initial palette. Size can't exceed `11`.
    sprite_text_typewriter(const bn::sprite_text_generator& text_generator, bn::keypad::key_type resume_key = {},
                           bn::keypad::key_type skip_key = {},
                           const bn::span<const bn::sprite_palette_item* const>& palettes = {});

public:
    /// @brief Updates the typewritting.
    void update();

    /// @brief Indicates if the typewritting is paused manually, waiting for `resume_key` to be pressed.
    bool paused() const;

    /// @brief Indicates if the typewritting is completed.
    bool done() const;

    /// @brief Indicates if the waiting is skipped, and rendered all in an instant.
    bool skipped() const;

    /// @brief Gets the position of the next rendered character.
    /// @note This doesn't consider word-wrap, so it might differ from the *real* next character position.
    auto next_character_position() const -> bn::fixed_point;

public:
    /// @brief Starts typewritting text sprites.
    /// @param top_left_x Horizontal top-left position of the first generated sprite, considering the current alignment.
    /// @param top_left_y Vertical top-left position of the first generated sprite, considering the current alignment.
    /// @param wait_updates Number of times that the typewriter needs to be updated to render the next character.
    /// @param write_sound Sound to play when writting non-whitespace character. Can be `nullptr`.
    /// @param line_width Maximum width of a text line in pixels.
    /// @param line_spacing Vertical space between lines in pixels.
    void start(bn::fixed top_left_x, bn::fixed top_left_y, const bn::string_view& text,
               bn::ivector<bn::sprite_ptr>& output_sprites, int wait_updates, const bn::sound_item* write_sound,
               int line_width, bn::fixed line_spacing)
    {
        start(bn::fixed_point(top_left_x, top_left_y), text, output_sprites, wait_updates, write_sound, line_width,
              line_spacing);
    }

    /// @brief Starts typewritting text sprites.
    /// @param top_left_position Top-left position of the first generated sprite, considering the current alignment.
    /// @param wait_updates Number of times that the typewriter needs to be updated to render the next character.
    /// @param write_sound Sound to play when writting non-whitespace character. Can be `nullptr`.
    /// @param line_width Maximum width of a text line in pixels.
    /// @param line_spacing Vertical space between lines in pixels.
    void start(const bn::fixed_point& top_left_position, const bn::string_view& text,
               bn::ivector<bn::sprite_ptr>& output_sprites, int wait_updates, const bn::sound_item* write_sound,
               int line_width, bn::fixed line_spacing);

public:
    auto resume_key() const -> bn::keypad::key_type;
    void set_resume_key(bn::keypad::key_type resume_key);

    auto skip_key() const -> bn::keypad::key_type;
    void set_skip_key(bn::keypad::key_type skip_key);

private:
    void stop_write_sound();

private:
    bool new_sprite_required() const;
    void flag_new_sprite_required();

    void move_to_newline();

    void change_palette_index(int palette_index);

    bool check_word_wrap() const;

private:
    const bn::sprite_text_generator& _text_generator;
    const int _max_chunk_width;
    const bn::vector<const bn::sprite_palette_item*, PALETTES_MAX_SIZE> _palettes;
    bn::keypad::key_type _resume_key;
    bn::keypad::key_type _skip_key;

    bn::fixed_point _init_position;
    bn::string_view _text;
    bn::ivector<bn::sprite_ptr>* _output_sprites;
    int _init_sprite_index = 0;
    int _wait_updates;
    const bn::sound_item* _write_sound;
    int _max_line_width;
    bn::fixed _line_spacing;
    bn::sprite_text_generator::alignment_type _alignment;

    bool _prev_whitespace = true;
    bool _skip_requested = false;
    bool _paused_manual = false;
    int _paused_remaining_delay = 0;
    bn::fixed _current_line_y = 0;
    int _palette_index = 0;
    int _current_updates = 0;
    bn::optional<bn::sound_handle> _write_sound_handle;
    int _current_line_width = 0;
    int _current_chunk_width = 0;
    int _text_char_index = 0;
    bn::string<65 * 4> _text_chunk; // (max sprite width + 1) * (max utf-8 octet)
    int _line_first_sprite_index = 0;
    int _sprite_index = 0;
};

} // namespace ibn
