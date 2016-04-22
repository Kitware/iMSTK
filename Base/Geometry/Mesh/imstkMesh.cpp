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

#include "imstkMesh.h"

namespace imstk {
void
Mesh::setInitialVerticesPositions(const std::vector<Vec3d>& vertices)
{
    m_initialVerticesPositions = vertices;
}

const std::vector<Vec3d>&
Mesh::getInitialVerticesPositions() const
{
    return m_initialVerticesPositions;
}

const Vec3d&
Mesh::getInitialVertexPosition(const int& vertNum) const
{
    return m_initialVerticesPositions.at(vertNum);
}

void
Mesh::setVerticesPositions(const std::vector<Vec3d>& vertices)
{
    m_verticesPositions = vertices;
}

const std::vector<Vec3d>&
Mesh::getVerticesPositions() const
{
    return m_verticesPositions;
}

void Mesh::setVerticePosition(const int& vertNum, const Vec3d& pos)
{
    m_verticesPositions.at(vertNum) = pos;
}

const Vec3d&
Mesh::getVertexPosition(const int& vertNum) const
{
    return m_verticesPositions.at(vertNum);
}

void
Mesh::setVerticesDisplacements(const std::vector<Vec3d>& diff)
{
    m_verticesDisplacements = diff;
}

const std::vector<Vec3d>&
Mesh::getVerticesDisplacements() const
{
    return m_verticesDisplacements;
}

const Vec3d&
Mesh::getVerticeDisplacement(const int& vertNum) const
{
    return m_verticesDisplacements.at(vertNum);
}

const int
Mesh::getNumVertices() const
{
    return m_initialVerticesPositions.size();
}

void
Mesh::computeBoundingBox(Vec3d& min, Vec3d& max, const double percent) const
{
    min = Vec3d(std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max());

    max = Vec3d(std::numeric_limits<double>::min(),
        std::numeric_limits<double>::min(),
        std::numeric_limits<double>::min());

    for (auto it = m_verticesPositions.begin(); it != m_verticesPositions.end(); ++it)
    {
        for (int i = 0; i < 3; ++i)
        {
            min[i] = std::min(min[i], (*it)[i]);
            max[i] = std::max(max[i], (*it)[i]);
        }
    }

    if (percent == 0.0)
    {
        return;
    }
    else
    {
        Vec3d range = max - min;
        min = min - range*(percent / 100);
        max = max + range*(percent / 100);
    }
}
}
