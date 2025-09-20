#include "ibn_sprite_text_typewriter.h"

#include <bn_assert.h>
#include <bn_keypad.h>
#include <bn_sound_item.h>
#include <bn_utf8_character.h>

#include <algorithm>

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

constexpr bool is_whitespace(bn::utf8_character ch)
{
    return ch.data() == CH_SPACE.data() || ch.data() == CH_TAB.data() || ch.data() == CH_NEWLINE.data();
}

constexpr bool is_control_char(bn::utf8_character ch)
{
    return ch.data() == CH_PAUSE_MANUAL.data() ||
           (CH_PAUSE_1.data() <= ch.data() && ch.data() <= CH_PAUSE_1.data() + 9) || ch.data() == CH_PAL_A_0.data() ||
           (CH_PAL_A_1.data() <= ch.data() && ch.data() <= CH_PAL_A_1.data() + 9) || ch.data() == CH_PAL_B_0.data() ||
           (CH_PAL_B_1.data() <= ch.data() && ch.data() <= CH_PAL_B_1.data() + 9);
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

} // namespace

sprite_text_typewriter::sprite_text_typewriter(const bn::sprite_text_generator& text_generator,
                                               bn::keypad::key_type resume_key, bn::keypad::key_type skip_key,
                                               const bn::span<const bn::sprite_palette_item* const>& palettes)
    : _text_generator(text_generator),
      _max_chunk_width(text_generator.font().item().shape_size().height() >= 32 ? 64 : 32),
      _palettes(init_palettes(text_generator, palettes)), _resume_key(resume_key), _skip_key(skip_key)
{
    BN_ASSERT(!text_generator.one_sprite_per_character(), "DO NOT set `one_sprite_per_character`!");
    BN_ASSERT((static_cast<std::uint16_t>(resume_key) & static_cast<std::uint16_t>(skip_key)) == 0,
              "Resume & skip keys shouldn't overlap");
}

void sprite_text_typewriter::update()
{
    BN_ASSERT(!done(), "Typewritting is done");

    if (!_skip_requested && bn::keypad::pressed(_skip_key))
    {
        _skip_requested = true;
        _might_half_baked = true;
    }

    if (_paused_manual)
    {
        if (_skip_requested || bn::keypad::pressed(_resume_key))
            _paused_manual = false;
        else
            return;
    }

    if (!_skip_requested && ++_current_updates != _wait_updates)
        return;
    _current_updates = 0;

    if (!_skip_requested && _paused_remaining_delay != 0)
    {
        if (--_paused_remaining_delay != 0)
            return;
    }
    _paused_remaining_delay = 0;

    bool non_whitespace_rendered = false;
    while (!done())
    {
        const bn::utf8_character ch(_text[_text_char_index]);
        bool break_loop = false;

        if (ch.data() == CH_NEWLINE.data())
        {
            move_to_newline();
        }
        else if (ch.data() == CH_PAUSE_MANUAL.data())
        {
            if (!_skip_requested)
            {
                _paused_manual = true;
                break_loop = true;
            }
        }
        else if (CH_PAUSE_1.data() <= ch.data() && ch.data() <= CH_PAUSE_1.data() + 9)
        {
            if (!_skip_requested)
            {
                _paused_remaining_delay = ch.data() - CH_PAUSE_1.data() + 1;
                break_loop = true;
            }
        }
        else if (ch.data() == CH_PAL_A_0.data() ||
                 (CH_PAL_A_1.data() <= ch.data() && ch.data() <= CH_PAL_A_1.data() + 9))
        {
            const int pal_idx = (ch.data() == CH_PAL_A_0.data()) ? 0 : ch.data() - CH_PAL_A_1.data() + 1;

            change_palette_index(pal_idx);
        }
        else if (ch.data() == CH_PAL_B_0.data() ||
                 (CH_PAL_B_1.data() <= ch.data() && ch.data() <= CH_PAL_B_1.data() + 9))
        {
            const int pal_idx = (ch.data() == CH_PAL_B_0.data()) ? 0 : ch.data() - CH_PAL_B_1.data() + 1;

            change_palette_index(pal_idx);
        }
        else // Rendered text character
        {
            // Check for word-wrap
            const bool word_wrap = check_word_wrap();

            // Add new char to chunk
            _text_chunk.append(_text.substr(_text_char_index, ch.size()));
            int new_chunk_width = _text_generator.width(_text_chunk);

            // Check if adding new char would result in width overflow
            const bool chunk_overflow = new_chunk_width > _max_chunk_width;
            const bool line_overflow = _current_line_width - _current_chunk_width + new_chunk_width > _max_line_width;
            if (chunk_overflow || word_wrap || line_overflow)
            {
                // Remove incorrectly added char
                _text_chunk.shrink(_text_chunk.size() - ch.size());

                // New line
                if (word_wrap || line_overflow)
                    move_to_newline();
                // New temporary chunk string
                else
                    flag_new_sprite_required();

                // Re-add new char to new chunk
                _text_chunk.append(_text.substr(_text_char_index, ch.size()));
                new_chunk_width = _text_generator.width(_text_chunk);
            }

            const int prev_line_width = _current_line_width;

            // Remove the current chunk if not new
            if (!_skip_requested && !new_sprite_required())
            {
                _output_sprites->pop_back();
            }

            _current_line_width -= _current_chunk_width;

            // Adjust the positions of existing sprites in line
            using alignment_type = bn::sprite_text_generator::alignment_type;
            if (_alignment != alignment_type::LEFT)
            {
                bn::fixed diff = prev_line_width - (_current_line_width + new_chunk_width);
                if (_alignment == alignment_type::CENTER)
                    diff /= 2;

                for (int idx = _line_first_sprite_index; idx < _sprite_index; ++idx)
                {
                    auto& spr = (*_output_sprites)[idx];
                    spr.set_x(spr.x() + diff);
                }
            }

            // Render the new chunk
            if (!_skip_requested)
                render_chunk(_current_line_width, new_chunk_width);
            else
                _half_baked = _might_half_baked;

            _current_line_width += new_chunk_width;
            _current_chunk_width = new_chunk_width;

            _prev_whitespace = is_whitespace(ch);
            if (!_prev_whitespace)
                non_whitespace_rendered = true;

            break_loop = true;
        }

        _text_char_index += ch.size();
        if (!_skip_requested && break_loop)
            break;
    }

    // Final flushing when skipping
    if (done() && _skip_requested)
    {
        flag_new_sprite_required();
    }

    if (non_whitespace_rendered)
    {
        if (_write_sound != nullptr)
        {
            stop_write_sound();
            _write_sound_handle = _write_sound->play();
        }
    }
}

bool sprite_text_typewriter::paused() const
{
    return _paused_manual;
}

bool sprite_text_typewriter::done() const
{
    return _text_char_index == _text.size();
}

bool sprite_text_typewriter::skipped() const
{
    return _skip_requested;
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
                                   const bn::sound_item* write_sound, int line_width, bn::fixed line_spacing)
{
    BN_ASSERT(wait_updates > 0, "Invalid wait updates: ", wait_updates);

    _init_position = top_left_position;
    _text = text;
    _output_sprites = &output_sprites;
    _init_sprite_index = output_sprites.size();
    _wait_updates = wait_updates;
    _write_sound = write_sound;
    _max_line_width = line_width;
    _line_spacing = line_spacing;
    _alignment = _text_generator.alignment();

    _prev_whitespace = true;
    _skip_requested = false;
    _half_baked = false;
    _might_half_baked = false;
    _paused_manual = false;
    _paused_remaining_delay = 0;
    _current_line_y = _init_position.y();
    _palette_index = 0;
    _current_updates = 0;
    stop_write_sound();
    _current_line_width = 0;
    _current_chunk_width = 0;
    _text_char_index = 0;
    _text_chunk.clear();
    _line_first_sprite_index = _init_sprite_index;
    _sprite_index = _init_sprite_index;
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

void sprite_text_typewriter::flag_new_sprite_required()
{
    if (_half_baked)
    {
        if (!new_sprite_required())
            _output_sprites->pop_back();
        render_chunk(_current_line_width - _current_chunk_width, _current_chunk_width);
    }
    else if (!_might_half_baked && _skip_requested)
        render_chunk(_current_line_width - _current_chunk_width, _current_chunk_width);

    _sprite_index = _output_sprites->size();

    _text_chunk.clear();
    _current_chunk_width = 0;

    _half_baked = false;
    _might_half_baked = false;
}

void sprite_text_typewriter::move_to_newline()
{
    flag_new_sprite_required();

    _current_line_y += _line_spacing;

    _current_line_width = 0;

    _line_first_sprite_index = _sprite_index;
}

void sprite_text_typewriter::change_palette_index(int palette_index)
{
    if (palette_index == _palette_index)
        return;

    flag_new_sprite_required();
    _palette_index = palette_index;
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

} // namespace ibn
