/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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

    m_actor->SetVisibility(m_visualModel->isVisible() ? 1 : 0);
}
} // namespace imstk