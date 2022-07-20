/*=========================================================================
   Library: iMSTK
   Copyright (c) Kitware, Inc.

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

#include "imstkPointSet.h"
#include "imstkVecDataArray.h"

#include <unordered_set>

#pragma once

namespace imstk
{
///
/// \class AbstractCellMesh
///
/// \brief Provides non templated base for cell based meshes
///
class AbstractCellMesh : public PointSet
{
public:
    AbstractCellMesh() = default;
    ~AbstractCellMesh() override = default;

    ///
    /// \brief Returns true if the geometry is a mesh, else returns false
    ///
    bool isMesh() const override { return true; }

    void clear() override;

    ///
    /// \brief Print the surface mesh
    ///
    void print() const override;

    virtual int getNumCells() const = 0;

    ///
    /// \brief Computes neighboring cells for all vertices
    ///
    virtual void computeVertexToCellMap() { }

    ///
    /// \brief Computes neighboring vertices for all vertices
    ///
    virtual void computeVertexNeighbors() { }

    ///
    /// \brief Get cells as abstract array. Overridden by derived classes to return
    ///     cells as point indices.
    ///
    virtual std::shared_ptr<AbstractDataArray> getAbstractCells() const = 0;

    ///
    /// \brief Returns map of vertices to cells that contain the vertex (reverse linkage)
    ///
    const std::vector<std::unordered_set<int>>& getVertexToCellMap() const { return m_vertexToCells; }

    ///
    /// \brief Returns map of vertices to neighboring vertices
    ///
    const std::vector<std::unordered_set<int>>& getVertexNeighbors() const { return m_vertexToNeighborVertex; }

    // Attributes
    ///
    /// \brief Get the cell attributes map
    ///
    const std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>>& getCellAttributes() const { return m_cellAttributes; }

    void setCellAttribute(const std::string& arrayName, std::shared_ptr<AbstractDataArray> arr)
    {
        m_cellAttributes[arrayName] = arr;
    }

    std::shared_ptr<AbstractDataArray> getCellAttribute(const std::string& name) const
    {
        auto it = m_cellAttributes.find(name);
        if (it == m_cellAttributes.end())
        {
            LOG(FATAL) << "No attribute with name " << name << " found in " << getTypeName();
            return nullptr;
        }
        return it->second;
    }

    ///
    /// \brief Check if a specific data array exists.
    ///
    bool hasCellAttribute(const std::string& arrayName) const
    {
        return (m_cellAttributes.find(arrayName) != m_cellAttributes.end());
    }

    ///
    /// \brief Set the cell attributes map
    ///
    void setCellAttributes(std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> attributes) { m_cellAttributes = attributes; }

    ///
    /// \brief Get/Set the active scalars
    ///@{
    void setCellScalars(const std::string& arrayName, std::shared_ptr<AbstractDataArray> scalars);
    void setCellScalars(const std::string& arrayName);
    std::string getActiveCellScalars() const { return m_activeCellScalars; }
    std::shared_ptr<AbstractDataArray> getCellScalars() const;
    ///@}

    ///
    /// \brief Get/Set the active normals
    ///@{
    void setCellNormals(const std::string& arrayName, std::shared_ptr<VecDataArray<double, 3>> normals);
    void setCellNormals(const std::string& arrayName);
    std::string getActiveCellNormals() const { return m_activeCellNormals; }
    std::shared_ptr<VecDataArray<double, 3>> getCellNormals() const;
    ///@}

    ///
    /// \brief Get/Set the active tangents
    ///@{
    void setCellTangents(const std::string& arrayName, std::shared_ptr<VecDataArray<double, 3>> tangents);
    void setCellTangents(const std::string& arrayName);
    std::string getActiveCellTangents() const { return m_activeCellTangents; }
    std::shared_ptr<VecDataArray<double, 3>> getCellTangents() const;
///@}

protected:
    void setCellActiveAttribute(std::string& activeAttributeName, std::string attributeName,
                                const int expectedNumComponents, const ScalarTypeId expectedScalarType);

    std::vector<std::unordered_set<int>> m_vertexToCells;          ///< Map of vertices to neighbor cells
    std::vector<std::unordered_set<int>> m_vertexToNeighborVertex; ///< Map of vertice sto neighbor vertices

    ///< Per cell attributes
    std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> m_cellAttributes;

    std::string m_activeCellNormals  = "";
    std::string m_activeCellTangents = "";
    std::string m_activeCellScalars  = "";
};
} // namespace imstk