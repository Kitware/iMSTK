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

#include "imstkIsometricMap.h"
namespace imstk {

void
IsometricMap::setTransform(const RigidTransform3d& affineTransform)
{
    m_rigidTransform = affineTransform;
}

const imstk::RigidTransform3d
IsometricMap::getTransform() const
{
    return m_rigidTransform;
}

void IsometricMap::print() const
{
    std::cout << this->getTypeName() << std::endl;
}

void
IsometricMap::applyMap()
{
    if (m_isActive)
    {
        if (!m_master || !m_slave)
        {
            LOG(WARNING) << "Isometric map is being applied without valid geometries\n";
            return;
        }

        // First set the follower mesh configuration to that of master
        m_slave->setPosition(m_master->getPosition());
        m_slave->setOrientation(m_master->getOrientation());

        // Now, apply the offset transform
        m_slave->transform(m_rigidTransform);
    }
}

}