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

#pragma once

#include "imstkMacros.h"
#include "imstkPointSet.h"
#include "imstkVecDataArray.h"

#include <array>
#include <unordered_set>

namespace imstk
{
struct Color;

///
/// \class LineMesh
///
/// \brief Base class for all volume mesh types
///
class LineMesh : public PointSet
{
public:
    LineMesh();
    ~LineMesh() override = default;

    IMSTK_TYPE_NAME(LineMesh)

    ///
    /// \brief Initializes the rest of the data structures given vertex positions and
    ///  line connectivity
    ///
    void initialize(std::shared_ptr<VecDataArray<double, 3>> vertices, std::shared_ptr<VecDataArray<int, 2>> lines);

    ///
    /// \brief
    ///
    void clear() override;

    ///
    /// \brief
    ///
    void print() const override;

    ///
    /// \brief Returns true if the geometry is a mesh, else returns false
    ///
    bool isMesh() const override { return true; }

    ///
    /// \brief Computes neighboring cells for all vertices
    ///
    void computeVertexToCellMap();

    ///
    /// \brief Computes neighboring vertices for all vertices
    ///
    void computeNeighborVertices();

// Accessors
    ///
    /// \brief Get the number of segments/cells
    ///
    int getNumLines() const;

    ///
    /// \brief Returns the indices of the faces neighboring a vertex
    /// ComputeVertexToCellMap can be called to produce these
    ///
    const std::vector<std::unordered_set<int>>& getVertexToCellMap() { return m_vertexToCells; }

    ///
    /// \brief Returns the indices of the vertices neighboring a vertex
    /// ComputeVertexNeighborVertices can be called to produce these
    ///
    const std::vector<std::unordered_set<int>>& getVertexNeighbors() { return m_vertexToNeighborVertex; }

    ///
    /// \brief Set the connectivity of the segments
    ///
    void setLinesIndices(std::shared_ptr<VecDataArray<int, 2>> lines) { m_segmentIndices = lines; }

    ///
    /// \brief Get the connectivity of the segments
    ///
    std::shared_ptr<VecDataArray<int, 2>> getLinesIndices() const { return m_segmentIndices; }

    ///
    /// \brief Get the connectivity of a segment
    ///@{
    const Vec2i& getLineIndices(const size_t pos) const;
    Vec2i& getLineIndices(const size_t pos);
    ///@}

    ///
    /// \brief Get cells as abstract array.
    ///
    const AbstractDataArray* getCellIndices() const override { return m_segmentIndices.get(); }

// Attributes
    ///
    /// \brief Set a data array holding some per cell data
    ///
    void setCellAttribute(const std::string& arrayName, std::shared_ptr<AbstractDataArray> arr);

    ///
    /// \brief Get a specific data array. If the array name cannot be found, nullptr is returned.
    ///
    std::shared_ptr<AbstractDataArray> getCellAttribute(const std::string& arrayName) const;

    ///
    /// \brief Get the cell attributes map
    ///
    const std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>>& getCellAttributes() const { return m_cellAttributes; }

    ///
    /// \brief Check if a specific data array exists.
    ///
    bool hasCellAttribute(const std::string& arrayName) const;

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

protected:
    void setCellActiveAttribute(std::string& activeAttributeName, std::string attributeName,
                                const int expectedNumComponents, const ScalarTypeId expectedScalarType);

    std::shared_ptr<VecDataArray<int, 2>> m_segmentIndices;         ///< line connectivity
    std::vector<std::unordered_set<int>>  m_vertexToCells;          ///< Reverse linkage, gives vertex ids -> N cell ids
    std::vector<std::unordered_set<int>>  m_vertexToNeighborVertex; ///< Gives map of vertex ids to neighboring vertex ids

    std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> m_cellAttributes;
    std::string m_activeCellScalars = "";

    size_t m_originalNumLines = 0;
    size_t m_maxNumLines      = 0;
};
} // namespace imstk
