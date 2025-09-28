#pragma once

#include "ibn_enum_as_flags.h"

#include <bn_bg_palettes_actions.h>
#include <bn_bgs_mosaic_actions.h>
#include <bn_blending_actions.h>
#include <bn_dmg_music_actions.h>
#include <bn_music_actions.h>
#include <bn_optional.h>
#include <bn_sound_actions.h>
#include <bn_sprites_mosaic_actions.h>

#include <cstdint>
#include <limits>
#include <type_traits>

namespace ibn
{

/// @brief Simple global effects transitions manager.
/// This manages blending, mosaic and volumes at the same time.
class transitions
{
public:
    enum class kinds : std::uint32_t
    {
        NONE = 0,

        FADE = (1u << 0),
        TRANSPARENCY = (1u << 1),
        INTENSITY = (1u << 2),
        SPRITES_MOSAIC_HORIZONTAL = (1u << 3),
        SPRITES_MOSAIC_VERTICAL = (1u << 4),
        BGS_MOSAIC_HORIZONTAL = (1u << 5),
        BGS_MOSAIC_VERTICAL = (1u << 6),
        BG_PALS_FADE = (1u << 7),
        BG_PALS_BRIGHTNESS = (1u << 8),
        BG_PALS_GRAYSCALE = (1u << 9),
        BG_PALS_CONTRAST = (1u << 10),
        BG_PALS_HUE_SHIFT = (1u << 11),
        BG_PALS_INTENSITY = (1u << 12),
        MUSIC_VOLUME = (1u << 13),
        DMG_MUSIC_VOLUME = (1u << 14),
        SOUND_VOLUME = (1u << 15),

        SPRITES_MOSAIC = SPRITES_MOSAIC_HORIZONTAL | SPRITES_MOSAIC_VERTICAL,
        BGS_MOSAIC = BGS_MOSAIC_HORIZONTAL | BGS_MOSAIC_VERTICAL,

        ALL = std::numeric_limits<std::underlying_type_t<kinds>>::max()
    };

public:
    /// @brief Call this once per frame.
    void update();

public:
    /// @brief Helper function to set the alpha values for many effects once.
    /// @note Keep in mind that the transparency/volume alpha value is inverted.
    /// (`0` being fully visible/audible, `1` being fully transparent/muted.)
    /// @param flags Effect kind(s) to apply the alpha value.
    /// @param alpha Alpha value to apply. `[0..1]`
    void set_alpha(kinds flags, bn::fixed alpha);

    /// @brief Starts a transition from the current alpha value to the final alpha value.
    /// @note Keep in mind that the transparency/volume alpha value is inverted.
    /// (`0` being fully visible/audible, `1` being fully transparent/muted.)
    /// @param flags Effect kind(s) to transit.
    /// @param duration_updates Number of times that the transitions must be updated to complete.
    /// @param final_alpha Final alpha value when the transitions are done. `[0..1]`
    void start(kinds flags, int duration_updates, bn::fixed final_alpha);

    /// @brief Indicates if all the specified transitions are completed.
    /// @param flags Effect kind(s) to check if they are all done.
    bool done(kinds flags = kinds::ALL) const;

    /// @brief Clears the transition.
    /// @note This never resets the alpha value.
    /// @param flags Effect kind(s) to clear.
    void clear(kinds flags = kinds::ALL);

private:
    bn::optional<bn::blending_fade_alpha_to_action> _fade_action;
    bn::optional<bn::blending_transparency_alpha_to_action> _transparency_action;
    bn::optional<bn::blending_intensity_alpha_to_action> _intensity_action;
    bn::optional<bn::sprites_mosaic_horizontal_stretch_to_action> _sprites_mosaic_h_action;
    bn::optional<bn::sprites_mosaic_vertical_stretch_to_action> _sprites_mosaic_v_action;
    bn::optional<bn::bgs_mosaic_horizontal_stretch_to_action> _bgs_mosaic_h_action;
    bn::optional<bn::bgs_mosaic_vertical_stretch_to_action> _bgs_mosaic_v_action;
    bn::optional<bn::bg_palettes_fade_to_action> _bg_pals_fade_action;
    bn::optional<bn::bg_palettes_brightness_to_action> _bg_pals_brightness_action;
    bn::optional<bn::bg_palettes_grayscale_to_action> _bg_pals_grayscale_action;
    bn::optional<bn::bg_palettes_contrast_to_action> _bg_pals_contrast_action;
    bn::optional<bn::bg_palettes_hue_shift_to_action> _bg_pals_hue_shift_action;
    bn::optional<bn::bg_palettes_intensity_to_action> _bg_pals_intensity_action;
    bn::optional<bn::music_volume_to_action> _music_volume_action;
    bn::optional<bn::dmg_music_volume_to_action> _dmg_music_volume_action;
    bn::optional<bn::sound_master_volume_to_action> _sound_volume_action;
};

IBN_ENUM_AS_FLAGS(transitions::kinds);

} // namespace ibn
