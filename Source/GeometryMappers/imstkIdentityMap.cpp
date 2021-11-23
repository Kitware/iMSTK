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

#include "imstkIdentityMap.h"
#include "imstkLogger.h"

namespace imstk
{
void
IdentityMap::apply()
{
    // Check Map active
    if (!m_isActive)
    {
        LOG(WARNING) << "Identity map is not active";
        return;
    }

    // Check geometries
    CHECK(m_parentGeom != nullptr && m_childGeom != nullptr) << "Identity map is being applied without valid geometries";

    // Set the child mesh configuration to be same as that of parent
    m_childGeom->setTranslation(m_parentGeom->getTranslation());
    m_childGeom->setRotation(m_parentGeom->getRotation());
}

const RigidTransform3d
IdentityMap::getTransform() const
{
    return RigidTransform3d::Identity();
}
} // imstk
