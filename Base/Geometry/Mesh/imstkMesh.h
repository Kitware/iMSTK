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

#ifndef imstkMesh_h
#define imstkMesh_h

#include "imstkGeometry.h"

namespace imstk {

///
/// \class Mesh
///
/// \brief Base class for all geometries represented by discrete points and elements
///
class Mesh : public Geometry
{
public:

    ~Mesh() = default;

    ///
    /// \brief Initializes the data structure given vertex positions
    ///
    void initialize(const std::vector<Vec3d>& vertices);

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
    void computeBoundingBox(Vec3d& min, Vec3d& max, const double percent = 0.0) const;

    // Accessors

    ///
    /// \brief Sets initial positions from an array
    ///
    void setInitialVerticesPositions(const std::vector<Vec3d>& vertices);

    ///
    /// \brief Returns the vector of initial positions of the mesh vertices
    ///
    const std::vector<Vec3d>& getInitialVerticesPositions() const;

    ///
    /// \brief Returns the initial position of a vertex given its index
    ///
    const Vec3d& getInitialVertexPosition(const int& vertNum) const;

    ///
    /// \brief Sets current vertex positions of the mesh from an array
    ///
    void setVerticesPositions(const std::vector<Vec3d>& vertices);

    ///
    /// \brief Returns the vector of current positions of the mesh vertices
    ///
    const std::vector<Vec3d>& getVerticesPositions() const;

    ///
    /// \brief Set the current position of a vertex given its index to certain position
    ///
    void setVerticePosition(const int &vertNum, const Vec3d& pos);

    ///
    /// \brief Returns the position of a vertex given its index
    ///
    const Vec3d& getVertexPosition(const int& vertNum) const;

    ///
    /// \brief Sets the displacements of mesh vertices from an array
    ///
    void setVerticesDisplacements(const std::vector<Vec3d>& diff);

    ///
    /// \brief Sets the displacements of mesh vertices from a linearized displacement vector
    ///
    void setVerticesDisplacements(const Vectord& u);

    ///
    /// \brief Returns the vector displacements of mesh vertices
    ///
    const std::vector<Vec3d>& getVerticesDisplacements() const;

    ///
    /// \brief Returns the displacement of a given vertex
    ///
    const Vec3d& getVerticeDisplacement(const int& vertNum) const;

    ///
    /// \brief Sets the point data for all arrays at each vertice
    ///
    void setPointDataMap(const std::map<std::string, std::vector<Vectorf>>& pointData);

    ///
    /// \brief Get the map of the point data for all arrays at each vertice
    ///
    const std::map<std::string, std::vector<Vectorf>>& getPointDataMap() const;

    ///
    /// \brief Set a data array holding some point data
    ///
    void setPointDataArray(const std::string& arrayName, const std::vector<Vectorf>& arrayData);

    ///
    /// \brief Get a specific data array
    ///
    const std::vector<Vectorf>& getPointDataArray(const std::string& arrayName) const;

    ///
    /// \brief Returns the number of total vertices in the mesh
    ///
    const int getNumVertices() const;

protected:

    Mesh(Geometry::Type type) : Geometry(type) {}

    friend class SurfaceMeshRenderDelegate;
    friend class TetrahedralMeshRenderDelegate;
    std::vector<Vec3d>& getVerticesPositionsNotConst()
    {
        return m_verticesPositions;
    }

    //   Orientation * Scaling * initialVerticesPositions
    // + Position (Initial translation)
    // + verticesDisplacements
    // = verticesPositions
    std::vector<Vec3d> m_initialVerticesPositions; //> Initial positions of vertices
    std::vector<Vec3d> m_verticesPositions; //> Current positions of vertices
    std::vector<Vec3d> m_verticesDisplacements; //> Displacements of vertices

    std::map<std::string, std::vector<Vectorf>> m_pointDataMap; ///> vector of data arrays per vertice
};
}

#endif // ifndef imstkMesh_h
