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

#include "imstkVTKPolyDataRenderDelegate.h"

class vtkSphereSource;

namespace imstk
{
///
/// \class VTKSphereRenderDelegate
///
/// \brief Sphere object render delegate with VTK backend
///
class VTKSphereRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    ///
    /// \brief Constructor
    ///
    VTKSphereRenderDelegate(std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief Destructor
    ///
    virtual ~VTKSphereRenderDelegate() override = default;

    ///
    /// \brief Update sphere source based on the sphere geometry
    ///
    void processEvents() override;

protected:
    vtkSmartPointer<vtkSphereSource> m_sphereSource;
};
} // imstk
