/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#pragma once

#include <memory>

namespace imstk
{
// forward declarations
class VolumeRenderMaterial;

class VolumeRenderMaterialPresets
{
public:
    enum Presets
    {
        CT_AAA,
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
    };

    static std::shared_ptr<VolumeRenderMaterial> getPreset(int p = CT_BONES);
};
}
