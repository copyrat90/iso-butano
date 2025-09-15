/*
 * Copyright (c) 2020-2025 Gustavo Valiente gustavo.valiente@protonmail.com
 * zlib License, see `licenses/butano.txt` file.
 *
 * 2023-03-24: Modified by copyrat90
 *   + `ibn::sprite_palette_swap_toggle_action` is an ALTERED version of `bn::sprite_palette_inverted_toggle_action`,
 *     which changes it to toggle between two sprite palettes.
 */

#ifndef IBN_SPRITE_PALETTE_SWAP_ACTION_H
#define IBN_SPRITE_PALETTE_SWAP_ACTION_H

#include <bn_sprite_palette_item.h>
#include <bn_sprite_palette_ptr.h>
#include <bn_value_template_actions.h>

namespace ibn
{

// swap

/**
 * @brief Manages if the colors of a sprite_palette_ptr must be swapped or not.
 *
 * @ingroup sprite
 * @ingroup palette
 * @ingroup action
 */
class sprite_palette_swap_manager
{
    friend class sprite_palette_swap_toggle_action;

private:
    struct _managed_palette_swap
    {
        bn::sprite_palette_ptr ptr;
        bool swapped;
        bn::sprite_palette_item original_item;
        bn::sprite_palette_item swapped_item;
    };

public:
    /**
     * @brief Indicates if the colors of the given sprite_palette_ptr are swapped or not.
     */
    [[nodiscard]] static bool get(const _managed_palette_swap& palette_swap)
    {
        return palette_swap.swapped;
    }

    /**
     * @brief Sets if the colors of the given sprite_palette_ptr must be swapped or not.
     */
    static void set(bool swapped, _managed_palette_swap& palette_swap)
    {
        palette_swap.ptr.set_colors(swapped ? palette_swap.swapped_item : palette_swap.original_item);
        palette_swap.swapped = swapped;
    }
};

/**
 * @brief Toggles if the colors of a sprite_palette_ptr must be swapped or not
 * when the action is updated a given number of times.
 *
 * @ingroup sprite
 * @ingroup palette
 * @ingroup action
 */
class sprite_palette_swap_toggle_action
    : public bn::bool_toggle_value_template_action<sprite_palette_swap_manager::_managed_palette_swap,
                                                   sprite_palette_swap_manager>
{

public:
    /**
     * @brief Constructor.
     * @param palette sprite_palette_ptr to copy.
     * @param original_palette_item original palette item.
     * @param swapped_palette_item swapped palette item.
     * @param duration_updates How much times the action has to be updated to toggle
     * if the colors of the given sprite_palette_ptr must be inverted or not.
     */
    sprite_palette_swap_toggle_action(const bn::sprite_palette_ptr& palette,
                                      const bn::sprite_palette_item& original_palette_item,
                                      const bn::sprite_palette_item& swapped_palette_item, int duration_updates)
        : bool_toggle_value_template_action(sprite_palette_swap_manager::_managed_palette_swap{palette, false,
                                                                                               original_palette_item,
                                                                                               swapped_palette_item},
                                            duration_updates)
    {
    }

    /**
     * @brief Constructor.
     * @param palette sprite_palette_ptr to move.
     * @param original_palette_item original palette item.
     * @param swapped_palette_item swapped palette item.
     * @param duration_updates How much times the action has to be updated to toggle
     * if the colors of the given sprite_palette_ptr must be inverted or not.
     */
    sprite_palette_swap_toggle_action(bn::sprite_palette_ptr&& palette,
                                      const bn::sprite_palette_item& original_palette_item,
                                      const bn::sprite_palette_item& swapped_palette_item, int duration_updates)
        : bool_toggle_value_template_action(sprite_palette_swap_manager::_managed_palette_swap{bn::move(palette), false,
                                                                                               original_palette_item,
                                                                                               swapped_palette_item},
                                            duration_updates)
    {
    }

    /**
     * @brief Returns the sprite_palette_ptr to modify.
     */
    [[nodiscard]] const bn::sprite_palette_ptr& palette() const
    {
        return value().ptr;
    }
};

} // namespace ibn

#endif
