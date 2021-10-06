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

#include "imstkPointSet.h"

#include <array>

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
    ///
    /// \brief Constructor
    ///
    LineMesh(const std::string& name = std::string(""));

    ///
    /// \brief Deconstructor
    ///
    virtual ~LineMesh() override = default;

    ///
    /// \brief Returns the string representing the type name of the geometry
    ///
    virtual const std::string getTypeName() const override { return "LineMesh"; }

public:
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

// Accessors
public:
    ///
    /// \brief Get the number of segments/cells
    ///
    int getNumLines() const;

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
    ///
    const Vec2i& getLineIndices(const size_t pos) const;
    Vec2i& getLineIndices(const size_t pos);

// Attributes
public:
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
    ///
    void setCellScalars(const std::string& arrayName, std::shared_ptr<AbstractDataArray> scalars);
    void setCellScalars(const std::string& arrayName);
    std::string getActiveCellScalars() const { return m_activeCellScalars; }
    std::shared_ptr<AbstractDataArray> getCellScalars() const;

protected:
    void setCellActiveAttribute(std::string& activeAttributeName, std::string attributeName,
                                const int expectedNumComponents, const ScalarTypeId expectedScalarType);

protected:
    std::shared_ptr<VecDataArray<int, 2>> m_segmentIndices;   ///> line connectivity

    std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> m_cellAttributes;
    std::string m_activeCellScalars = "";

    size_t m_originalNumLines = 0;
    size_t m_maxNumLines      = 0;
};
} // imstk
