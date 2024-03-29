/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkAbstractCellMesh.h"

namespace imstk
{
void
AbstractCellMesh::clear()
{
    PointSet::clear();

    m_vertexToCells.clear();
    m_vertexToNeighborVertex.clear();
    for (auto i : m_cellAttributes)
    {
        i.second->clear();
    }

    m_activeCellNormals  = "";
    m_activeCellTangents = "";
    m_activeCellScalars  = "";
}

void
AbstractCellMesh::print() const
{
    PointSet::print();

    LOG(INFO) << "Number of cells: " << getNumCells();
    LOG(INFO) << "Active Cell Normals: " << m_activeCellNormals;
    LOG(INFO) << "Active Cell Tangents: " << m_activeCellTangents;
    LOG(INFO) << "Active Cell Scalars: " << m_activeCellScalars;
}

const std::vector<int>
AbstractCellMesh::getCellsForVertex(const int vertexId)
{
    if (vertexId < 0 || vertexId >= getNumVertices())
    {
        return {};
    }

    if (m_vertexToCells.size() == 0)
    {
        computeVertexToCellMap();
    }
    std::vector<int> cells(m_vertexToCells[vertexId].size());
    std::copy(m_vertexToCells[vertexId].begin(), m_vertexToCells[vertexId].end(), cells.begin());
    return cells;
}

void
AbstractCellMesh::setCellActiveAttribute(std::string& activeAttributeName, std::string attributeName,
                                         const int expectedNumComponents, const ScalarTypeId expectedScalarType)
{
    std::shared_ptr<AbstractDataArray> attribute = m_cellAttributes[attributeName];
    if (attribute->getNumberOfComponents() != expectedNumComponents)
    {
        LOG(WARNING) << "Failed to set cell attribute on Mesh " + getName() + " with "
                     << attribute->getNumberOfComponents() << " components. Expected " <<
            expectedNumComponents << " components.";
        return;
    }
    else if (attribute->getScalarType() != expectedScalarType)
    {
        LOG(INFO) << "Tried to set cell attribute on Mesh " + getName() + " with scalar type "
                  << static_cast<int>(attribute->getScalarType()) << ". Casting to "
                  << static_cast<int>(expectedScalarType) << " scalar type";
        m_cellAttributes[attributeName] = attribute->cast(expectedScalarType);
    }
    activeAttributeName = attributeName;
}

void
AbstractCellMesh::setCellScalars(const std::string& arrayName, std::shared_ptr<AbstractDataArray> scalars)
{
    m_activeCellScalars = arrayName;
    m_cellAttributes[arrayName] = scalars;
}

void
AbstractCellMesh::setCellScalars(const std::string& arrayName)
{
    if (hasCellAttribute(arrayName))
    {
        m_activeCellScalars = arrayName;
    }
}

std::shared_ptr<AbstractDataArray>
AbstractCellMesh::getCellScalars() const
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
AbstractCellMesh::setCellNormals(const std::string& arrayName, std::shared_ptr<VecDataArray<double, 3>> normals)
{
    m_activeCellNormals = arrayName;
    m_cellAttributes[arrayName] = normals;
}

void
AbstractCellMesh::setCellNormals(const std::string& arrayName)
{
    if (hasCellAttribute(arrayName))
    {
        setCellActiveAttribute(m_activeCellNormals, arrayName, 3, IMSTK_DOUBLE);
    }
}

std::shared_ptr<VecDataArray<double, 3>>
AbstractCellMesh::getCellNormals() const
{
    if (hasCellAttribute(m_activeCellNormals))
    {
        return std::dynamic_pointer_cast<VecDataArray<double, 3>>(m_cellAttributes.at(m_activeCellNormals));
    }
    else
    {
        return nullptr;
    }
}

void
AbstractCellMesh::setCellTangents(const std::string& arrayName, std::shared_ptr<VecDataArray<double, 3>> tangents)
{
    m_activeCellTangents = arrayName;
    m_cellAttributes[arrayName] = tangents;
}

void
AbstractCellMesh::setCellTangents(const std::string& arrayName)
{
    if (hasCellAttribute(arrayName))
    {
        setCellActiveAttribute(m_activeCellTangents, arrayName, 3, IMSTK_DOUBLE);
    }
}

std::shared_ptr<VecDataArray<double, 3>>
AbstractCellMesh::getCellTangents() const
{
    if (hasCellAttribute(m_activeCellTangents))
    {
        return std::dynamic_pointer_cast<VecDataArray<double, 3>>(m_cellAttributes.at(m_activeCellTangents));
    }
    else
    {
        return nullptr;
    }
}
} // namespace imstk