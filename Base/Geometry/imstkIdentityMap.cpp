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

namespace imstk {

    const imstk::RigidTransform3d& IdentityMap::getTransform() const
    {
        return imstk::RigidTransform3d::Identity();
    }

    void IdentityMap::applyMap()
    {
        if (m_isActive)
        {
            if (!m_master || !m_slave)
            {
                LOG(WARNING) << "Identity map is being applied without valid geometries\n";
                return;
            }

            // Set the follower mesh configuration to be same as that of master
            m_slave->setPosition(m_master->getPosition());
            m_slave->setOrientation(m_master->getOrientation());
        }
    }
}