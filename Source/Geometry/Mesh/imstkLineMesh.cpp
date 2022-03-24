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
LineMesh::LineMesh() : PointSet(),
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

int
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

void
LineMesh::setCellAttribute(const std::string& arrayName, std::shared_ptr<AbstractDataArray> arr)
{
    m_cellAttributes[arrayName] = arr;
}

std::shared_ptr<AbstractDataArray>
LineMesh::getCellAttribute(const std::string& arrayName) const
{
    auto it = m_cellAttributes.find(arrayName);
    if (it == m_cellAttributes.end())
    {
        LOG(WARNING) << "No array with such name holds any cell data.";
        return nullptr;
    }
    return it->second;
}

bool
LineMesh::hasCellAttribute(const std::string& arrayName) const
{
    return (m_cellAttributes.find(arrayName) != m_cellAttributes.end());
}

void
LineMesh::setCellScalars(const std::string& arrayName, std::shared_ptr<AbstractDataArray> scalars)
{
    m_activeCellScalars = arrayName;
    m_cellAttributes[arrayName] = scalars;
}

void
LineMesh::setCellScalars(const std::string& arrayName)
{
    if (hasCellAttribute(arrayName))
    {
        m_activeCellScalars = arrayName;
    }
}

std::shared_ptr<AbstractDataArray>
LineMesh::getCellScalars() const
{
    if (hasCellAttribute(m_activeCellScalars))
    {
        return m_cellAttributes.at(m_activeCellScalars);
    }
    else
    {
        return nullptr;
    }
}

void
LineMesh::setCellActiveAttribute(std::string& activeAttributeName, std::string attributeName,
                                 const int expectedNumComponents, const ScalarTypeId expectedScalarType)
{
    std::shared_ptr<AbstractDataArray> attribute = m_cellAttributes[attributeName];
    if (attribute->getNumberOfComponents() != expectedNumComponents)
    {
        LOG(WARNING) << "Failed to set cell attribute on LineMesh with "
                     << attribute->getNumberOfComponents() << " components. Expected " <<
            expectedNumComponents << " components.";
        return;
    }
    else if (attribute->getScalarType() != expectedScalarType)
    {
        LOG(INFO) << "Tried to set cell attribute on LineMesh with scalar type "
                  << static_cast<int>(attribute->getScalarType()) << ". Casting to "
                  << static_cast<int>(expectedScalarType) << " scalar type";
        m_cellAttributes[attributeName] = attribute->cast(expectedScalarType);
    }
    activeAttributeName = attributeName;
}
} // namespace imstk