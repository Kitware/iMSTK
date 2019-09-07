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

namespace imstk
{
class Graph;

///
/// \class PointSet
///
/// \brief Base class for all geometries represented by discrete points and elements
///
class PointSet : public Geometry
{
public:
    ///
    /// \brief Protected constructor
    ///
    PointSet(const Type type = Geometry::Type::PointSet, const std::string name = std::string("")) :
        Geometry(type, name) {}

    ///
    /// \brief Initializes the data structure given vertex positions
    ///
    void initialize(const StdVectorOfVec3d& vertices);

    ///
    /// \brief Clears all the mesh data
    ///
    virtual void clear();

    ///
    /// \brief Print the mesh info
    ///
    virtual void print() const override;

    ///
    /// \brief Returns the volume of the geometry (if valid)
    ///
    virtual double getVolume() const override { return 0; }

    ///
    /// \brief Compute the bounding box for the entire mesh
    ///
    virtual void computeBoundingBox(Vec3d& lowerCorner, Vec3d& upperCorner, const double paddingPercent = 0.0) override;

    // Accessors

    ///
    /// \brief Sets initial positions from an array
    ///
    void setInitialVertexPositions(const StdVectorOfVec3d& vertices);

    ///
    /// \brief Returns the vector of initial positions of the mesh vertices
    ///
    const StdVectorOfVec3d& getInitialVertexPositions() const;

    ///
    /// \brief Returns the initial position of a vertex given its index
    ///
    const Vec3d& getInitialVertexPosition(const size_t vertNum) const;

    ///
    /// \brief Sets current vertex positions of the mesh from an array
    ///
    void setVertexPositions(const StdVectorOfVec3d& vertices);

    ///
    /// \brief Returns the vector of current positions of the mesh vertices
    ///
    const StdVectorOfVec3d& getVertexPositions(DataType type = DataType::PostTransform);

    ///
    /// \brief Set the current position of a vertex given its index to certain position (this is not a thread-safe method)
    ///
    void setVertexPosition(const size_t vertNum, const Vec3d& pos);

    ///
    /// \brief Returns the position of a vertex given its index
    ///
    const Vec3d& getVertexPosition(const size_t vertNum, DataType type = DataType::PostTransform);

    ///
    /// \brief Sets the displacements of mesh vertices from an array
    ///
    void setVertexDisplacements(const StdVectorOfVec3d& diff);

    ///
    /// \brief Sets the displacements of mesh vertices from a linearized displacement vector
    ///
    void setVertexDisplacements(const Vectord& u);

    ///
    /// \brief Concatenate the displacements of mesh vertices using the given 3D vector
    ///
    void translateVertices(const Vec3d& t);

    ///
    /// \brief Sets the point data for all arrays at each vertex
    ///
    void setPointDataMap(const std::map<std::string, StdVectorOfVectorf>& pointData);

    ///
    /// \brief Get the map of the point data for all arrays at each vertex
    ///
    const std::map<std::string, StdVectorOfVectorf>& getPointDataMap() const;

    ///
    /// \brief Set a data array holding some point data
    ///
    void setPointDataArray(const std::string& arrayName, const StdVectorOfVectorf& arrayData);

    ///
    /// \brief Get a specific data array. If the array name cannot be found, nullptr is returned.
    ///
    const StdVectorOfVectorf* getPointDataArray(const std::string& arrayName) const;

    ///
    /// \brief Check if a specific data array exists.
    ///
    bool hasPointDataArray(const std::string& arrayName) const;

    ///
    /// \brief Returns the number of total vertices in the mesh
    ///
    size_t getNumVertices() const;

    ///
    /// \brief Set the topologyChanged flag
    ///
    void setTopologyChangedFlag(const bool flag) { m_topologyChanged = flag; }
    bool getTopologyChangedFlag() const { return m_topologyChanged; }

    ///
    /// \brief Set load factor
    /// \param loadFactor the maximum number of vertices; a multiple of the original vertex count
    ///
    virtual void setLoadFactor(double loadFactor);
    virtual double getLoadFactor();

    ///
    /// \brief Get the maximum number of vertices
    ///
    size_t getMaxNumVertices();

protected:
    friend class VTKPointSetRenderDelegate;

    ///
    /// \brief Get vertices positions
    ///
    StdVectorOfVec3d& getVertexPositionsNotConst() { return m_vertexPositions; }

    void applyTranslation(const Vec3d t) override;
    void applyRotation(const Mat3d r) override;
    void applyScaling(const double s) override;
    void updatePostTransformData() override;

    StdVectorOfVec3d m_initialVertexPositions;                ///> Initial positions of vertices
    StdVectorOfVec3d m_vertexPositions;                       ///> Current positions of vertices
    StdVectorOfVec3d m_vertexPositionsPostTransform;          ///> Positions of vertices after transform

    std::map<std::string, StdVectorOfVectorf> m_pointDataMap; ///> vector of data arrays per vertice

    bool   m_topologyChanged     = false;
    double m_loadFactor          = 2.0;
    size_t m_maxNumVertices      = 0;
    size_t m_originalNumVertices = 0;
};
} // imstk
