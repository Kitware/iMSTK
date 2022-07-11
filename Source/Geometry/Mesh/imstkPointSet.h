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

#include "imstkGeometry.h"
#include "imstkTypes.h"

#include <unordered_map>

namespace imstk
{
class AbstractDataArray;
template<typename T, int N> class VecDataArray;

///
/// \class PointSet
///
/// \brief Base class for all geometries represented by discrete points and elements
/// The pointsets follow a pipeline:
///
class PointSet : public Geometry
{
public:
    PointSet();
    ~PointSet() override = default;

    IMSTK_TYPE_NAME(PointSet)

    ///
    /// \brief Initializes the data structure given vertex positions
    ///
    void initialize(std::shared_ptr<VecDataArray<double, 3>> positions);

    ///
    /// \brief Clears all the mesh data
    ///
    virtual void clear();

    ///
    /// \brief Print the mesh info
    ///
    virtual void print() const override;

    ///
    /// \brief Compute the bounding box for the entire mesh
    ///
    virtual void computeBoundingBox(Vec3d& lowerCorner, Vec3d& upperCorner, const double paddingPercent = 0.0) override;

// Accessors
    ///
    /// \brief Sets initial positions from an array
    ///
    void setInitialVertexPositions(std::shared_ptr<VecDataArray<double, 3>> vertices);

    ///
    /// \brief Returns the vector of initial positions of the mesh vertices
    ///
    std::shared_ptr<VecDataArray<double, 3>> getInitialVertexPositions() const { return m_initialVertexPositions; }

    ///
    /// \brief Returns the initial position of a vertex given its index
    ///
    Vec3d& getInitialVertexPosition(const size_t vertNum);

    ///
    /// \brief Sets current vertex positions of the mesh
    ///
    void setVertexPositions(std::shared_ptr<VecDataArray<double, 3>> positions);

    ///
    /// \brief Returns the vector of current positions of the mesh vertices
    ///
    std::shared_ptr<VecDataArray<double, 3>> getVertexPositions(DataType type = DataType::PostTransform) const;

    ///
    /// \brief Set the current position of a vertex given its index to certain position (this is not a thread-safe method)
    ///
    void setVertexPosition(const size_t vertNum, const Vec3d& pos);

    ///
    /// \brief Returns the position of a vertex given its index
    ///@{
    const Vec3d& getVertexPosition(const size_t vertNum, DataType type = DataType::PostTransform) const;
    Vec3d& getVertexPosition(const size_t vertNum, DataType type       = DataType::PostTransform);
    ///@}

    ///
    /// \brief Returns the number of total vertices in the mesh
    ///
    int getNumVertices() const;

// Attributes
    ///
    /// \brief Set a data array holding some per vertex data
    ///
    void setVertexAttribute(const std::string& arrayName, std::shared_ptr<AbstractDataArray> arr);

    ///
    /// \brief Get a specific data array. If the array name cannot be found, nullptr is returned.
    ///
    std::shared_ptr<AbstractDataArray> getVertexAttribute(const std::string& arrayName) const;

    ///
    /// \brief Check if a specific data array exists.
    ///
    bool hasVertexAttribute(const std::string& arrayName) const;

    ///
    /// \brief Set the vertex attributes map
    ///
    void setVertexAttributes(std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> attributes) { m_vertexAttributes = attributes; }

    ///
    /// \brief Get the vertex attributes map
    ///
    const std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>>& getVertexAttributes() const { return m_vertexAttributes; }

    ///
    /// \brief Get/Set the active scalars
    ///@{
    void setVertexScalars(const std::string& arrayName, std::shared_ptr<AbstractDataArray> scalars);
    void setVertexScalars(const std::string& arrayName);
    std::string getActiveVertexScalars() const { return m_activeVertexScalars; }
    std::shared_ptr<AbstractDataArray> getVertexScalars() const;
    ///@}

    ///
    /// \brief Get/Set the active normals
    ///@{
    void setVertexNormals(const std::string& arrayName, std::shared_ptr<VecDataArray<double, 3>> normals);
    void setVertexNormals(const std::string& arrayName);
    std::string getActiveVertexNormals() const { return m_activeVertexNormals; }
    std::shared_ptr<VecDataArray<double, 3>> getVertexNormals() const;
    ///@}

    ///
    /// \brief Get/Set the active tangents
    ///@{
    void setVertexTangents(const std::string& arrayName, std::shared_ptr<VecDataArray<float, 3>> tangents);
    void setVertexTangents(const std::string& arrayName);
    std::string getActiveVertexTangents() const { return m_activeVertexTangents; }
    std::shared_ptr<VecDataArray<float, 3>> getVertexTangents() const;
    ///@}

    ///
    /// \brief Get/Set the active tcoords
    ///@{
    void setVertexTCoords(const std::string& arrayName, std::shared_ptr<VecDataArray<float, 2>> tcoords);
    void setVertexTCoords(const std::string& arrayName);
    std::string getActiveVertexTCoords() const { return m_activeVertexTCoords; }
    std::shared_ptr<VecDataArray<float, 2>> getVertexTCoords() const;
    ///@}

    ///
    /// \brief Applies the geometries member transform to produce currPositions
    ///
    void updatePostTransformData() const override;

protected:
    ///
    /// \brief Applies transformation m directly the initial and post transform data
    ///
    void applyTransform(const Mat4d& m) override;

    ///
    /// \brief Sets the active vertex attribute name as long as the # components is satisfied.
    /// Throws message and casts if incorrect scalar type is provided.
    ///
    void setActiveVertexAttribute(std::string& activeAttributeName, const std::string attributeName,
                                  const int expectedNumComponents, const ScalarTypeId expectedScalarType);

    std::shared_ptr<VecDataArray<double, 3>> m_initialVertexPositions;
    std::shared_ptr<VecDataArray<double, 3>> m_vertexPositions;

    std::unordered_map<std::string, std::shared_ptr<AbstractDataArray>> m_vertexAttributes;
    std::string m_activeVertexNormals  = "";
    std::string m_activeVertexScalars  = "";
    std::string m_activeVertexTangents = "";
    std::string m_activeVertexTCoords  = "";
};
} // namespace imstk