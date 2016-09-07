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

#ifndef imstkSphereRenderDelegate_h
#define imstkSphereRenderDelegate_h

#include <memory>

#include "imstkRenderDelegate.h"
#include "imstkSphere.h"

#include "vtkSphereSource.h"

namespace imstk
{

///
/// \class SphereRenderDelegate
///
/// \brief
///
class SphereRenderDelegate : public RenderDelegate
{
public:
    ///
    /// \brief
    ///
    ~SphereRenderDelegate() = default;

    ///
    /// \brief
    ///
    SphereRenderDelegate(std::shared_ptr<Sphere>sphere);

    ///
    /// \brief
    ///
    std::shared_ptr<Geometry>getGeometry() const override;

protected:
    std::shared_ptr<Sphere> m_geometry; ///>
};

} // imstk

#endif // ifndef imstkSphereRenderDelegate_h