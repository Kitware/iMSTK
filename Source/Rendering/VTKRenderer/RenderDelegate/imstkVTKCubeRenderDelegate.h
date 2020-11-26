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

namespace imstk
{
///
/// \class VTKCubeRenderDelegate
///
/// \brief Cube render delegate with VTK backend
///
class VTKCubeRenderDelegate : public VTKPolyDataRenderDelegate
{
public:
    ///
    /// \brief Constructor
    ///
    VTKCubeRenderDelegate(std::shared_ptr<VisualModel> visualModel);

    ///
    /// \brief Destructor
    ///
    virtual ~VTKCubeRenderDelegate() override = default;

    ///
    /// \brief Update cube source based on the cube geometry
    ///
    void processEvents() override;
};
} // imstk
