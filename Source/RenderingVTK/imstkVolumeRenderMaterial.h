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
