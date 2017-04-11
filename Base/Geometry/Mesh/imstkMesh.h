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

namespace imstk
{

///
/// \class Mesh
///
/// \brief Base class for all geometries represented by discrete points and elements
///
class Mesh : public Geometry
{
public:
    ///
    /// \brief Destructor
    ///
    ~Mesh() = default;
    //Mesh() = default;

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
    /// \brief Compute the bounding box for the entire mesh
    ///
    void computeBoundingBox(Vec3d& min, Vec3d& max, const double percent = 0.0) const;

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
    const Vec3d& getInitialVertexPosition(const size_t& vertNum) const;

    ///
    /// \brief Sets current vertex positions of the mesh from an array
    ///
    void setVertexPositions(const StdVectorOfVec3d& vertices);

    ///
    /// \brief Returns the vector of current positions of the mesh vertices
    ///
    const StdVectorOfVec3d& getVertexPositions() const;

    ///
    /// \brief Set the current position of a vertex given its index to certain position
    ///
    void setVertexPosition(const size_t& vertNum, const Vec3d& pos);

    ///
    /// \brief Returns the position of a vertex given its index
    ///
    const Vec3d& getVertexPosition(const size_t& vertNum) const;

    ///
    /// \brief Sets the displacements of mesh vertices from an array
    ///
    void setVertexDisplacements(const StdVectorOfVec3d& diff);

    ///
    /// \brief Sets the displacements of mesh vertices from a linearized displacement vector
    ///
    void setVertexDisplacements(const Vectord& u);

    ///
    /// \brief Returns the vector displacements of mesh vertices
    ///
    const StdVectorOfVec3d& getVertexDisplacements() const;

    ///
    /// \brief Returns the displacement of a given vertex
    ///
    const Vec3d& getVertexDisplacement(const size_t& vertNum) const;

    ///
    /// \brief Sets the point data for all arrays at each vertice
    ///
    void setPointDataMap(const std::map<std::string, StdVectorOfVectorf>& pointData);

    ///
    /// \brief Get the map of the point data for all arrays at each vertice
    ///
    const std::map<std::string, StdVectorOfVectorf>& getPointDataMap() const;

    ///
    /// \brief Set a data array holding some point data
    ///
    void setPointDataArray(const std::string& arrayName, const StdVectorOfVectorf& arrayData);

    ///
    /// \brief Get a specific data array
    ///
    const StdVectorOfVectorf& getPointDataArray(const std::string& arrayName) const;

    ///
    /// \brief Returns the number of total vertices in the mesh
    ///
    const size_t getNumVertices() const;

protected:

    ///
    /// \brief Protected constructor
    ///
    Mesh(Geometry::Type type) : Geometry(type) {}

    friend class VTKSurfaceMeshRenderDelegate;
    friend class VTKTetrahedralMeshRenderDelegate;
    friend class VTKLineMeshRenderDelegate;
    friend class VTKHexahedralMeshRenderDelegate;

    ///
    /// \brief Get vertices positions
    ///
    StdVectorOfVec3d& getVerticesPositionsNotConst()
    {
        return m_vertexPositions;
    }

    //   Orientation * Scaling * initialVerticesPositions
    // + Position (Initial translation)
    // + verticesDisplacements
    // = verticesPositions
    StdVectorOfVec3d m_initialVertexPositions; //> Initial positions of vertices
    StdVectorOfVec3d m_vertexPositions; //> Current positions of vertices
    StdVectorOfVec3d m_vertexDisplacements; //> Displacements of vertices

    std::map<std::string, StdVectorOfVectorf> m_pointDataMap; ///> vector of data arrays per vertice
};

} // imstk

#endif // ifndef imstkMesh_h
