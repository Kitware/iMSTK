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
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smErrorLog.h"
#include "smCore/smTextureManager.h"
#include "smUtilities/smVector.h"
#include "smUtilities/smMatrix.h"
#include "smCollision/smCollisionConfig.h"
#include "smCore/smGeometry.h"

#define SIMMEDTK_MESH_AABBSKINFACTOR 0.1  ///Bounding box skin value
#define SIMMEDTK_MESH_RESERVEDMAXEDGES 6000  ///this value is initially allocated buffer size for thge edges

struct smTexCoord;
struct smTriangle;
struct smTetrahedra;
struct smEdge;

/// \brief designates what purpose/scenario the mesh is used for
enum smMeshType
{
    SMMESH_DEFORMABLE,
    SMMESH_DEFORMABLECUTABLE,
    SMMESH_RIGIDCUTABLE,
    SMMESH_RIGID
};

/// \brief designates input mesh file type
enum smMeshFileType
{
    SM_FILETYPE_NONE,
    SM_FILETYPE_OBJ,
    SM_FILETYPE_3DS,
    SM_FILETYPE_VOLUME,
};

class smShader;

/// \brief !!
struct smTextureAttachment
{
    smTextureAttachment();
    smInt textureId;
};

/// \brief base class for the mesh
class smBaseMesh: public smCoreClass
{
public:
    /// \brief constructor
    smBaseMesh();

    /// \brief query if the mesh has textures available for rendering
    smBool isMeshTextured();

    /// \brief assign the texture
    void assignTexture(smInt p_textureId);

    /// \brief assign the texture
    void assignTexture(const smString& p_referenceName);

    /// \brief update the original texture vertices with the current
    void updateOriginalVertsWithCurrent();

public:
    smCollisionGroup collisionGroup; ///< !!
    smGLInt renderingID; ///< !!
    std::shared_ptr<smErrorLog> log; ///< record the log
    smStdVector3d vertices; ///< vertices co-ordinate data at time t
    smStdVector3d origVerts; ///< vertices co-ordinate data at time t=0
    smInt  nbrVertices; ///< number of vertices
    smAABB aabb; ///< Axis aligned bounding box
    smBool isTextureCoordAvailable; ///< true if the texture co-ordinate is available
    smTexCoord *texCoord; ///< texture co-ordinates
    std::vector<smTextureAttachment> textureIds; ///< !!
};

/// \brief: this is a generic Mesh class from which surface and volume meshes are inherited
/// Note: this class cannot exist on its own
class smMesh: public smBaseMesh
{
public:
    /// \brief constructor
    smMesh();

    /// \brief destructor
    virtual ~smMesh();

    /// \brief compute the neighbors of the vertex
    void getVertexNeighbors();

    /// \brief compute the neighbors of the triangle
    void getTriangleNeighbors();

    /// \brief initialize vertex arrays
    smBool initVertexArrays(smInt nbr);

    /// \brief initialize triangle arrays
    smBool initTriangleArrays(smInt nbr);

    /// \brief initialize the neighbors of the vertex
    void initVertexNeighbors();

    /// \brief !!
    void allocateAABBTris();

    /// \brief compute the normal of a triangle
    smVec3d calculateTriangleNormal(smInt triNbr);

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
    void calculateTangent(smVec3d& p1, smVec3d& p2, smVec3d& p3, smTexCoord& t1, smTexCoord& t2, smTexCoord& t3, smVec3d& t);

    /// \brief !!
    void calculateTangent_test(smVec3d& p1, smVec3d& p2, smVec3d& p3, smTexCoord& t1, smTexCoord& t2, smTexCoord& t3, smVec3d& t);

    /// \brief find the neighbors of all vertices of mesh
    void calcNeighborsVertices();

    /// \brief find all the edges of the mesh
    void calcEdges();

    /// \brief translate the mesh
    void translate(smFloat, smFloat, smFloat);

    /// \brief translate the mesh
    void translate(smVec3d p_offset);

    /// \brief scale the mesh
    void scale(smVec3d p_scaleFactors);

    /// \brief rotate the mesh
    void rotate(const smMatrix33d &p_rot);

    /// \brief check if there is a consistent orientation of triangle vertices
    /// across the entire surface mesh
    void checkCorrectWinding();

    /// \brief get the type fo mesh
    smMeshType getMeshType()
    {
        return meshType;
    };

    /// \brief load the mesh
    virtual smBool loadMesh(const smString& fileName, const smMeshFileType &fileType) = 0;

    /// \brief render the surface mesh
    virtual void draw(const smDrawParam &p_params) override;

public:
    smInt  nbrTriangles; ///< number of triangles
    smTriangle *triangles; ///< list of triangles
    smTexCoord *texCoordForTrianglesOBJ; ///< !! tansel for OBJ
    int nbrTexCoordForTrainglesOBJ; ///< !! tansel for OBJ
    smVec3d *triNormals; ///< triangle normals
    smVec3d *vertNormals; ///< vertex normals
    smVec3d *triTangents; ///< triangle tangents
    smVec3d *vertTangents; ///< vertex tangents
    smBool tangentChannel; ///< !!
    std::vector< std::vector<smInt> > vertTriNeighbors; ///< list of neighbors for a triangle
    std::vector< std::vector<smInt> > vertVertNeighbors; ///< list of neighbors for a vertex
    std::vector<smEdge> edges; ///< list of edges

    ///AABBB of the mesh.
    ///This value is allocated and computed by only collision detection module
    ///Therefore it is initially NULL
    smAABB *triAABBs;

    smMeshType meshType; ///< type of mesh (rigid, deformable etc.)
    smMeshFileType meshFileType; ///< type of input mesh
};

/// \brief holds the texture co-ordinates
struct smTexCoord
{
    smFloat u, v;
};

/// \brief holds the vertex indices of triangle
struct smTriangle
{
    smUInt vert[3];
};

/// \brief holds the vertex indices of tetrahedron
struct smTetrahedra
{
    smInt vert[4];
};

/// \brief holds the vertex indices of edge
struct smEdge
{
    smUInt vert[2];
};

/// \brief !!
class smLineMesh: public smBaseMesh
{
public:
    /// \brief destructor
    ~smLineMesh()
    {
        delete[]edgeAABBs;
        delete[]texCoord;
        delete[]edges;
    }

    /// \brief constructor
    smLineMesh(smInt p_nbrVertices);

    /// \brief constructor
    smLineMesh(smInt p_nbrVertices, smBool autoEdge);

    /// \brief !!
    void createAutoEdges();

    /// \brief !!
    virtual void createCustomEdges() {};

    /// \brief updat AABB when the mesh moves
    void updateAABB();

    /// \brief translate the vertices of mesh
    void translate(smFloat p_offsetX, smFloat p_offsetY, smFloat p_offsetZ);

    /// \brief translate the vertices of mesh
    void translate(smVec3d p_offset);

    /// \brief scale the mesh
    void scale(smVec3d p_scaleFactors);

    /// \brief rotate the mesh
    void rotate(smMatrix33d p_rot);

    /// \brief query if the mesh is textured
    smBool isMeshTextured();

    /// \brief draw the mesh
    void draw(const smDrawParam &p_params) override;

public:
    smAABB *edgeAABBs;///< AABBs for the edges in the mesh
    smEdge *edges;///< edges of the line mesh
    smInt nbrEdges;///< number of edges of the line mesh

};

#endif
