/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <memory>

namespace imstk
{
class VolumeRenderMaterial;

///
/// \brief TODO
///
class VolumeRenderMaterialPresets
{
public:
    enum class Presets
    {
        CT_AAA = 0,
        CT_AAA2,
        CT_BONE,
        CT_BONES,
        CT_CARDIAC,
        CT_CARDIAC2,
        CT_CARDIAC3,
        CT_CHEST_CONTRAST_ENHANCED,
        CT_CHEST_VESSELS,
        CT_CORONARY_ARTERIES,
        CT_CORONARY_ARTERIES_2,
        CT_CORONARY_ARTERIES_3,
        CT_VOLUME_BONE,
        CT_FAT,
        CT_LIVER_VASCULATURE,
        CT_LUNG,
        CT_MIP,
        CT_MUSCLE,
        CT_PULMONARY_ARTERIES,
        CT_SOFT_TISSUE,
        CT_AIR,
        CT_X_RAY,
        MR_ANGIO,
        MR_DEFAULT,
        MR_MIP,
        MR_T2_BRAIN,
        NUM_PRESETS
    };

    ///
    /// \brief TODO
    ///
    static std::shared_ptr<VolumeRenderMaterial> getPreset(Presets p = Presets::CT_BONES);

    // Convenience API to get preset names
    static const char* getPresetName(Presets p);
};
} // namespace imstk
