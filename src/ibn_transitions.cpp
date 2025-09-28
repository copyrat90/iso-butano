#include "ibn_transitions.h"

#include <bn_assert.h>

#include <concepts>

namespace ibn
{

namespace
{

template <typename T>
concept optional_action = requires(T obj) {
    requires std::same_as<T, bn::optional<typename T::value_type>>;
    obj->update();
    obj->done();
};

template <optional_action OptionalAction>
void update_and_reset_if_done(OptionalAction& action)
{
    if (action.has_value())
    {
        action->update();
        if (action->done())
            action.reset();
    }
}

} // namespace

void transitions::update()
{
    update_and_reset_if_done(_fade_action);
    update_and_reset_if_done(_transparency_action);
    update_and_reset_if_done(_intensity_action);
    update_and_reset_if_done(_sprites_mosaic_h_action);
    update_and_reset_if_done(_sprites_mosaic_v_action);
    update_and_reset_if_done(_bgs_mosaic_h_action);
    update_and_reset_if_done(_bgs_mosaic_v_action);
    update_and_reset_if_done(_bg_pals_fade_action);
    update_and_reset_if_done(_bg_pals_brightness_action);
    update_and_reset_if_done(_bg_pals_grayscale_action);
    update_and_reset_if_done(_bg_pals_contrast_action);
    update_and_reset_if_done(_bg_pals_hue_shift_action);
    update_and_reset_if_done(_bg_pals_intensity_action);
    update_and_reset_if_done(_music_volume_action);
    update_and_reset_if_done(_dmg_music_volume_action);
    update_and_reset_if_done(_sound_volume_action);
}

void transitions::set_alpha(kinds flags, bn::fixed alpha)
{
    if (!!(flags & kinds::FADE))
        bn::blending::set_fade_alpha(alpha);
    if (!!(flags & kinds::TRANSPARENCY))
        bn::blending::set_transparency_alpha(1 - alpha); // inverted
    if (!!(flags & kinds::INTENSITY))
        bn::blending::set_intensity_alpha(alpha);
    if (!!(flags & kinds::SPRITES_MOSAIC_HORIZONTAL))
        bn::sprites_mosaic::set_horizontal_stretch(alpha);
    if (!!(flags & kinds::SPRITES_MOSAIC_VERTICAL))
        bn::sprites_mosaic::set_vertical_stretch(alpha);
    if (!!(flags & kinds::BGS_MOSAIC_HORIZONTAL))
        bn::bgs_mosaic::set_horizontal_stretch(alpha);
    if (!!(flags & kinds::BGS_MOSAIC_VERTICAL))
        bn::bgs_mosaic::set_vertical_stretch(alpha);
    if (!!(flags & kinds::BG_PALS_FADE))
        bn::bg_palettes::set_fade_intensity(alpha);
    if (!!(flags & kinds::BG_PALS_BRIGHTNESS))
        bn::bg_palettes::set_brightness(alpha);
    if (!!(flags & kinds::BG_PALS_GRAYSCALE))
        bn::bg_palettes::set_grayscale_intensity(alpha);
    if (!!(flags & kinds::BG_PALS_CONTRAST))
        bn::bg_palettes::set_contrast(alpha);
    if (!!(flags & kinds::BG_PALS_HUE_SHIFT))
        bn::bg_palettes::set_hue_shift_intensity(alpha / 2); // half
    if (!!(flags & kinds::BG_PALS_INTENSITY))
        bn::bg_palettes::set_intensity(alpha);
    if (!!(flags & kinds::MUSIC_VOLUME))
        bn::music::set_volume(1 - alpha); // inverted
    if (!!(flags & kinds::DMG_MUSIC_VOLUME))
        bn::dmg_music::set_volume(1 - alpha); // inverted
    if (!!(flags & kinds::SOUND_VOLUME))
        bn::sound::set_master_volume(1 - alpha); // inverted
}

void transitions::start(kinds flags, int duration_updates, bn::fixed final_alpha)
{
    BN_ASSERT(!(!!(flags & kinds::FADE) && (!!(flags & kinds::TRANSPARENCY) || !!(kinds::INTENSITY))),
              "Fade and other blendings can't be enabled at the same time");

    if (!!(flags & kinds::FADE))
        _fade_action.emplace(duration_updates, final_alpha);
    if (!!(flags & kinds::TRANSPARENCY))
        _transparency_action.emplace(duration_updates, 1 - final_alpha); // inverted
    if (!!(flags & kinds::INTENSITY))
        _intensity_action.emplace(duration_updates, final_alpha);
    if (!!(flags & kinds::SPRITES_MOSAIC_HORIZONTAL))
        _sprites_mosaic_h_action.emplace(duration_updates, final_alpha);
    if (!!(flags & kinds::SPRITES_MOSAIC_VERTICAL))
        _sprites_mosaic_v_action.emplace(duration_updates, final_alpha);
    if (!!(flags & kinds::BGS_MOSAIC_HORIZONTAL))
        _bgs_mosaic_h_action.emplace(duration_updates, final_alpha);
    if (!!(flags & kinds::BGS_MOSAIC_VERTICAL))
        _bgs_mosaic_v_action.emplace(duration_updates, final_alpha);
    if (!!(flags & kinds::BG_PALS_FADE))
        _bg_pals_fade_action.emplace(duration_updates, final_alpha);
    if (!!(flags & kinds::BG_PALS_BRIGHTNESS))
        _bg_pals_brightness_action.emplace(duration_updates, final_alpha);
    if (!!(flags & kinds::BG_PALS_GRAYSCALE))
        _bg_pals_grayscale_action.emplace(duration_updates, final_alpha);
    if (!!(flags & kinds::BG_PALS_CONTRAST))
        _bg_pals_contrast_action.emplace(duration_updates, final_alpha);
    if (!!(flags & kinds::BG_PALS_HUE_SHIFT))
        _bg_pals_hue_shift_action.emplace(duration_updates, final_alpha / 2); // half
    if (!!(flags & kinds::BG_PALS_INTENSITY))
        _bg_pals_intensity_action.emplace(duration_updates, final_alpha);
    if (!!(flags & kinds::MUSIC_VOLUME))
        _music_volume_action.emplace(duration_updates, 1 - final_alpha); // inverted
    if (!!(flags & kinds::DMG_MUSIC_VOLUME))
        _dmg_music_volume_action.emplace(duration_updates, 1 - final_alpha); // inverted
    if (!!(flags & kinds::SOUND_VOLUME))
        _sound_volume_action.emplace(duration_updates, 1 - final_alpha); // inverted
}

bool transitions::done(kinds flags) const
{
    bool is_done = true;

    if (!!(flags & kinds::FADE))
        is_done &= !_fade_action.has_value() || _fade_action->done();
    if (!!(flags & kinds::TRANSPARENCY))
        is_done &= !_transparency_action.has_value() || _transparency_action->done();
    if (!!(flags & kinds::INTENSITY))
        is_done &= !_intensity_action.has_value() || _intensity_action->done();
    if (!!(flags & kinds::SPRITES_MOSAIC_HORIZONTAL))
        is_done &= !_sprites_mosaic_h_action.has_value() || _sprites_mosaic_h_action->done();
    if (!!(flags & kinds::SPRITES_MOSAIC_VERTICAL))
        is_done &= !_sprites_mosaic_v_action.has_value() || _sprites_mosaic_v_action->done();
    if (!!(flags & kinds::BGS_MOSAIC_HORIZONTAL))
        is_done &= !_bgs_mosaic_h_action.has_value() || _bgs_mosaic_h_action->done();
    if (!!(flags & kinds::BGS_MOSAIC_VERTICAL))
        is_done &= !_bgs_mosaic_v_action.has_value() || _bgs_mosaic_v_action->done();
    if (!!(flags & kinds::BG_PALS_FADE))
        is_done &= !_bg_pals_fade_action.has_value() || _bg_pals_fade_action->done();
    if (!!(flags & kinds::BG_PALS_BRIGHTNESS))
        is_done &= !_bg_pals_brightness_action.has_value() || _bg_pals_brightness_action->done();
    if (!!(flags & kinds::BG_PALS_GRAYSCALE))
        is_done &= !_bg_pals_grayscale_action.has_value() || _bg_pals_grayscale_action->done();
    if (!!(flags & kinds::BG_PALS_CONTRAST))
        is_done &= !_bg_pals_contrast_action.has_value() || _bg_pals_contrast_action->done();
    if (!!(flags & kinds::BG_PALS_HUE_SHIFT))
        is_done &= !_bg_pals_hue_shift_action.has_value() || _bg_pals_hue_shift_action->done();
    if (!!(flags & kinds::BG_PALS_INTENSITY))
        is_done &= !_bg_pals_intensity_action.has_value() || _bg_pals_intensity_action->done();
    if (!!(flags & kinds::MUSIC_VOLUME))
        is_done &= !_music_volume_action.has_value() || _music_volume_action->done();
    if (!!(flags & kinds::DMG_MUSIC_VOLUME))
        is_done &= !_dmg_music_volume_action.has_value() || _dmg_music_volume_action->done();
    if (!!(flags & kinds::SOUND_VOLUME))
        is_done &= !_sound_volume_action.has_value() || _sound_volume_action->done();

    return is_done;
}

void transitions::clear(kinds flags)
{
    if (!!(flags & kinds::FADE))
        _fade_action.reset();
    if (!!(flags & kinds::TRANSPARENCY))
        _transparency_action.reset();
    if (!!(flags & kinds::INTENSITY))
        _intensity_action.reset();
    if (!!(flags & kinds::SPRITES_MOSAIC_HORIZONTAL))
        _sprites_mosaic_h_action.reset();
    if (!!(flags & kinds::SPRITES_MOSAIC_VERTICAL))
        _sprites_mosaic_v_action.reset();
    if (!!(flags & kinds::BGS_MOSAIC_HORIZONTAL))
        _bgs_mosaic_h_action.reset();
    if (!!(flags & kinds::BGS_MOSAIC_VERTICAL))
        _bgs_mosaic_v_action.reset();
    if (!!(flags & kinds::BG_PALS_FADE))
        _bg_pals_fade_action.reset();
    if (!!(flags & kinds::BG_PALS_BRIGHTNESS))
        _bg_pals_brightness_action.reset();
    if (!!(flags & kinds::BG_PALS_GRAYSCALE))
        _bg_pals_grayscale_action.reset();
    if (!!(flags & kinds::BG_PALS_CONTRAST))
        _bg_pals_contrast_action.reset();
    if (!!(flags & kinds::BG_PALS_HUE_SHIFT))
        _bg_pals_hue_shift_action.reset();
    if (!!(flags & kinds::BG_PALS_INTENSITY))
        _bg_pals_intensity_action.reset();
    if (!!(flags & kinds::MUSIC_VOLUME))
        _music_volume_action.reset();
    if (!!(flags & kinds::DMG_MUSIC_VOLUME))
        _dmg_music_volume_action.reset();
    if (!!(flags & kinds::SOUND_VOLUME))
        _sound_volume_action.reset();
}

} // namespace ibn
