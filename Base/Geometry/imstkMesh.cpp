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
const std::vector<Vec3d>&
Mesh::getInitialVertexPositions() const
{
    return m_initialVertexPositions;
}

const imstk::Vec3d& Mesh::getInitialVertexPosition(const int vertNum) const
{
    return m_initialVertexPositions.at(vertNum);
}

void
Mesh::setInitialVertexPositions(const std::vector<Vec3d>& vertices)
{
    m_initialVertexPositions = vertices;
}

const std::vector<Vec3d>&
Mesh::getVertexPositions() const
{
    return m_vertexPositions;
}

void
Mesh::setVertexPositions(const std::vector<Vec3d>& vertices)
{
    m_vertexPositions = vertices;
}

const imstk::Vec3d& Mesh::getVertexPosition(const int vertNum) const
{
    return m_vertexPositions.at(vertNum);
}

void Mesh::setVertexPosition(const int vertNum, const imstk::Vec3d& pos)
{
    m_vertexPositions.at(vertNum) = pos;
}

const std::vector<Vec3d>&
Mesh::getVertexDisplacements() const
{
    return m_vertexDisplacements;
}

void
Mesh::setVertexDisplacements(const std::vector<Vec3d>& diff)
{
    m_vertexDisplacements = diff;
}

const imstk::Vec3d& Mesh::getVertexDisplacement(const int vertNum) const
{
    return m_vertexDisplacements.at(vertNum);
}

int Mesh::getNumVertices() const
{
    return m_initialVertexPositions.size();
}

void Mesh::computeBoundingBox(imstk::Vec3d& min, imstk::Vec3d& max, const double percent) const
{
    min = imstk::Vec3d(std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max());

    max = imstk::Vec3d(std::numeric_limits<double>::min(),
        std::numeric_limits<double>::min(),
        std::numeric_limits<double>::min());

    for (auto it = m_vertexPositions.begin(); it != m_vertexPositions.end(); ++it)
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
        imstk::Vec3d range = max - min;
        min = min - range*(percent / 100);
        max = max + range*(percent / 100);
    }
}

}
