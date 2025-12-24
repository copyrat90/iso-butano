// SPDX-FileCopyrightText: Copyright 2021-2025 Guyeon Yu <copyrat90@gmail.com>
// SPDX-License-Identifier: Zlib

#include "ibn_stats.h"

#include <bn_bg_maps.h>
#include <bn_bg_palettes.h>
#include <bn_bg_tiles.h>
#include <bn_bgs.h>
#include <bn_core.h>
#include <bn_memory.h>
#include <bn_sprite_palettes.h>
#include <bn_sprite_tiles.h>
#include <bn_sprites.h>

#if IBN_CFG_STATS_ENABLED

namespace ibn
{

void stats::update()
{
    _last_used_cpu = static_cast<std::uint32_t>((bn::core::last_cpu_usage() * 100).ceil_integer());
    _used_ew = static_cast<std::uint32_t>(bn::memory::used_static_ewram() + bn::memory::used_alloc_ewram());
    _used_bg_tiles = static_cast<std::uint16_t>(bn::bg_tiles::used_tiles_count());
    _used_bg_maps = static_cast<std::uint16_t>(bn::bg_maps::used_cells_count());
    _used_bg_palettes = static_cast<std::uint16_t>(bn::bg_palettes::used_colors_count());
    _used_sprite_tiles = static_cast<std::uint16_t>(bn::sprite_tiles::used_tiles_count());
    _used_sprite_palettes = static_cast<std::uint16_t>(bn::sprite_palettes::used_colors_count());
    _used_bgs = static_cast<std::uint16_t>(bn::bgs::used_items_count());
    _used_sprites = static_cast<std::uint16_t>(bn::sprites::used_items_count());
}

void stats::update_iw()
{
    const int cur_iw = bn::memory::used_static_iwram() + bn::memory::used_stack_iwram();
    if (cur_iw > _max_used_iw)
        _max_used_iw = cur_iw;
}

} // namespace ibn

#endif // IBN_CFG_STATS_ENABLED
