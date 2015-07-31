// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef SMMESH_H
#define SMMESH_H

// STL includes
#include <vector>

// SimMedTK includes
#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "Core/ErrorLog.h"
#include "Core/Vector.h"
#include "Core/Matrix.h"
#include "Core/CollisionConfig.h"
#include "Core/Geometry.h"

#define SIMMEDTK_MESH_AABBSKINFACTOR 0.1  ///Bounding box skin value
#define SIMMEDTK_MESH_RESERVEDMAXEDGES 6000  ///this value is initially allocated buffer size for thge edges

// VegaFEM class
class ObjMesh;

struct TexCoord;
struct Triangle;
struct Tetrahedra;
struct Edge;


/// \brief !!
struct TextureAttachment
{
    TextureAttachment();
    int textureId;
};

/// \brief base class for the mesh
class BaseMesh: public CoreClass
{
public:
    /// \brief designates what purpose/scenario the mesh is used for
    enum class MeshType
    {
        Deformable,
        DeformableCutable,
        RigidCutable,
        Rigid
    };

    /// \brief designates input mesh file type
    enum class MeshFileType
    {
        None,
        Obj,
        ThreeDS,
        Volume,
    };

    /// \brief constructor
    BaseMesh();

    /// \brief query if the mesh has textures available for rendering
    bool isMeshTextured();

    /// \brief assign the texture
    void assignTexture(int p_textureId);

    /// \brief assign the texture
    void assignTexture(const std::string& p_referenceName);

    /// \brief update the original texture vertices with the current
    void updateOriginalVertsWithCurrent();

    const std::vector<core::Vec3d> &getVertices() const
    {
        return this->vertices;
    }

    std::vector<core::Vec3d> &getVertices()
    {
        return this->vertices;
    }

    int getNumVertices()
    {
        return nbrVertices;
    }

public:
    CollisionGroup collisionGroup; ///< !!
    GLint renderingID; ///< !!
    std::shared_ptr<ErrorLog> log; ///< record the log
    std::vector<core::Vec3d> vertices; ///< vertices co-ordinate data at time t
    std::vector<core::Vec3d> origVerts; ///< vertices co-ordinate data at time t=0
    int  nbrVertices; ///< number of vertices
    AABB aabb; ///< Axis aligned bounding box
    bool isTextureCoordAvailable; ///< true if the texture co-ordinate is available
    TexCoord *texCoord; ///< texture co-ordinates
    std::vector<TextureAttachment> textureIds; ///< !!
};

/// \brief: this is a generic Mesh class from which surface and volume meshes are inherited
/// Note: this class cannot exist on its own
class Mesh: public BaseMesh
{
public:
    /// \brief constructor
    Mesh();

    /// \brief destructor
    virtual ~Mesh();

    /// \brief compute the neighbors of the vertex
    void getVertexNeighbors();

    /// \brief compute the neighbors of the triangle
    void getTriangleNeighbors();

    /// \brief initialize vertex arrays
    bool initVertexArrays(int nbr);

    /// \brief initialize triangle arrays
    bool initTriangleArrays(int nbr);

    /// \brief initialize the neighbors of the vertex
    void initVertexNeighbors();

    /// \brief !!
    void allocateAABBTris();

    /// \brief compute the normal of a triangle
    core::Vec3d calculateTriangleNormal(int triNbr);

    /// \brief update the normals of triangles after they moved
    void updateTriangleNormals();

    /// \brief update the normals of vertices after they moved
    void updateVertexNormals();

    /// \brief update AABB after the mesh moved
    void upadateAABB();

    /// \brief update AABB of each triangle after mesh moved
    void updateTriangleAABB();

    /// \brief compute triangle tangents
    void calcTriangleTangents();

    /// \brief compute the tangent give the three vertices
    void calculateTangent(core::Vec3d& p1, core::Vec3d& p2, core::Vec3d& p3, TexCoord& t1, TexCoord& t2, TexCoord& t3, core::Vec3d& t);

    /// \brief !!
    void calculateTangent_test(core::Vec3d& p1, core::Vec3d& p2, core::Vec3d& p3, TexCoord& t1, TexCoord& t2, TexCoord& t3, core::Vec3d& t);

    /// \brief find the neighbors of all vertices of mesh
    void calcNeighborsVertices();

    /// \brief find all the edges of the mesh
    void calcEdges();

    /// \brief translate the mesh
    void translate(float, float, float);

    /// \brief translate the mesh
    void translate(core::Vec3d p_offset);

    /// \brief scale the mesh
    void scale(core::Vec3d p_scaleFactors);

    /// \brief rotate the mesh
    void rotate(const Matrix33d &p_rot);

    /// \brief check if there is a consistent orientation of triangle vertices
    /// across the entire surface mesh
    void checkCorrectWinding();

    /// \brief get the type fo mesh
    MeshType getMeshType()
    {
        return meshType;
    };

    /// \brief load the mesh
    virtual bool loadMesh(const std::string& fileName, const MeshFileType &fileType) = 0;

    /// \brief load the mesh
    bool importSurfaceMeshFromVegaFormat(std::shared_ptr<ObjMesh> vegaSurfaceMesh, const bool perProcessingStage);

    /// \brief update the surface mesh data after the deformation
    void updateSurfaceMeshFromVegaFormat(std::shared_ptr<ObjMesh> vegaSurfaceMesh);

    /// \brief get number of triangles
    int getNumTriangles()  const;

    /// \brief get number of edges
    int getNumEdges()  const;

public:
    int  nbrTriangles; ///< number of triangles
    std::vector<Triangle> triangles; ///< list of triangles
    TexCoord *texCoordForTrianglesOBJ; ///< !! tansel for OBJ
    int nbrTexCoordForTrainglesOBJ; ///< !! tansel for OBJ
    core::Vec3d *triNormals; ///< triangle normals
    core::Vec3d *vertNormals; ///< vertex normals
    core::Vec3d *triTangents; ///< triangle tangents
    core::Vec3d *vertTangents; ///< vertex tangents
    bool tangentChannel; ///< !!
    std::vector< std::vector<int> > vertTriNeighbors; ///< list of neighbors for a triangle
    std::vector< std::vector<int> > vertVertNeighbors; ///< list of neighbors for a vertex
    std::vector<Edge> edges; ///< list of edges

    ///AABBB of the mesh.
    ///This value is allocated and computed by only collision detection module
    ///Therefore it is initially nullptr
    std::vector<AABB> triAABBs;

    MeshType meshType; ///< type of mesh (rigid, deformable etc.)
    MeshFileType meshFileType; ///< type of input mesh
};

/// \brief holds the texture co-ordinates
struct TexCoord
{
    float u, v;
};

/// \brief holds the vertex indices of triangle
struct Triangle
{
    std::array<unsigned int,3> vert;
};

/// \brief holds the vertex indices of tetrahedron
struct Tetrahedra
{
    std::array<unsigned int,4> vert;
};

/// \brief holds the vertex indices of edge
struct Edge
{
    std::array<unsigned int,2> vert;
};

/// \brief !!
class LineMesh: public BaseMesh
{
public:
    /// \brief destructor
    ~LineMesh()
    {
        delete[]edgeAABBs;
        delete[]texCoord;
        delete[]edges;
    }

    /// \brief constructor
    LineMesh(int p_nbrVertices);

    /// \brief constructor
    LineMesh(int p_nbrVertices, bool autoEdge);

    /// \brief !!
    void createAutoEdges();

    /// \brief !!
    virtual void createCustomEdges() {};

    /// \brief updat AABB when the mesh moves
    void updateAABB();

    /// \brief translate the vertices of mesh
    void translate(float p_offsetX, float p_offsetY, float p_offsetZ);

    /// \brief translate the vertices of mesh
    void translate(core::Vec3d p_offset);

    /// \brief scale the mesh
    void scale(core::Vec3d p_scaleFactors);

    /// \brief rotate the mesh
    void rotate(Matrix33d p_rot);

    /// \brief query if the mesh is textured
    bool isMeshTextured();

public:
    AABB *edgeAABBs;///< AABBs for the edges in the mesh
    Edge *edges;///< edges of the line mesh
    int nbrEdges;///< number of edges of the line mesh

};

#endif
