/*
 * Copyright (c) 2020-2025 Gustavo Valiente gustavo.valiente@protonmail.com
 * zlib License, see `licenses/butano.txt` file.
 *
 * 2025-09-18: Modified by copyrat90
 *   + `ibn::sprite_text_generator` is an ALTERED version of `bn::sprite_text_generator`,
 *     which adds multi-line text generation support with the newline character '\n'.
 */

#include "ibn_sprite_text_generator.h"

#include <bn_sprite_ptr.h>

namespace ibn
{

void sprite_text_generator::generate(bn::fixed x, bn::fixed y, const bn::string_view& text,
                                     bn::ivector<bn::sprite_ptr>& output_sprites, bn::fixed line_spacing) const
{
    bn::string_view str(text);

    while (!str.empty())
    {
        int pos = str.find('\n');
        if (pos == bn::string_view::npos)
            pos = str.size();

        bn::sprite_text_generator::generate(x, y, str.substr(0, pos), output_sprites);

        y += line_spacing;
        str = str.substr(pos + 1);
    }
}

void sprite_text_generator::generate_top_left(bn::fixed top_left_x, bn::fixed top_left_y, const bn::string_view& text,
                                              bn::ivector<bn::sprite_ptr>& output_sprites, bn::fixed line_spacing) const
{
    bn::string_view str(text);

    while (!str.empty())
    {
        int pos = str.find('\n');
        if (pos == bn::string_view::npos)
            pos = str.size();

        bn::sprite_text_generator::generate_top_left(top_left_x, top_left_y, str.substr(0, pos), output_sprites);

        top_left_y += line_spacing;
        str = str.substr(pos + 1);
    }
}

bool sprite_text_generator::generate_optional(bn::fixed x, bn::fixed y, const bn::string_view& text,
                                              bn::ivector<bn::sprite_ptr>& output_sprites, bn::fixed line_spacing) const
{
    const int output_sprites_count = output_sprites.size();

    bn::string_view str(text);
    bool result = true;

    while (!str.empty())
    {
        int pos = str.find('\n');
        if (pos == bn::string_view::npos)
            pos = str.size();

        result = bn::sprite_text_generator::generate_optional(x, y, str.substr(0, pos), output_sprites);
        if (!result)
        {
            output_sprites.shrink(output_sprites_count);
            break;
        }

        y += line_spacing;
        str = str.substr(pos + 1);
    }

    return result;
}

bool sprite_text_generator::generate_top_left_optional(bn::fixed top_left_x, bn::fixed top_left_y,
                                                       const bn::string_view& text,
                                                       bn::ivector<bn::sprite_ptr>& output_sprites,
                                                       bn::fixed line_spacing) const
{
    const int output_sprites_count = output_sprites.size();

    bn::string_view str(text);
    bool result = true;

    while (!str.empty())
    {
        int pos = str.find('\n');
        if (pos == bn::string_view::npos)
            pos = str.size();

        result = bn::sprite_text_generator::generate_top_left_optional(top_left_x, top_left_y, str.substr(0, pos),
                                                                       output_sprites);
        if (!result)
        {
            output_sprites.shrink(output_sprites_count);
            break;
        }

        top_left_y += line_spacing;
        str = str.substr(pos + 1);
    }

    return result;
}

} // namespace ibn
