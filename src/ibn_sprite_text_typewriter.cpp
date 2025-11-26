// SPDX-FileCopyrightText: Copyright 2021-2025 Guyeon Yu <copyrat90@gmail.com>
// SPDX-License-Identifier: Zlib

#include "ibn_sprite_text_typewriter.h"

#include <bn_assert.h>
#include <bn_keypad.h>
#include <bn_sound_item.h>
#include <bn_utf8_character.h>

#include <new>

namespace ibn
{

namespace
{

constexpr bn::utf8_character CH_SPACE(" ");
constexpr bn::utf8_character CH_NEWLINE("\n");
constexpr bn::utf8_character CH_TAB("\t");
constexpr bn::utf8_character CH_PAUSE_MANUAL("⏯");
constexpr bn::utf8_character CH_PAUSE_1("⓵");
constexpr bn::utf8_character CH_PAL_A_0("⓿");
constexpr bn::utf8_character CH_PAL_A_1("❶");
constexpr bn::utf8_character CH_PAL_B_0("🄌");
constexpr bn::utf8_character CH_PAL_B_1("➊");
constexpr bn::utf8_character CH_DELE_A_0("⓪");
constexpr bn::utf8_character CH_DELE_A_1("①");
constexpr bn::utf8_character CH_DELE_B_0("🄋");
constexpr bn::utf8_character CH_DELE_B_1("➀");

constexpr bool is_whitespace(bn::utf8_character ch)
{
    return ch.data() == CH_SPACE.data() || ch.data() == CH_TAB.data() || ch.data() == CH_NEWLINE.data();
}

constexpr bool is_control_char(bn::utf8_character ch)
{
    return ch.data() == CH_PAUSE_MANUAL.data() ||
           (CH_PAUSE_1.data() <= ch.data() && ch.data() <= CH_PAUSE_1.data() + 9) || ch.data() == CH_PAL_A_0.data() ||
           (CH_PAL_A_1.data() <= ch.data() &&
            ch.data() <= CH_PAL_A_1.data() + sprite_text_typewriter::PALETTES_MAX_SIZE - 2) ||
           ch.data() == CH_PAL_B_0.data() || (CH_PAL_B_1.data() <= ch.data() && ch.data() <= CH_PAL_B_1.data() + 9) ||
           ch.data() == CH_DELE_A_0.data() ||
           (CH_DELE_A_1.data() <= ch.data() &&
            ch.data() <= CH_DELE_A_1.data() + sprite_text_typewriter::DELEGATES_MAX_SIZE - 2) ||
           ch.data() == CH_DELE_B_0.data() || (CH_DELE_B_1.data() <= ch.data() && ch.data() <= CH_DELE_B_1.data() + 9);
}

auto init_palettes(const bn::sprite_text_generator& text_generator,
                   const bn::span<const bn::sprite_palette_item* const>& palettes)
{
    BN_ASSERT(palettes.size() <= sprite_text_typewriter::PALETTES_MAX_SIZE, "Too many palettes: ", palettes.size());

    bn::vector<const bn::sprite_palette_item*, sprite_text_typewriter::PALETTES_MAX_SIZE> result;
    for (const auto* pal : palettes)
        result.push_back(pal);

    // If no palette specified, use the default palette of the `text_generator`
    if (result.empty())
        result.push_back(&text_generator.font().item().palette_item());

    return result;
}

auto init_delegates(const bn::span<const sprite_text_typewriter::delegate_type>& delegates)
{
    BN_ASSERT(delegates.size() <= sprite_text_typewriter::DELEGATES_MAX_SIZE, "Too many delegates: ", delegates.size());

    bn::vector<sprite_text_typewriter::delegate_type, sprite_text_typewriter::DELEGATES_MAX_SIZE> result;
    for (const auto& dele : delegates)
        result.push_back(dele);

    return result;
}

} // namespace

sprite_text_typewriter::sprite_text_typewriter(const bn::sprite_text_generator& text_generator,
                                               bn::keypad::key_type resume_key, bn::keypad::key_type skip_key,
                                               const bn::span<const bn::sprite_palette_item* const>& palettes,
                                               const bn::span<const delegate_type>& delegates)
    : _text_generator(text_generator),
      _max_chunk_width(text_generator.font().item().shape_size().height() >= 32 ? 64 : 32),
      _palettes(init_palettes(text_generator, palettes)), _delegates(init_delegates(delegates)),
      _resume_key(resume_key), _skip_key(skip_key), _state(create_state<done_state>()),
      _next_state(nullptr, state_deleter(_state_pool))
{
    BN_ASSERT(!text_generator.one_sprite_per_character(), "DO NOT set `one_sprite_per_character`!");
    BN_ASSERT((static_cast<std::uint16_t>(resume_key) & static_cast<std::uint16_t>(skip_key)) == 0,
              "Resume & skip keys shouldn't overlap");

    _state->enter(*this);
}

void sprite_text_typewriter::update()
{
    BN_ASSERT(!done(), "Typewritting is done");

    // User requested state change
    apply_reserved_state_change();

    _state->update(*this);

    // State requested state change
    apply_reserved_state_change();
}

bool sprite_text_typewriter::paused() const
{
    return _paused_manual;
}

void sprite_text_typewriter::pause()
{
    if (!done())
    {
        _paused_manual = true;
        reserve_next_state<manual_pause_state>();
    }
}

void sprite_text_typewriter::resume()
{
    BN_ASSERT(paused(), "typewriter is not paused");

    _paused_manual = false;
    reserve_next_state<type_state>();
}

bool sprite_text_typewriter::done() const
{
    return _text_char_index == _text.size();
}

void sprite_text_typewriter::skip()
{
    if (!done())
        reserve_next_state<skip_state>();
}

auto sprite_text_typewriter::next_character_position() const -> bn::fixed_point
{
    using alignment_type = bn::sprite_text_generator::alignment_type;

    const bn::fixed diff = _alignment == alignment_type::LEFT     ? _current_line_width
                           : _alignment == alignment_type::CENTER ? bn::fixed(_current_line_width) / 2
                                                                  : 0;

    return bn::fixed_point{
        _init_position.x() + diff,
        _current_line_y,
    };
}

void sprite_text_typewriter::start(const bn::fixed_point& top_left_position, const bn::string_view& text,
                                   bn::ivector<bn::sprite_ptr>& output_sprites, int wait_updates,
                                   const bn::sound_item* write_sound, int line_width, bn::fixed line_spacing,
                                   int max_lines)
{
    BN_ASSERT(wait_updates > 0, "Invalid wait updates: ", wait_updates);
    BN_ASSERT(max_lines > 0, "Invalid max lines: ", max_lines);

    _init_position = top_left_position;
    _text = text;
    _output_sprites = &output_sprites;
    _init_sprite_index = output_sprites.size();
    _wait_updates = wait_updates;
    _write_sound = write_sound;
    _max_line_width = line_width;
    _line_spacing = line_spacing;
    _max_lines = max_lines;
    _alignment = _text_generator.alignment();

    _prev_whitespace = true;
    _paused_manual = false;
    _current_line_y = _init_position.y();
    _palette_index = 0;
    stop_write_sound();
    _current_line_width = 0;
    _current_chunk_width = 0;
    _current_line = 0;
    _text_char_index = 0;
    _text_chunk.clear();
    _line_first_sprite_index = _init_sprite_index;
    _sprite_index = _init_sprite_index;

    reserve_next_state<type_state>();
}

auto sprite_text_typewriter::resume_key() const -> bn::keypad::key_type
{
    return _resume_key;
}

void sprite_text_typewriter::set_resume_key(bn::keypad::key_type resume_key)
{
    _resume_key = resume_key;
}

auto sprite_text_typewriter::skip_key() const -> bn::keypad::key_type
{
    return _skip_key;
}

void sprite_text_typewriter::set_skip_key(bn::keypad::key_type skip_key)
{
    _skip_key = skip_key;
}

void sprite_text_typewriter::stop_write_sound()
{
    if (_write_sound_handle.has_value())
    {
        if (_write_sound_handle->active())
            _write_sound_handle->stop();

        _write_sound_handle.reset();
    }
}

bool sprite_text_typewriter::new_sprite_required() const
{
    // If `_text_chunk` only contains whitespaces, it won't create an additional sprite.
    // In that case, this remains `true`.
    // (i.e. We still require a new sprite)
    return _sprite_index == _output_sprites->size();
}

bool sprite_text_typewriter::max_lines_overflow() const
{
    return _current_line >= _max_lines;
}

void sprite_text_typewriter::wipe_and_reset_pos()
{
    while (_output_sprites->size() > _init_sprite_index)
        _output_sprites->pop_back();

    _current_line_y = _init_position.y();
    _current_line_width = 0;
    _line_first_sprite_index = _init_sprite_index;

    _sprite_index = _init_sprite_index;

    _current_line = 0;
}

bool sprite_text_typewriter::check_word_wrap() const
{
    bn::utf8_character ch(_text[_text_char_index]);

    // Only check on the word start to avoid duplicated checks
    if (!_prev_whitespace || is_whitespace(ch))
        return false;

    // Calculate the width of the word
    int word_width = 0;
    for (int idx = _text_char_index; idx < _text.size(); idx += ch.size())
    {
        ch = bn::utf8_character(_text[idx]);

        // Break on whitespace found
        if (is_whitespace(ch))
            break;

        // Ignore control characters
        if (is_control_char(ch))
            continue;

        bn::string_view ch_str(_text.substr(idx, ch.size()));
        word_width += _text_generator.width(ch_str);
    }

    // Determine the word wrap
    return word_width <= _max_line_width && _current_line_width + word_width > _max_line_width;
}

void sprite_text_typewriter::render_chunk(int current_line_width, int new_chunk_width)
{
    auto& gen = const_cast<bn::sprite_text_generator&>(_text_generator);
    const auto prev_align = gen.alignment();
    const auto prev_palette = gen.palette_item();
    gen.set_palette_item(*_palettes[_palette_index]);
    switch (_alignment)
    {
        using alignment_type = bn::sprite_text_generator::alignment_type;
    case alignment_type::LEFT: {
        gen.set_alignment(alignment_type::LEFT);
        gen.generate_top_left(_init_position.x() + current_line_width, _current_line_y, _text_chunk, *_output_sprites);
        break;
    }
    case alignment_type::CENTER: {
        gen.set_alignment(alignment_type::LEFT);
        const bn::fixed next_line_width = current_line_width + new_chunk_width;
        gen.generate_top_left(_init_position.x() + next_line_width / 2 - new_chunk_width, _current_line_y, _text_chunk,
                              *_output_sprites);
        break;
    }
    case alignment_type::RIGHT: {
        gen.set_alignment(alignment_type::LEFT);
        gen.generate_top_left(_init_position.x() - new_chunk_width, _current_line_y, _text_chunk, *_output_sprites);
        break;
    }
    default:
        BN_ERROR("Invalid text alignment: ", (int)_alignment);
    }
    gen.set_alignment(prev_align);
    gen.set_palette_item(prev_palette);
}

void sprite_text_typewriter::call_custom_delegate(int delegate_index)
{
    _delegates[delegate_index](delegate_index);
}

bool sprite_text_typewriter::state::skip_if_key_pressed(sprite_text_typewriter& writer)
{
    if (bn::keypad::pressed(writer._skip_key))
    {
        writer.skip();
        return true;
    }

    return false;
}

void sprite_text_typewriter::type_state::update(sprite_text_typewriter& writer)
{
    if (skip_if_key_pressed(writer))
        return;

    if (++_current_updates != writer._wait_updates)
        return;
    _current_updates = 0;

    if (_timed_pause_remaining != 0)
    {
        if (--_timed_pause_remaining != 0)
            return;
    }

    bool non_whitespace_rendered = false;
    while (!writer.done())
    {
        const bn::utf8_character ch(writer._text[writer._text_char_index]);
        bool break_loop = false;

        if (ch.data() == CH_NEWLINE.data())
        {
            move_to_newline(writer);
            if (writer.max_lines_overflow())
            {
                writer.pause();
                break_loop = true;
            }
        }
        else if (ch.data() == CH_PAUSE_MANUAL.data())
        {
            writer.pause();
            break_loop = true;
        }
        else if (CH_PAUSE_1.data() <= ch.data() && ch.data() <= CH_PAUSE_1.data() + 9)
        {
            _timed_pause_remaining = ch.data() - CH_PAUSE_1.data() + 1;
            break_loop = true;
        }
        else if (ch.data() == CH_PAL_A_0.data() ||
                 (CH_PAL_A_1.data() <= ch.data() &&
                  ch.data() <= CH_PAL_A_1.data() + sprite_text_typewriter::PALETTES_MAX_SIZE - 2))
        {
            const int pal_idx = (ch.data() == CH_PAL_A_0.data()) ? 0 : ch.data() - CH_PAL_A_1.data() + 1;
            change_palette_index(pal_idx, writer);
        }
        else if (ch.data() == CH_PAL_B_0.data() ||
                 (CH_PAL_B_1.data() <= ch.data() && ch.data() <= CH_PAL_B_1.data() + 9))
        {
            const int pal_idx = (ch.data() == CH_PAL_B_0.data()) ? 0 : ch.data() - CH_PAL_B_1.data() + 1;
            change_palette_index(pal_idx, writer);
        }
        else if (ch.data() == CH_DELE_A_0.data() ||
                 (CH_DELE_A_1.data() <= ch.data() &&
                  ch.data() <= CH_DELE_A_1.data() + sprite_text_typewriter::DELEGATES_MAX_SIZE - 2))
        {
            const int dele_idx = (ch.data() == CH_DELE_A_0.data()) ? 0 : ch.data() - CH_DELE_A_1.data() + 1;
            writer.call_custom_delegate(dele_idx);
        }
        else if (ch.data() == CH_DELE_B_0.data() ||
                 (CH_DELE_B_1.data() <= ch.data() && ch.data() <= CH_DELE_B_1.data() + 9))
        {
            const int dele_idx = (ch.data() == CH_DELE_B_0.data()) ? 0 : ch.data() - CH_DELE_B_1.data() + 1;
            writer.call_custom_delegate(dele_idx);
        }
        else // Rendered text character
        {
            // Check for word-wrap
            const bool word_wrap = writer.check_word_wrap();

            // Add new char to chunk
            writer._text_chunk.append(writer._text.substr(writer._text_char_index, ch.size()));
            int new_chunk_width = writer._text_generator.width(writer._text_chunk);

            // Check if adding new char would result in width overflow
            const bool chunk_overflow = new_chunk_width > writer._max_chunk_width;
            const bool line_overflow =
                writer._current_line_width - writer._current_chunk_width + new_chunk_width > writer._max_line_width;
            if (chunk_overflow || word_wrap || line_overflow)
            {
                // Remove incorrectly added char
                writer._text_chunk.shrink(writer._text_chunk.size() - ch.size());

                // New line
                if (word_wrap || line_overflow)
                {
                    move_to_newline(writer);
                    if (writer.max_lines_overflow())
                    {
                        writer.pause();
                        // This character should be re-parsed after pausing, so force-break
                        break;
                    }
                }
                // New temporary chunk string
                else
                    flag_new_sprite_required(writer);

                // Re-add new char to new chunk
                writer._text_chunk.append(writer._text.substr(writer._text_char_index, ch.size()));
                new_chunk_width = writer._text_generator.width(writer._text_chunk);
            }

            const int prev_line_width = writer._current_line_width;

            // Remove the current chunk if not new
            if (!writer.new_sprite_required())
            {
                writer._output_sprites->pop_back();
            }

            writer._current_line_width -= writer._current_chunk_width;

            // Adjust the positions of existing sprites in line
            using alignment_type = bn::sprite_text_generator::alignment_type;
            if (writer._alignment != alignment_type::LEFT)
            {
                bn::fixed diff = prev_line_width - (writer._current_line_width + new_chunk_width);
                if (writer._alignment == alignment_type::CENTER)
                    diff /= 2;

                for (int idx = writer._line_first_sprite_index; idx < writer._sprite_index; ++idx)
                {
                    auto& spr = (*writer._output_sprites)[idx];
                    spr.set_x(spr.x() + diff);
                }
            }

            // Render the new chunk
            writer.render_chunk(writer._current_line_width, new_chunk_width);

            writer._current_line_width += new_chunk_width;
            writer._current_chunk_width = new_chunk_width;

            writer._prev_whitespace = is_whitespace(ch);
            if (!writer._prev_whitespace)
                non_whitespace_rendered = true;

            break_loop = true;
        }

        writer._text_char_index += ch.size();
        if (break_loop)
            break;
    }

    if (non_whitespace_rendered)
    {
        if (writer._write_sound != nullptr)
        {
            writer.stop_write_sound();
            writer._write_sound_handle = writer._write_sound->play();
        }
    }

    if (writer.done())
        writer.reserve_next_state<done_state>();
}

void sprite_text_typewriter::type_state::flag_new_sprite_required(sprite_text_typewriter& writer)
{
    writer._sprite_index = writer._output_sprites->size();

    writer._text_chunk.clear();
    writer._current_chunk_width = 0;
}

void sprite_text_typewriter::type_state::move_to_newline(sprite_text_typewriter& writer)
{
    flag_new_sprite_required(writer);

    ++writer._current_line;
    // When lines overflow, don't adjust positions
    if (writer.max_lines_overflow())
        return;

    writer._current_line_y += writer._line_spacing;

    writer._current_line_width = 0;

    writer._line_first_sprite_index = writer._sprite_index;
}

void sprite_text_typewriter::type_state::change_palette_index(int palette_index, sprite_text_typewriter& writer)
{
    if (palette_index == writer._palette_index)
        return;

    flag_new_sprite_required(writer);
    writer._palette_index = palette_index;
}

void sprite_text_typewriter::manual_pause_state::enter(sprite_text_typewriter& writer)
{
    writer._paused_manual = true;
}

void sprite_text_typewriter::manual_pause_state::exit(sprite_text_typewriter& writer)
{
    writer._paused_manual = false;

    if (writer.max_lines_overflow())
        writer.wipe_and_reset_pos();
}

void sprite_text_typewriter::manual_pause_state::update(sprite_text_typewriter& writer)
{
    if (skip_if_key_pressed(writer))
        return;

    resume_if_key_pressed(writer);
}

bool sprite_text_typewriter::manual_pause_state::resume_if_key_pressed(sprite_text_typewriter& writer)
{
    if (bn::keypad::pressed(writer._resume_key))
    {
        writer.resume();
        return true;
    }

    return false;
}

void sprite_text_typewriter::skip_state::exit(sprite_text_typewriter& writer)
{
    // Final flushing when skipping
    flag_new_sprite_required(writer);
}

void sprite_text_typewriter::skip_state::update(sprite_text_typewriter& writer)
{
    bool non_whitespace_rendered = false;
    while (!writer.done())
    {
        const bn::utf8_character ch(writer._text[writer._text_char_index]);
        bool break_loop = false;

        if (ch.data() == CH_NEWLINE.data())
        {
            move_to_newline(writer);
            if (writer.max_lines_overflow())
            {
                writer.pause();
                break_loop = true;
            }
        }
        else if (ch.data() == CH_PAUSE_MANUAL.data())
        {
            // Ignore manual pause command in skip
        }
        else if (CH_PAUSE_1.data() <= ch.data() && ch.data() <= CH_PAUSE_1.data() + 9)
        {
            // Ignore timed pause commands in skip
        }
        else if (ch.data() == CH_PAL_A_0.data() ||
                 (CH_PAL_A_1.data() <= ch.data() &&
                  ch.data() <= CH_PAL_A_1.data() + sprite_text_typewriter::PALETTES_MAX_SIZE - 2))
        {
            const int pal_idx = (ch.data() == CH_PAL_A_0.data()) ? 0 : ch.data() - CH_PAL_A_1.data() + 1;
            change_palette_index(pal_idx, writer);
        }
        else if (ch.data() == CH_PAL_B_0.data() ||
                 (CH_PAL_B_1.data() <= ch.data() && ch.data() <= CH_PAL_B_1.data() + 9))
        {
            const int pal_idx = (ch.data() == CH_PAL_B_0.data()) ? 0 : ch.data() - CH_PAL_B_1.data() + 1;
            change_palette_index(pal_idx, writer);
        }
        else if (ch.data() == CH_DELE_A_0.data() ||
                 (CH_DELE_A_1.data() <= ch.data() &&
                  ch.data() <= CH_DELE_A_1.data() + sprite_text_typewriter::DELEGATES_MAX_SIZE - 2))
        {
            const int dele_idx = (ch.data() == CH_DELE_A_0.data()) ? 0 : ch.data() - CH_DELE_A_1.data() + 1;
            writer.call_custom_delegate(dele_idx);
        }
        else if (ch.data() == CH_DELE_B_0.data() ||
                 (CH_DELE_B_1.data() <= ch.data() && ch.data() <= CH_DELE_B_1.data() + 9))
        {
            const int dele_idx = (ch.data() == CH_DELE_B_0.data()) ? 0 : ch.data() - CH_DELE_B_1.data() + 1;
            writer.call_custom_delegate(dele_idx);
        }
        else // Rendered text character
        {
            // Check for word-wrap
            const bool word_wrap = writer.check_word_wrap();

            // Add new char to chunk
            writer._text_chunk.append(writer._text.substr(writer._text_char_index, ch.size()));
            int new_chunk_width = writer._text_generator.width(writer._text_chunk);

            // Check if adding new char would result in width overflow
            const bool chunk_overflow = new_chunk_width > writer._max_chunk_width;
            const bool line_overflow =
                writer._current_line_width - writer._current_chunk_width + new_chunk_width > writer._max_line_width;
            if (chunk_overflow || word_wrap || line_overflow)
            {
                // Remove incorrectly added char
                writer._text_chunk.shrink(writer._text_chunk.size() - ch.size());

                // New line
                if (word_wrap || line_overflow)
                {
                    move_to_newline(writer);
                    if (writer.max_lines_overflow())
                    {
                        writer.pause();
                        // This character should be re-parsed after pausing, so force-break
                        break;
                    }
                }
                // New temporary chunk string
                else
                    flag_new_sprite_required(writer);

                // Re-add new char to new chunk
                writer._text_chunk.append(writer._text.substr(writer._text_char_index, ch.size()));
                new_chunk_width = writer._text_generator.width(writer._text_chunk);
            }

            const int prev_line_width = writer._current_line_width;

            writer._current_line_width -= writer._current_chunk_width;

            // Adjust the positions of existing sprites in line
            using alignment_type = bn::sprite_text_generator::alignment_type;
            if (writer._alignment != alignment_type::LEFT)
            {
                bn::fixed diff = prev_line_width - (writer._current_line_width + new_chunk_width);
                if (writer._alignment == alignment_type::CENTER)
                    diff /= 2;

                for (int idx = writer._line_first_sprite_index; idx < writer._sprite_index; ++idx)
                {
                    auto& spr = (*writer._output_sprites)[idx];
                    spr.set_x(spr.x() + diff);
                }
            }

            // Flag whether the last sprite's half baked
            _half_baked = _might_half_baked;

            writer._current_line_width += new_chunk_width;
            writer._current_chunk_width = new_chunk_width;

            writer._prev_whitespace = is_whitespace(ch);
            if (!writer._prev_whitespace)
                non_whitespace_rendered = true;
        }

        writer._text_char_index += ch.size();
        if (break_loop)
            break;
    }

    if (non_whitespace_rendered)
    {
        if (writer._write_sound != nullptr)
        {
            writer.stop_write_sound();
            writer._write_sound_handle = writer._write_sound->play();
        }
    }

    if (writer.done())
        writer.reserve_next_state<done_state>();
}

void sprite_text_typewriter::skip_state::flag_new_sprite_required(sprite_text_typewriter& writer)
{
    if (_half_baked)
    {
        if (!writer.new_sprite_required())
            writer._output_sprites->pop_back();
    }

    writer.render_chunk(writer._current_line_width - writer._current_chunk_width, writer._current_chunk_width);

    writer._sprite_index = writer._output_sprites->size();

    writer._text_chunk.clear();
    writer._current_chunk_width = 0;

    _half_baked = false;
    _might_half_baked = false;
}

void sprite_text_typewriter::skip_state::move_to_newline(sprite_text_typewriter& writer)
{
    flag_new_sprite_required(writer);

    ++writer._current_line;
    // When lines overflow, don't adjust positions
    if (writer.max_lines_overflow())
        return;

    writer._current_line_y += writer._line_spacing;

    writer._current_line_width = 0;

    writer._line_first_sprite_index = writer._sprite_index;
}

void sprite_text_typewriter::skip_state::change_palette_index(int palette_index, sprite_text_typewriter& writer)
{
    if (palette_index == writer._palette_index)
        return;

    flag_new_sprite_required(writer);
    writer._palette_index = palette_index;
}

void sprite_text_typewriter::done_state::update(sprite_text_typewriter&)
{
}

sprite_text_typewriter::state_deleter::state_deleter(state_pool_t& pool) : _pool(&pool)
{
}

void sprite_text_typewriter::state_deleter::operator()(state* state) const
{
    _pool->destroy(*state);
}

void sprite_text_typewriter::apply_reserved_state_change()
{
    if (_next_state)
    {
        _state->exit(*this);
        _state = std::move(_next_state);
        _state->enter(*this);
    }
}

} // namespace ibn
