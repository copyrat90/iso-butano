/*
 * Copyright (c) 2020-2025 Gustavo Valiente gustavo.valiente@protonmail.com
 * zlib License, see `licenses/butano.txt` file.
 *
 * 2023-03-24: Modified by copyrat90
 *   + `ibn::variable_wait_sprite_animate_action` is an ALTERED version of `bn::sprite_animate_actions`,
 *     which adds variable `wait_updates`.
 */

#ifndef IBN_VARIABLE_WAIT_SPRITE_ANIMATE_ACTION_H
#define IBN_VARIABLE_WAIT_SPRITE_ANIMATE_ACTION_H

#include <bn_sprite_animate_actions.h>

namespace ibn
{

template <int MaxSize>
class variable_wait_sprite_animate_action
{
    static_assert(MaxSize > 1);

public:
    /**
     * @brief Generates a variable_wait_sprite_animate_action which loops over the given sprite tile sets only once.
     * @param sprite sprite_ptr to copy.
     * @param tiles_item It creates the new sprite tiles to use by the given sprite_ptr.
     * @param graphics_indexes Indexes of the tile sets to reference in tiles_item.
     * @param wait_updates Variable numbers of times the action must be updated before changing the tiles of the given
     * sprite_ptr.
     * @return The requested sprite_animate_action.
     */
    [[nodiscard]] static variable_wait_sprite_animate_action once(const bn::sprite_ptr& sprite,
                                                                  const bn::sprite_tiles_item& tiles_item,
                                                                  const bn::span<const uint16_t>& graphics_indexes,
                                                                  const bn::span<const uint16_t>& wait_updates)
    {
        return variable_wait_sprite_animate_action(sprite, tiles_item, false, graphics_indexes, wait_updates);
    }

    /**
     * @brief Generates a variable_wait_sprite_animate_action which loops over the given sprite tile sets only once.
     * @param sprite sprite_ptr to move.
     * @param tiles_item It creates the new sprite tiles to use by the given sprite_ptr.
     * @param graphics_indexes Indexes of the tile sets to reference in tiles_item.
     * @param wait_updates Variable numbers of times the action must be updated before changing the tiles of the given
     * sprite_ptr.
     * @return The requested sprite_animate_action.
     */
    [[nodiscard]] static variable_wait_sprite_animate_action once(bn::sprite_ptr&& sprite,
                                                                  const bn::sprite_tiles_item& tiles_item,
                                                                  const bn::span<const uint16_t>& graphics_indexes,
                                                                  const bn::span<const uint16_t>& wait_updates)
    {
        return variable_wait_sprite_animate_action(move(sprite), tiles_item, false, graphics_indexes, wait_updates);
    }

    /**
     * @brief Generates a variable_wait_sprite_animate_action which loops over the given sprite tile sets forever.
     * @param sprite sprite_ptr to copy.
     * @param tiles_item It creates the new sprite tiles to use by the given sprite_ptr.
     * @param graphics_indexes Indexes of the tile sets to reference in tiles_item.
     * @param wait_updates Variable numbers of times the action must be updated before changing the tiles of the given
     * sprite_ptr.
     * @return The requested sprite_animate_action.
     */
    [[nodiscard]] static variable_wait_sprite_animate_action forever(const bn::sprite_ptr& sprite,
                                                                     const bn::sprite_tiles_item& tiles_item,
                                                                     const bn::span<const uint16_t>& graphics_indexes,
                                                                     const bn::span<const uint16_t>& wait_updates)
    {
        return variable_wait_sprite_animate_action(sprite, tiles_item, true, graphics_indexes, wait_updates);
    }

    /**
     * @brief Generates a variable_wait_sprite_animate_action which loops over the given sprite tile sets forever.
     * @param sprite sprite_ptr to move.
     * @param tiles_item It creates the new sprite tiles to use by the given sprite_ptr.
     * @param graphics_indexes Indexes of the tile sets to reference in tiles_item.
     * @param wait_updates Variable numbers of times the action must be updated before changing the tiles of the given
     * sprite_ptr.
     * @return The requested sprite_animate_action.
     */
    [[nodiscard]] static variable_wait_sprite_animate_action forever(bn::sprite_ptr&& sprite,
                                                                     const bn::sprite_tiles_item& tiles_item,
                                                                     const bn::span<const uint16_t>& graphics_indexes,
                                                                     const bn::span<const uint16_t>& wait_updates)
    {
        return variable_wait_sprite_animate_action(move(sprite), tiles_item, true, graphics_indexes, wait_updates);
    }

    /**
     * @brief Changes the tile set of the given sprite_ptr when the given amount of update calls are done.
     */
    void update()
    {
        BN_ASSERT(!done(), "Action is done");

        if (_started && _current_wait_updates)
        {
            --_current_wait_updates;
        }
        else
        {
            _action.update();

            if (!_started)
            {
                _started = true;
                _current_wait_updates_index = 0;
            }
            else if (update_forever() && _current_wait_updates_index == _wait_updates.size() - 1)
            {
                _current_wait_updates_index = 0;
            }
            else
            {
                ++_current_wait_updates_index;
            }

            _current_wait_updates = _wait_updates[_current_wait_updates_index];
        }
    }

    /**
     * @brief Indicates if the action must not be updated anymore.
     * @return `true` if it reaches last graphics index AND updates few more times, specified by the last element of
     * wait_updates.
     */
    [[nodiscard]] bool done() const
    {
        return _action.done() && _current_wait_updates <= 0;
    }

    /**
     * @brief Returns the sprite_ptr to modify.
     */
    [[nodiscard]] const bn::sprite_ptr& sprite() const
    {
        return _action.sprite();
    }

    /**
     * @brief Returns the number of times the action must be updated before changing the tiles of the given sprite_ptr.
     */
    [[nodiscard]] int wait_updates() const
    {
        return _wait_updates[_current_wait_updates_index];
    }

    /**
     * @brief Returns the CURRENT number of times the action must be updated before changing the tiles of the given
     * sprite_ptr.
     */
    [[nodiscard]] int current_wait_updates() const
    {
        return _current_wait_updates;
    }

    /**
     * @brief Returns the sprite_tiles_item used to create the new sprite tiles to use by the given sprite_ptr.
     */
    [[nodiscard]] const bn::sprite_tiles_item& tiles_item() const
    {
        return _action.tiles_item();
    }

    /**
     * @brief Returns the indexes of the tile sets to reference in the given sprite_tiles_item.
     */
    [[nodiscard]] const bn::ivector<uint16_t>& graphics_indexes() const
    {
        return _action.graphics_indexes();
    }

    /**
     * @brief Indicates if the action can be updated forever or not.
     */
    [[nodiscard]] bool update_forever() const
    {
        return _action.update_forever();
    }

    /**
     * @brief Returns the current index of the given graphics_indexes
     * (not the current index of the tile set to reference in the given tiles_item).
     */
    [[nodiscard]] int current_index() const
    {
        return _action.current_index();
    }

private:
    bn::sprite_animate_action<MaxSize> _action;
    bn::vector<uint16_t, MaxSize> _wait_updates;
    uint16_t _current_wait_updates_index = 0;
    uint16_t _current_wait_updates = 0;
    bool _started = false;

    variable_wait_sprite_animate_action(const bn::sprite_ptr& sprite, const bn::sprite_tiles_item& tiles_item,
                                        bool forever, const bn::span<const uint16_t>& graphics_indexes,
                                        const bn::span<const uint16_t>& wait_updates)
        : _action(forever ? bn::sprite_animate_action<MaxSize>::forever(sprite, 0, tiles_item, graphics_indexes)
                          : bn::sprite_animate_action<MaxSize>::once(sprite, 0, tiles_item, graphics_indexes))
    {
        BN_ASSERT(graphics_indexes.size() == wait_updates.size(), "graphics_indexes size(", graphics_indexes.size(),
                  ") & wait_updates size(", wait_updates.size(), ") mismatch");

        for (uint16_t wait_update : wait_updates)
        {
            BN_ASSERT(wait_update >= 0, "Invalid wait update: ", wait_update);
            _wait_updates.push_back(wait_update);
        }
    }

    variable_wait_sprite_animate_action(bn::sprite_ptr&& sprite, const bn::sprite_tiles_item& tiles_item, bool forever,
                                        const bn::span<const uint16_t>& graphics_indexes,
                                        const bn::span<const uint16_t>& wait_updates)
        : _action(forever ? bn::sprite_animate_action<MaxSize>::forever(move(sprite), 0, tiles_item, graphics_indexes)
                          : bn::sprite_animate_action<MaxSize>::once(move(sprite), 0, tiles_item, graphics_indexes))
    {
        BN_ASSERT(graphics_indexes.size() == wait_updates.size(), "graphics_indexes size(", graphics_indexes.size(),
                  ") & wait_updates size(", wait_updates.size(), ") mismatch");

        for (uint16_t wait_update : wait_updates)
        {
            BN_ASSERT(wait_update >= 0, "Invalid wait update: ", wait_update);
            _wait_updates.push_back(wait_update);
        }
    }
};

} // namespace ibn

#endif
