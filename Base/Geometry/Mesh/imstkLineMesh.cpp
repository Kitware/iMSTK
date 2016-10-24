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
LineMesh::clear()
{}

void
LineMesh::print() const
{
    Mesh::print();
}

double
LineMesh::getVolume() const
{
    return 0.0;
}

void
LineMesh::setConnectivity(const std::vector<std::vector<int> >& lines)
{
    m_lines = lines;
}

int
LineMesh::getNumLines()
{
    return m_lines.size();
}

std::vector<std::vector<int>>
LineMesh::getLines() const
{
    return m_lines;
}

std::vector<int>
LineMesh::getLine(int index) const
{
    return m_lines[index];
}

} // imstk
