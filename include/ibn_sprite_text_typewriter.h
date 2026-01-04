// SPDX-FileCopyrightText: Copyright 2021-2025 Guyeon Yu <copyrat90@gmail.com>
// SPDX-License-Identifier: Zlib

#pragma once

#include "ibn_function.h"

#include <bn_fixed_point.h>
#include <bn_generic_pool.h>
#include <bn_keypad.h>
#include <bn_optional.h>
#include <bn_sound_handle.h>
#include <bn_span.h>
#include <bn_sprite_ptr.h>
#include <bn_sprite_text_generator.h>
#include <bn_string.h>
#include <bn_string_view.h>
#include <bn_unique_ptr.h>
#include <bn_vector.h>

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>

#ifndef IBN_CFG_SPRITE_TEXT_TYPEWRITER_PALETTES_MAX_SIZE
#define IBN_CFG_SPRITE_TEXT_TYPEWRITER_PALETTES_MAX_SIZE 11
#endif

#ifndef IBN_CFG_SPRITE_TEXT_TYPEWRITER_DELEGATES_MAX_SIZE
#define IBN_CFG_SPRITE_TEXT_TYPEWRITER_DELEGATES_MAX_SIZE 11
#endif

namespace bn
{

class sprite_font;

class sound_item;

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
/// * `⓪`, `①`, ..., `⑩`: Call custom delegate.
class sprite_text_typewriter
{
public:
    static constexpr int PALETTES_MAX_SIZE = IBN_CFG_SPRITE_TEXT_TYPEWRITER_PALETTES_MAX_SIZE;
    static constexpr int DELEGATES_MAX_SIZE = IBN_CFG_SPRITE_TEXT_TYPEWRITER_DELEGATES_MAX_SIZE;

    using delegate_type = function<void(int)>;

public:
    /// @brief Constructor.
    /// @note `text_generator` is not copied but referenced, so it should outlive
    /// `sprite_text_typewriter` to avoid dangling reference.
    /// @param text_generator Sprite text generator to reference.
    /// @param resume_key Key to press to resume the pause.
    /// @param skip_key Key to press to skip the typewriting, and render all right away.
    /// @param palettes Palettes to be used for the text. Index `0` is the initial palette. Size can't exceed `11`.
    sprite_text_typewriter(const bn::sprite_text_generator& text_generator,
                           bn::keypad::key_type resume_key = bn::keypad::key_type::A,
                           bn::keypad::key_type skip_key = bn::keypad::key_type::B,
                           const bn::span<const bn::sprite_palette_item* const>& palettes = {},
                           const bn::span<const delegate_type>& delegates = {});

public:
    /// @brief Updates the typewriting.
    void update();

    /// @brief Indicates if the typewriting is paused, waiting for `resume_key` to be pressed.
    bool paused() const;

    /// @brief Pauses the typewriting.
    void pause();

    /// @brief Resumes the typewriting.
    void resume();

    /// @brief Indicates if the typewriting is completed.
    bool done() const;

    /// @brief Indicates if the typewriting is failed, due to the sprite limit.
    bool failed() const;

    /// @brief Skips the typewriting.
    void skip();

    /// @brief Gets the position of the next rendered character.
    /// @note This doesn't consider word-wrap, so it might differ from the *real* next character position.
    auto next_character_position() const -> bn::fixed_point;

public:
    /// @brief Starts typewriting text sprites.
    /// @param top_left_x Horizontal top-left position of the first generated sprite, considering the current alignment.
    /// @param top_left_y Vertical top-left position of the first generated sprite, considering the current alignment.
    /// @param wait_updates Number of times that the typewriter needs to be updated to render the next character.
    /// @param write_sound Sound to play when writing non-whitespace character. Can be `nullptr`.
    /// @param line_width Maximum width of a text line in pixels.
    /// @param line_spacing Vertical space between lines in pixels.
    /// @param max_lines Maximum number of lines allowed.
    void start(bn::fixed top_left_x, bn::fixed top_left_y, const bn::string_view& text,
               bn::ivector<bn::sprite_ptr>& output_sprites, int wait_updates, const bn::sound_item* write_sound,
               int line_width, bn::fixed line_spacing, int max_lines)
    {
        start(bn::fixed_point(top_left_x, top_left_y), text, output_sprites, wait_updates, write_sound, line_width,
              line_spacing, max_lines);
    }

    /// @brief Starts typewriting text sprites.
    /// @param top_left_position Top-left position of the first generated sprite, considering the current alignment.
    /// @param wait_updates Number of times that the typewriter needs to be updated to render the next character.
    /// @param write_sound Sound to play when writing non-whitespace character. Can be `nullptr`.
    /// @param line_width Maximum width of a text line in pixels.
    /// @param line_spacing Vertical space between lines in pixels.
    /// @param max_lines Maximum number of lines allowed.
    void start(const bn::fixed_point& top_left_position, const bn::string_view& text,
               bn::ivector<bn::sprite_ptr>& output_sprites, int wait_updates, const bn::sound_item* write_sound,
               int line_width, bn::fixed line_spacing, int max_lines);

public:
    auto resume_key() const -> bn::keypad::key_type;
    void set_resume_key(bn::keypad::key_type resume_key);

    auto skip_key() const -> bn::keypad::key_type;
    void set_skip_key(bn::keypad::key_type skip_key);

private:
    void stop_write_sound();

private:
    bool new_sprite_required() const;

    bool max_lines_overflow() const;
    void wipe_and_reset_pos();

    bool check_word_wrap() const;

    void render_chunk(int current_line_width, int new_chunk_width);

private:
    void call_custom_delegate(int delegate_index);

private:
    class state;
    class state_deleter;

    using state_ptr = bn::unique_ptr<state, state_deleter>;

    class state
    {
    public:
        virtual ~state() = default;

        virtual void enter(sprite_text_typewriter&) {};
        virtual void exit(sprite_text_typewriter&) {};

        virtual void update(sprite_text_typewriter&) = 0;

    protected:
        bool skip_if_key_pressed(sprite_text_typewriter&);
    };

    class type_state final : public state
    {
    public:
        void update(sprite_text_typewriter&) override;

    private:
        void flag_new_sprite_required(sprite_text_typewriter&);
        void move_to_newline(sprite_text_typewriter&);

        void change_palette_index(int palette_index, sprite_text_typewriter&);

    private:
        int _current_updates = 0;
        int _timed_pause_remaining = 0;
    };

    class manual_pause_state final : public state
    {
    public:
        void enter(sprite_text_typewriter&) override;
        void exit(sprite_text_typewriter&) override;

        void update(sprite_text_typewriter&) override;

    private:
        bool resume_if_key_pressed(sprite_text_typewriter&);
    };

    class skip_state final : public state
    {
    public:
        void exit(sprite_text_typewriter&) override;

        void update(sprite_text_typewriter&) override;

    private:
        void flag_new_sprite_required(sprite_text_typewriter&);
        void move_to_newline(sprite_text_typewriter&);

        void change_palette_index(int palette_index, sprite_text_typewriter&);

    private:
        bool _half_baked = false;
        bool _might_half_baked = true;
    };

    class done_state final : public state
    {
    public:
        void update(sprite_text_typewriter&) override;
    };

    static constexpr int MAX_STATE_SIZE = std::max({
        sizeof(type_state),
        sizeof(manual_pause_state),
        sizeof(skip_state),
        sizeof(done_state),
    });

    static constexpr int MAX_STATE_ALIGN = std::max({
        alignof(type_state),
        alignof(manual_pause_state),
        alignof(skip_state),
        alignof(done_state),
    });

    using state_pool_t = bn::generic_pool<MAX_STATE_SIZE, 3, MAX_STATE_ALIGN>;

    class state_deleter final
    {
    public:
        state_deleter() = default;
        state_deleter(state_pool_t&);

        void operator()(state*) const;

    private:
        state_pool_t* _pool = nullptr;
    };

private:
    template <std::derived_from<state> State, typename... Args>
    [[nodiscard]] auto create_state(Args&&... args) -> state_ptr
    {
        return state_ptr(&_state_pool.create<State>(std::forward<Args>(args)...), state_deleter(_state_pool));
    }

    template <std::derived_from<state> State, typename... Args>
    void reserve_next_state(Args&&... args)
    {
        _next_state = create_state<State>(std::forward<Args>(args)...);
    }

    void apply_reserved_state_change();

private:
    const bn::sprite_text_generator& _text_generator;
    const int _max_chunk_width;
    const bn::vector<const bn::sprite_palette_item*, PALETTES_MAX_SIZE> _palettes;
    bn::vector<delegate_type, DELEGATES_MAX_SIZE> _delegates;
    bn::keypad::key_type _resume_key;
    bn::keypad::key_type _skip_key;

    state_pool_t _state_pool;
    state_ptr _state;
    state_ptr _next_state;

    bn::fixed_point _init_position;
    bn::string_view _text;
    bn::ivector<bn::sprite_ptr>* _output_sprites;
    int _init_sprite_index = 0;
    int _wait_updates;
    const bn::sound_item* _write_sound;
    int _max_line_width;
    bn::fixed _line_spacing;
    int _max_lines;
    bn::sprite_text_generator::alignment_type _alignment;

    bool _failed = false;

    bool _prev_whitespace = true;
    bool _paused_manual = false;
    bn::fixed _current_line_y = 0;
    int _palette_index = 0;
    bn::optional<bn::sound_handle> _write_sound_handle;
    int _current_line_width = 0;
    int _current_chunk_width = 0;
    int _current_line = 0;
    int _text_char_index = 0;
    bn::string<65 * 4> _text_chunk; // (max sprite width + 1) * (max utf-8 octet)
    int _line_first_sprite_index = 0;
    int _sprite_index = 0;
};

} // namespace ibn
