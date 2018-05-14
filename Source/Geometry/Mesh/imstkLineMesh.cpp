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

#include "imstkLineMesh.h"

#include "g3log/g3log.hpp"

namespace imstk
{
void
LineMesh::initialize(const StdVectorOfVec3d& vertices,
                     const std::vector<LineArray>& lines)
{
    this->clear();

    PointSet::initialize(vertices);

    this->setLinesVertices(lines);
}

void
LineMesh::clear()
{
    m_lines.clear();
    m_vertexColors.clear();
}

void
LineMesh::print() const
{
    PointSet::print();
}

double
LineMesh::getVolume() const
{
    return 0.0;
}

void
LineMesh::setVertexColors(const std::vector<Color>& colors)
{
    if (colors.size() <= m_maxNumVertices)
    {
        m_vertexColors = colors;
    }
    else
    {
        LOG(WARNING) << "Vertex colors not set, exceeded maximum number of vertices";
    }
}

void
LineMesh::setLinesVertices(const std::vector<LineArray>& lines)
{
    if (m_originalNumLines == 0)
    {
        m_originalNumLines = lines.size();
        m_maxNumLines = (size_t)(m_originalNumLines * m_loadFactor);
        m_lines.reserve(m_maxNumLines);
    }

    if (lines.size() <= m_maxNumLines)
    {
        m_topologyChanged = true;
        m_lines = lines;
    }
    else
    {
        LOG(WARNING) << "Lines not set, exceeded maximum number of lines";
    }
}

size_t
LineMesh::getNumLines()
{
    return m_lines.size();
}

std::vector<LineMesh::LineArray>
LineMesh::getLinesVertices() const
{
    return m_lines;
}

std::vector<Color>
LineMesh::getVertexColors() const
{
    return m_vertexColors;
}
} // imstk
