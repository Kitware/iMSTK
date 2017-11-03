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

namespace imstk
{

DecalPool::DecalPool(unsigned int maxNumDecals /*= 128*/)
    : Geometry(Geometry::Type::DecalPool)
{
    if (maxNumDecals <= 256)
    {
        m_maxNumDecals = maxNumDecals;
    }
    else
    {
        m_maxNumDecals = 256;
        LOG(WARNING) << "The maximum number of decals is 256";
    }

    m_vertexPositions[0] = glm::vec3(0.5, 0.5, 0.5);
    m_vertexPositions[1] = glm::vec3(0.5, 0.5, -0.5);
    m_vertexPositions[2] = glm::vec3(0.5, -0.5, 0.5);
    m_vertexPositions[3] = glm::vec3(0.5, -0.5, -0.5);
    m_vertexPositions[4] = glm::vec3(-0.5, 0.5, 0.5);
    m_vertexPositions[5] = glm::vec3(-0.5, 0.5, -0.5);
    m_vertexPositions[6] = glm::vec3(-0.5, -0.5, 0.5);
    m_vertexPositions[7] = glm::vec3(-0.5, -0.5, -0.5);

    m_triangles[0] = glm::ivec3(2, 1, 0);
    m_triangles[1] = glm::ivec3(1, 2, 3);
    m_triangles[2] = glm::ivec3(4, 5, 6);
    m_triangles[3] = glm::ivec3(7, 6, 5);
    m_triangles[4] = glm::ivec3(0, 1, 4);
    m_triangles[5] = glm::ivec3(5, 4, 1);
    m_triangles[6] = glm::ivec3(6, 3, 2);
    m_triangles[7] = glm::ivec3(3, 6, 7);
    m_triangles[8] = glm::ivec3(4, 2, 0);
    m_triangles[9] = glm::ivec3(2, 4, 6);
    m_triangles[10] = glm::ivec3(1, 3, 5);
    m_triangles[11] = glm::ivec3(7, 5, 3);

    for (unsigned int i = 0; i < maxNumDecals; i++)
    {
        m_freeDecals.push_back(std::make_shared<Decal>());
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