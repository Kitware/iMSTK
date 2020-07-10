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

#include "imstkVTKRenderDelegate.h"

class vtkTransformPolyDataFilter;

namespace imstk
{
///
/// \class VTKCapsuleRenderDelegate
///
/// \brief Render capsule object with vtk backend
///
class VTKCapsuleRenderDelegate : public VTKRenderDelegate
{
public:
    ///
    /// \brief Constructor
    ///
    explicit VTKCapsuleRenderDelegate(std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief destructor
    ///
    virtual ~VTKCapsuleRenderDelegate() override = default;

    ///
    /// \brief Update capsule source based on the capsule geometry
    ///
    void updateDataSource() override;

protected:
    vtkSmartPointer<vtkTransformPolyDataFilter> m_transformFilter; ///> Source
};
} // imstk
