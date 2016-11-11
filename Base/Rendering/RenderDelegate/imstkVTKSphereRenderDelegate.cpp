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

#include "imstkVTKSphereRenderDelegate.h"

#include "g3log/g3log.hpp"

namespace imstk
{

VTKSphereRenderDelegate::VTKSphereRenderDelegate(std::shared_ptr<Sphere>sphere) :
    m_geometry(sphere)
{
    auto source = vtkSmartPointer<vtkSphereSource>::New();

    source->SetCenter(WORLD_ORIGIN[0], WORLD_ORIGIN[1], WORLD_ORIGIN[2]);
    source->SetRadius(m_geometry->getRadius());
    source->SetPhiResolution(20);
    source->SetThetaResolution(20);

    this->setActorMapper(source->GetOutputPort());
    this->updateActorTransform();
}

std::shared_ptr<Geometry>
VTKSphereRenderDelegate::getGeometry() const
{
    return m_geometry;
}

} // imstk
