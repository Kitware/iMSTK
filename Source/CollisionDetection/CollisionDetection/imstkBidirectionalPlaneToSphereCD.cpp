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

#include "imstkBidirectionalPlaneToSphereCD.h"
#include "imstkNarrowPhaseCD.h"
#include "imstkCollisionData.h"

namespace imstk
{
BidirectionalPlaneToSphereCD::BidirectionalPlaneToSphereCD(std::shared_ptr<Plane>         planeA,
                                                           std::shared_ptr<Sphere>        sphereB,
                                                           std::shared_ptr<CollisionData> colData) :
    CollisionDetection(CollisionDetection::Type::BidirectionalPlaneToSphere, colData),
    m_planeA(planeA),
    m_sphereB(sphereB)
{
}

void
BidirectionalPlaneToSphereCD::computeCollisionData()
{
    m_colData->clearAll();
    NarrowPhaseCD::bidirectionalPlaneToSphere(m_planeA.get(), m_sphereB.get(), m_colData);
}
} // namespace imstk
