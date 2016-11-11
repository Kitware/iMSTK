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

#include "imstkVTKPlaneRenderDelegate.h"

#include "g3log/g3log.hpp"

namespace imstk
{

VTKPlaneRenderDelegate::VTKPlaneRenderDelegate(std::shared_ptr<Plane>plane) :
    m_geometry(plane)
{
    auto source = vtkSmartPointer<vtkPlaneSource>::New();

    source->SetCenter(WORLD_ORIGIN[0], WORLD_ORIGIN[1], WORLD_ORIGIN[2]);
    source->SetNormal(UP_VECTOR[0], UP_VECTOR[1], UP_VECTOR[2]);

    this->setActorMapper(source->GetOutputPort());
    this->updateActorTransform();
}

std::shared_ptr<Geometry>
VTKPlaneRenderDelegate::getGeometry() const
{
    return m_geometry;
}

} // imstk
