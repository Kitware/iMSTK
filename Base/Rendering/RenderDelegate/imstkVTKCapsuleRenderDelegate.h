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

#ifndef imstkVTKCapsuleRenderDelegate_h
#define imstkVTKCapsuleRenderDelegate_h

#include <memory>

#include "imstkVTKRenderDelegate.h"
#include "imstkCapsule.h"

#include "vtkCapsuleSource.h"

namespace imstk
{

///
/// \class CapsuleRenderDelegate
///
/// \brief Render capsule using vtk api
///
class VTKCapsuleRenderDelegate : public VTKRenderDelegate
{
public:
    ///
    /// \brief Default destructor
    ///
    ~VTKCapsuleRenderDelegate() = default;

    ///
    /// \brief Constructor
    ///
    VTKCapsuleRenderDelegate(std::shared_ptr<Capsule> capsule);

    ///
    /// \brief Returns the capsule geometry object
    ///
    std::shared_ptr<Geometry>getGeometry() const override { return m_capsuleGeometry; }

protected:
    std::shared_ptr<Capsule> m_capsuleGeometry; ///>Capsule geometry
};

} // imstk

#endif // ifndef imstkCapsuleRenderDelegate_h