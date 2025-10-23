-- SPDX-FileCopyrightText: Copyright 2021-2025 Guyeon Yu <copyrat90@gmail.com>
-- SPDX-License-Identifier: Zlib

-- Open this script in Mesen2 > Debug > Script Window.
-- You need to call `IBN_STATS_UPDATE` & `IBN_STATS_UPDATE_IW` in `ibn_stats.h` in your C++ sources.

reset_max_cpu_period = 30

last_max_cpu = 0
cur_max_cpu = 0
reset_max_cpu_counter = reset_max_cpu_period

cpu_usages = {}
max_cpu_usages_count = 240

function print_stats()
    local stats = emu.getLabelAddress("ibn_stats_instance_inst")

    if stats then
        if reset_max_cpu_counter == 0 then
            last_max_cpu = cur_max_cpu
            cur_max_cpu = 0
            reset_max_cpu_counter = reset_max_cpu_period
        end
        reset_max_cpu_counter = reset_max_cpu_counter - 1

        local last_used_cpu = emu.read32(stats.address, stats.memType, false)
        local used_ew = emu.read32(stats.address + 4, stats.memType, false)
        local max_used_iw = emu.read16(stats.address + 8, stats.memType, false)
        local used_bg_tiles = emu.read16(stats.address + 10, stats.memType, false)
        local used_bg_maps = emu.read16(stats.address + 12, stats.memType, false)
        local used_bg_palettes = emu.read16(stats.address + 14, stats.memType, false)
        local used_sprite_tiles = emu.read16(stats.address + 16, stats.memType, false)
        local used_sprite_palettes = emu.read16(stats.address + 18, stats.memType, false)

        cur_max_cpu = last_used_cpu > cur_max_cpu and last_used_cpu or cur_max_cpu

        -- store recent cpu logs
        if #cpu_usages >= max_cpu_usages_count then
            table.remove(cpu_usages, 1)
        end
        table.insert(cpu_usages, last_used_cpu)

        -- draw cpu usage graph
        cpu_graph_color = 0x40FFFFFF
        cpu_graph_100_color = 0x400000FF

        emu.drawLine(240 - max_cpu_usages_count, 160 - 100, 240, 160 - 100, cpu_graph_100_color)
        if #cpu_usages >= 2 then
            for i = 1, #cpu_usages - 1 do
                local x1 = 240 - #cpu_usages + i - 1
                local x2 = x1 + 1
                local y1 = 160 - cpu_usages[i]
                local y2 = 160 - cpu_usages[i + 1]
                emu.drawLine(x1, y1, x2, y2, cpu_graph_color)
            end
        end

        -- draw strings
        text_color = 0x80FFFFFF
        bg_color = 0x80000000

        emu.drawString(0, 0 * 9, string.format("cpu: %d%%", last_max_cpu), text_color, bg_color)
        emu.drawString(0, 1 * 9, string.format("iw: %d/%d (%.0f%%)", max_used_iw, 32768, max_used_iw / 32768 * 100),
            text_color, bg_color)
        emu.drawString(0, 2 * 9, string.format("ew: %d/%d (%.0f%%)", used_ew, 262144, used_ew / 262144 * 100), text_color,
            bg_color)
        emu.drawString(0, 3 * 9,
            string.format("bg_tiles: %d/%d (%.0f%%)", used_bg_tiles, 2048, used_bg_tiles / 2048 * 100), text_color,
            bg_color)
        emu.drawString(0, 4 * 9,
            string.format("bg_maps: %d/%d (%.0f%%)", used_bg_maps, 32768, used_bg_maps / 32768 * 100), text_color,
            bg_color)
        emu.drawString(0, 5 * 9,
            string.format("bg_pals: %d/%d (%.0f%%)", used_bg_palettes, 256, used_bg_palettes / 256 * 100), text_color,
            bg_color)
        emu.drawString(0, 6 * 9,
            string.format("spr_tiles: %d/%d (%.0f%%)", used_sprite_tiles, 1024, used_sprite_tiles / 1024 * 100),
            text_color, bg_color)
        emu.drawString(0, 7 * 9,
            string.format("spr_pals: %d/%d (%.0f%%)", used_sprite_palettes, 256, used_sprite_palettes / 256 * 100),
            text_color, bg_color)
    else
        emu.drawString(0, 0, "`ibn::stats` not found!", text_color, bg_color)
    end
end

emu.addEventCallback(print_stats, emu.eventType.endFrame);
emu.displayMessage("ibn", "`ibn_stats.lua` script loaded.")
