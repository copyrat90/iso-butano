// SPDX-FileCopyrightText: Copyright 2021-2025 Guyeon Yu <copyrat90@gmail.com>
// SPDX-License-Identifier: Zlib

#pragma once

#ifndef IBN_CFG_STATS_ENABLED
#define IBN_CFG_STATS_ENABLED true
#endif

#if IBN_CFG_STATS_ENABLED
// Call this once per frame.
#define IBN_STATS_UPDATE ibn::stats::instance().update()
// Call this in EVERY run-time function.
#define IBN_STATS_UPDATE_IW ibn::stats::instance().update_iw()
#else
#define IBN_STATS_UPDATE ((void)0)
#define IBN_STATS_UPDATE_IW ((void)0)
#endif

#if IBN_CFG_STATS_ENABLED

#include <bn_common.h>

#include <cstdint>

namespace ibn
{

class stats
{
public:
    /// @brief Gets the singleton instance.
    static auto instance() -> stats&
    {
        static BN_DATA_EWRAM_BSS stats inst;
        return inst;
    }

private:
    stats() = default;

public:
    /// @brief Call this once per frame.
    void update();

    /// @brief Call this in @b every run-time function.
    void update_iw();

private:
    volatile std::uint32_t _last_used_cpu = 0;        // %
    volatile std::uint32_t _used_ew = 0;              // max 262144
    volatile std::uint16_t _max_used_iw = 0;          // max 32768
    volatile std::uint16_t _used_bg_tiles = 0;        // max 2048
    volatile std::uint16_t _used_bg_maps = 0;         // max 32768
    volatile std::uint16_t _used_bg_palettes = 0;     // max 256
    volatile std::uint16_t _used_sprite_tiles = 0;    // max 1024
    volatile std::uint16_t _used_sprite_palettes = 0; // max 256
    volatile std::uint16_t _used_bgs = 0;             // default max 4
    volatile std::uint16_t _used_sprites = 0;         // default max 128
};

} // namespace ibn

#endif // IBN_CFG_STATS_ENABLED
