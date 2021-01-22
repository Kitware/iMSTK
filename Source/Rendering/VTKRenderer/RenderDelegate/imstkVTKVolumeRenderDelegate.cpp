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

#include "imstkVTKVolumeRenderDelegate.h"
#include "imstkVolumeRenderMaterial.h"
#include "imstkVisualModel.h"

#include <vtkGPUVolumeRayCastMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

namespace imstk
{
void
VTKVolumeRenderDelegate::updateRenderProperties()
{
    std::shared_ptr<VolumeRenderMaterial> material = std::dynamic_pointer_cast<VolumeRenderMaterial>(m_visualModel->getRenderMaterial());

    if (material != nullptr)
    {
        if (vtkSmartPointer<vtkVolumeMapper> volumeMapper = vtkVolumeMapper::SafeDownCast(m_mapper))
        {
            if (vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeRayCastMapper = vtkGPUVolumeRayCastMapper::SafeDownCast(volumeMapper))
            {
                volumeRayCastMapper->SetAutoAdjustSampleDistances(material->getUseAutoSample());
                volumeRayCastMapper->SetSampleDistance(material->getSampleDistance());
            }

            switch (material->getBlendMode())
            {
            case RenderMaterial::BlendMode::Alpha:
                volumeMapper->SetBlendMode(vtkVolumeMapper::COMPOSITE_BLEND);
                break;
            case RenderMaterial::BlendMode::Additive:
                volumeMapper->SetBlendMode(vtkVolumeMapper::ADDITIVE_BLEND);
                break;
            case RenderMaterial::BlendMode::MaximumIntensity:
                volumeMapper->SetBlendMode(vtkVolumeMapper::MAXIMUM_INTENSITY_BLEND);
                break;
            case RenderMaterial::BlendMode::MinimumIntensity:
                volumeMapper->SetBlendMode(vtkVolumeMapper::MINIMUM_INTENSITY_BLEND);
                break;
            default:
                volumeMapper->SetBlendMode(vtkVolumeMapper::COMPOSITE_BLEND);
                break;
            }
        }

        if (vtkSmartPointer<vtkVolume> volume = vtkVolume::SafeDownCast(m_actor))
        {
            volume->SetProperty(material->getVolumeProperty());
        }
    }

    m_actor->SetVisibility(m_visualModel->getVisible());
}
}