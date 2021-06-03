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

#include "imstkDecalPool.h"
#include "imstkDecal.h"
#include "imstkLogger.h"

namespace imstk
{
DecalPool::DecalPool(unsigned int maxNumDecals /*= 128*/, const std::string& name)
    : Geometry(name)
{
    if (maxNumDecals <= 128)
    {
        m_maxNumDecals = maxNumDecals;
    }
    else
    {
        m_maxNumDecals = 128;
        LOG(WARNING) << "The maximum number of decals is 128";
    }

    m_vertexPositions[0] = Vec3d(0.5, 0.5, 0.5);
    m_vertexPositions[1] = Vec3d(0.5, 0.5, -0.5);
    m_vertexPositions[2] = Vec3d(0.5, -0.5, 0.5);
    m_vertexPositions[3] = Vec3d(0.5, -0.5, -0.5);
    m_vertexPositions[4] = Vec3d(-0.5, 0.5, 0.5);
    m_vertexPositions[5] = Vec3d(-0.5, 0.5, -0.5);
    m_vertexPositions[6] = Vec3d(-0.5, -0.5, 0.5);
    m_vertexPositions[7] = Vec3d(-0.5, -0.5, -0.5);

    m_triangles[0]  = Vec3i(2, 1, 0);
    m_triangles[1]  = Vec3i(1, 2, 3);
    m_triangles[2]  = Vec3i(4, 5, 6);
    m_triangles[3]  = Vec3i(7, 6, 5);
    m_triangles[4]  = Vec3i(0, 1, 4);
    m_triangles[5]  = Vec3i(5, 4, 1);
    m_triangles[6]  = Vec3i(6, 3, 2);
    m_triangles[7]  = Vec3i(3, 6, 7);
    m_triangles[8]  = Vec3i(4, 2, 0);
    m_triangles[9]  = Vec3i(2, 4, 6);
    m_triangles[10] = Vec3i(1, 3, 5);
    m_triangles[11] = Vec3i(7, 5, 3);

    for (unsigned int i = 0; i < maxNumDecals; i++)
    {
        m_freeDecals.push_back(std::make_shared<Decal>("Decal-" + std::to_string(i)));
    }
}

std::shared_ptr<Decal>
DecalPool::addDecal()
{
    if (m_numDecals >= m_maxNumDecals && !m_recycle)
    {
        LOG(WARNING) << "Decal pool exhausted, unable to add more decals";
        return nullptr;
    }

    if (m_recycle && m_numDecals >= m_maxNumDecals)
    {
        m_freeDecals.push_back(m_orderedDecals.front());
        m_orderedDecals.pop_front();
    }

    auto decal = m_freeDecals.front();
    m_orderedDecals.push_back(decal);
    m_freeDecals.pop_front();

    if (m_numDecals < m_maxNumDecals)
    {
        m_numDecals++;
    }

    return decal;
}

void
DecalPool::removeDecal()
{
    m_freeDecals.push_back(m_orderedDecals.front());
    m_orderedDecals.pop_front();
    m_numDecals--;
}

std::deque<std::shared_ptr<Decal>>&
DecalPool::getDecals()
{
    return m_orderedDecals;
}

void
DecalPool::setRecycle(bool recycle)
{
    m_recycle = recycle;
}

bool
DecalPool::getRecycle()
{
    return m_recycle;
}

unsigned int
DecalPool::getNumDecals()
{
    return m_numDecals;
}

unsigned int
DecalPool::getMaxNumDecals()
{
    return m_maxNumDecals;
}
}
