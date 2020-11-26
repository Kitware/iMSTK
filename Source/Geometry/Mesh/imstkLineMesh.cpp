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
#include "imstkLogger.h"
#include "imstkVecDataArray.h"

namespace imstk
{
LineMesh::LineMesh(const std::string& name) : PointSet(Type::LineMesh, name),
    m_segmentIndices(std::make_shared<VecDataArray<int, 2>>())
{
}

void
LineMesh::initialize(std::shared_ptr<VecDataArray<double, 3>> vertices,
                     std::shared_ptr<VecDataArray<int, 2>> lines)
{
    this->clear();

    PointSet::initialize(vertices);

    this->setLinesIndices(lines);
}

void
LineMesh::clear()
{
    PointSet::clear();
    if (m_segmentIndices != nullptr)
    {
        m_segmentIndices->clear();
    }
}

void
LineMesh::print() const
{
    PointSet::print();
    LOG(INFO) << "Number of lines: " << this->getNumLines();
    LOG(INFO) << "Lines:";
    for (auto& segment : *m_segmentIndices)
    {
        LOG(INFO) << segment[0] << ", " << segment[1];
    }
}

size_t
LineMesh::getNumLines() const
{
    return m_segmentIndices->size();
}

const Vec2i&
LineMesh::getLineIndices(const size_t pos) const
{
    return (*m_segmentIndices)[pos];
}

Vec2i&
LineMesh::getLineIndices(const size_t pos)
{
    return (*m_segmentIndices)[pos];
}
} // imstk
