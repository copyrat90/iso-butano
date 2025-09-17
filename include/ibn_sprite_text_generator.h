/*
 * Copyright (c) 2020-2025 Gustavo Valiente gustavo.valiente@protonmail.com
 * zlib License, see `licenses/butano.txt` file.
 *
 * 2025-09-18: Modified by copyrat90
 *   + `ibn::sprite_text_generator` is an ALTERED version of `bn::sprite_text_generator`,
 *     which adds multi-line text generation support with the newline character '\n'.
 */

#pragma once

#include <bn_sprite_text_generator.h>

namespace ibn
{

/// @brief Extends `bn::sprite_text_generator` to add support for the
/// multi-line text generation with the newline character (`\n`).
class sprite_text_generator : public bn::sprite_text_generator
{
public:
    using bn::sprite_text_generator::sprite_text_generator;

    using bn::sprite_text_generator::generate;
    using bn::sprite_text_generator::generate_optional;
    using bn::sprite_text_generator::generate_top_left;
    using bn::sprite_text_generator::generate_top_left_optional;

public:
    /// @brief Generates text sprites for the given multiple lines of text.
    /// @tparam MaxSprites Maximum size of the returned sprite_ptr vector.
    /// @param text Multiple lines of text to print.
    /// @param line_spacing Space between lines in pixels.
    /// @return sprite_ptr vector containing the generated text sprites.
    template <int MaxSprites>
    [[nodiscard]] bn::vector<bn::sprite_ptr, MaxSprites> generate(const bn::string_view& text,
                                                                  bn::fixed line_spacing) const
    {
        bn::vector<bn::sprite_ptr, MaxSprites> output_sprites;
        generate(text, output_sprites, line_spacing);
        return output_sprites;
    }

    /// @brief Generates text sprites for the given multiple lines of text.
    /// @tparam MaxSprites Maximum size of the returned sprite_ptr vector.
    /// @param x Horizontal position of the first generated sprite, considering the current alignment.
    /// @param y Vertical position of the first generated sprite, considering the current alignment.
    /// @param text Multiple lines of text to print.
    /// @param line_spacing Space between lines in pixels.
    /// @return sprite_ptr vector containing the generated text sprites.
    template <int MaxSprites>
    [[nodiscard]] bn::vector<bn::sprite_ptr, MaxSprites> generate(bn::fixed x, bn::fixed y, const bn::string_view& text,
                                                                  bn::fixed line_spacing) const
    {
        bn::vector<bn::sprite_ptr, MaxSprites> output_sprites;
        generate(x, y, text, output_sprites, line_spacing);
        return output_sprites;
    }

    /// @brief Generates text sprites for the given multiple lines of text.
    /// @tparam MaxSprites Maximum size of the returned sprite_ptr vector.
    /// @param position Position of the first generated sprite, considering the current alignment.
    /// @param text Multiple lines of text to print.
    /// @param line_spacing Space between lines in pixels.
    /// @return sprite_ptr vector containing the generated text sprites.
    template <int MaxSprites>
    [[nodiscard]] bn::vector<bn::sprite_ptr, MaxSprites> generate(const bn::fixed_point& position,
                                                                  const bn::string_view& text,
                                                                  bn::fixed line_spacing) const
    {
        bn::vector<bn::sprite_ptr, MaxSprites> output_sprites;
        generate(position, text, output_sprites, line_spacing);
        return output_sprites;
    }

    /// @brief Generates text sprites for the given multiple lines of text.
    /// @param text Multiple lines of text to print.
    /// @param output_sprites Generated text sprites are stored in this vector.
    /// @param line_spacing Space between lines in pixels.
    ///
    /// Keep in mind that this vector is not cleared before generating text.
    void generate(const bn::string_view& text, bn::ivector<bn::sprite_ptr>& output_sprites,
                  bn::fixed line_spacing) const
    {
        return generate(0, 0, text, output_sprites, line_spacing);
    }

    /// @brief Generates text sprites for the given multiple lines of text.
    /// @param x Horizontal position of the first generated sprite, considering the current alignment.
    /// @param y Vertical position of the first generated sprite, considering the current alignment.
    /// @param text Multiple lines of text to print.
    /// @param output_sprites Generated text sprites are stored in this vector.
    /// @param line_spacing Space between lines in pixels.
    ///
    /// Keep in mind that this vector is not cleared before generating text.
    void generate(bn::fixed x, bn::fixed y, const bn::string_view& text, bn::ivector<bn::sprite_ptr>& output_sprites,
                  bn::fixed line_spacing) const;

    /// @brief Generates text sprites for the given multiple lines of text.
    /// @param position Position of the first generated sprite, considering the current alignment.
    /// @param text Multiple lines of text to print.
    /// @param output_sprites Generated text sprites are stored in this vector.
    /// @param line_spacing Space between lines in pixels.
    ///
    /// Keep in mind that this vector is not cleared before generating text.
    void generate(const bn::fixed_point& position, const bn::string_view& text,
                  bn::ivector<bn::sprite_ptr>& output_sprites, bn::fixed line_spacing) const
    {
        generate(position.x(), position.y(), text, output_sprites, line_spacing);
    }

    /// @brief Generates text sprites for the given multiple lines of text.
    /// @tparam MaxSprites Maximum size of the returned sprite_ptr vector.
    /// @param top_left_x Horizontal top-left position of the first generated sprite, considering the current alignment.
    /// @param top_left_y Vertical top-left position of the first generated sprite, considering the current alignment.
    /// @param text Multiple lines of text to print.
    /// @param line_spacing Space between lines in pixels.
    /// @return sprite_ptr vector containing the generated text sprites.
    template <int MaxSprites>
    [[nodiscard]] bn::vector<bn::sprite_ptr, MaxSprites> generate_top_left(bn::fixed top_left_x, bn::fixed top_left_y,
                                                                           const bn::string_view& text,
                                                                           bn::fixed line_spacing) const
    {
        bn::vector<bn::sprite_ptr, MaxSprites> output_sprites;
        generate_top_left(top_left_x, top_left_y, text, output_sprites, line_spacing);
        return output_sprites;
    }

    /// @brief Generates text sprites for the given multiple lines of text.
    /// @tparam MaxSprites Maximum size of the returned sprite_ptr vector.
    /// @param top_left_position Top-left position of the first generated sprite, considering the current alignment.
    /// @param text Multiple lines of text to print.
    /// @param line_spacing Space between lines in pixels.
    /// @return sprite_ptr vector containing the generated text sprites.
    template <int MaxSprites>
    [[nodiscard]] bn::vector<bn::sprite_ptr, MaxSprites> generate_top_left(const bn::fixed_point& top_left_position,
                                                                           const bn::string_view& text,
                                                                           bn::fixed line_spacing) const
    {
        bn::vector<bn::sprite_ptr, MaxSprites> output_sprites;
        generate_top_left(top_left_position, text, output_sprites, line_spacing);
        return output_sprites;
    }

    /// @brief Generates text sprites for the given multiple lines of text.
    /// @param top_left_x Horizontal top-left position of the first generated sprite, considering the current alignment.
    /// @param top_left_y Vertical top-left position of the first generated sprite, considering the current alignment.
    /// @param text Multiple lines of text to print.
    /// @param output_sprites Generated text sprites are stored in this vector.
    /// @param line_spacing Space between lines in pixels.
    ///
    /// Keep in mind that this vector is not cleared before generating text.
    void generate_top_left(bn::fixed top_left_x, bn::fixed top_left_y, const bn::string_view& text,
                           bn::ivector<bn::sprite_ptr>& output_sprites, bn::fixed line_spacing) const;

    /// @brief Generates text sprites for the given multiple lines of text.
    /// @param top_left_position Top-left position of the first generated sprite, considering the current alignment.
    /// @param text Multiple lines of text to print.
    /// @param output_sprites Generated text sprites are stored in this vector.
    /// @param line_spacing Space between lines in pixels.
    ///
    /// Keep in mind that this vector is not cleared before generating text.
    void generate_top_left(const bn::fixed_point& top_left_position, const bn::string_view& text,
                           bn::ivector<bn::sprite_ptr>& output_sprites, bn::fixed line_spacing) const
    {
        generate_top_left(top_left_position.x(), top_left_position.y(), text, output_sprites, line_spacing);
    }

    /// @brief Generates text sprites for the given multiple lines of text.
    /// @param text Multiple lines of text to print.
    /// @param output_sprites Generated text sprites are stored in this vector.
    /// @param line_spacing Space between lines in pixels.
    ///
    /// Keep in mind that this vector is not cleared before generating text.
    ///
    /// @return `true` if the text generation finished successfully, otherwise `false`.
    [[nodiscard]] bool generate_optional(const bn::string_view& text, bn::ivector<bn::sprite_ptr>& output_sprites,
                                         bn::fixed line_spacing) const
    {
        return generate_optional(0, 0, text, output_sprites, line_spacing);
    }

    /// @brief Generates text sprites for the given multiple lines of text.
    /// @param x Horizontal position of the first generated sprite, considering the current alignment.
    /// @param y Vertical position of the first generated sprite, considering the current alignment.
    /// @param text Multiple lines of text to print.
    /// @param output_sprites Generated text sprites are stored in this vector.
    /// @param line_spacing Space between lines in pixels.
    ///
    /// Keep in mind that this vector is not cleared before generating text.
    ///
    /// @return `true` if the text generation finished successfully, otherwise `false`.
    [[nodiscard]] bool generate_optional(bn::fixed x, bn::fixed y, const bn::string_view& text,
                                         bn::ivector<bn::sprite_ptr>& output_sprites, bn::fixed line_spacing) const;

    /// @brief Generates text sprites for the given multiple lines of text.
    /// @param position Position of the first generated sprite, considering the current alignment.
    /// @param text Multiple lines of text to print.
    /// @param output_sprites Generated text sprites are stored in this vector.
    /// @param line_spacing Space between lines in pixels.
    ///
    /// Keep in mind that this vector is not cleared before generating text.
    ///
    /// @return `true` if the text generation finished successfully, otherwise `false`.
    [[nodiscard]] bool generate_optional(const bn::fixed_point& position, const bn::string_view& text,
                                         bn::ivector<bn::sprite_ptr>& output_sprites, bn::fixed line_spacing) const
    {
        return generate_optional(position.x(), position.y(), text, output_sprites, line_spacing);
    }

    /// @brief Generates text sprites for the given multiple lines of text.
    /// @param top_left_x Horizontal top-left position of the first generated sprite, considering the current alignment.
    /// @param top_left_y Vertical top-left position of the first generated sprite, considering the current alignment.
    /// @param text Multiple lines of text to print.
    /// @param output_sprites Generated text sprites are stored in this vector.
    /// @param line_spacing Space between lines in pixels.
    ///
    /// Keep in mind that this vector is not cleared before generating text.
    [[nodiscard]] bool generate_top_left_optional(bn::fixed top_left_x, bn::fixed top_left_y,
                                                  const bn::string_view& text,
                                                  bn::ivector<bn::sprite_ptr>& output_sprites,
                                                  bn::fixed line_spacing) const;

    /// @brief Generates text sprites for the given multiple lines of text.
    /// @param top_left_position Top-left position of the first generated sprite, considering the current alignment.
    /// @param text Multiple lines of text to print.
    /// @param output_sprites Generated text sprites are stored in this vector.
    /// @param line_spacing Space between lines in pixels.
    ///
    /// Keep in mind that this vector is not cleared before generating text.
    ///
    /// @return `true` if the text generation finished successfully, otherwise `false`.
    [[nodiscard]] bool generate_top_left_optional(const bn::fixed_point& top_left_position, const bn::string_view& text,
                                                  bn::ivector<bn::sprite_ptr>& output_sprites,
                                                  bn::fixed line_spacing) const
    {
        return generate_top_left_optional(top_left_position.x(), top_left_position.y(), text, output_sprites,
                                          line_spacing);
    }
};

} // namespace ibn
