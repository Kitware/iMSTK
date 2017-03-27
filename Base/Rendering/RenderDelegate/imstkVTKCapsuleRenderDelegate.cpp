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

#include "imstkVTKCapsuleRenderDelegate.h"

namespace imstk
{

VTKCapsuleRenderDelegate::VTKCapsuleRenderDelegate(std::shared_ptr<Capsule> capsule) :
m_capsuleGeometry(capsule)
{
    auto source = vtkSmartPointer<vtkCapsuleSource>::New();

    source->SetCenter(WORLD_ORIGIN[0], WORLD_ORIGIN[1], WORLD_ORIGIN[2]);
    source->SetRadius(m_capsuleGeometry->getRadius());
    source->SetCylinderLength(m_capsuleGeometry->getHeight());

    // Set the resolution for rendering
    source->SetLatLongTessellation(10);
    source->SetPhiResolution(10);
    source->SetThetaResolution(10);

    // Setup Mapper & Actor
    this->setUpMapper(source->GetOutputPort());
    this->updateActorTransform();
}

} // imstk