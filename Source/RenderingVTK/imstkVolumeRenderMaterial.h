/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkRenderMaterial.h"

#include <vtkSmartPointer.h>

class vtkVolumeProperty;

namespace imstk
{
class VolumeRenderMaterial : public RenderMaterial
{
public:
    VolumeRenderMaterial();
    ~VolumeRenderMaterial() override = default;

    ///
    /// \brief Get the volume property
    ///
    vtkSmartPointer<vtkVolumeProperty> getVolumeProperty();

    bool getUseAutoSample() const { return m_useAutoSample; }
    void setUseAutoSample(bool useAutoSample) { m_useAutoSample = useAutoSample; }

    double getSampleDistance() const { return m_sampleDistance; }
    void setSampleDistance(double sampleDistance) { m_sampleDistance = sampleDistance; }

protected:
    vtkSmartPointer<vtkVolumeProperty> m_property;

    bool   m_useAutoSample  = true;
    double m_sampleDistance = 0.0;
};
} // namespace imstk
