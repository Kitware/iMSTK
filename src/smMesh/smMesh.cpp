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

#include "smMesh/smMesh.h"
// #include "smCore/smSDK.h"
#include "smRendering/smGLRenderer.h"
#include "smRendering/smViewer.h"
#include "smCore/smFactory.h"

smBaseMesh::smBaseMesh()
{
//     smSDK::getInstance()->registerMesh(safeDownCast<smBaseMesh>());
}

void smBaseMesh::updateOriginalVertsWithCurrent()
{
    origVerts = vertices;
}

/// \brief constructor
smMesh::smMesh()
{
    triangles = 0;
    texCoord = 0;
    triNormals = 0;
    vertNormals = 0;
    triTangents = 0;
    vertTangents = 0;
    type = SIMMEDTK_SMMESH;
    isTextureCoordAvailable = false;
    tangentChannel = false;
    this->setRenderDelegate(
      smFactory<smRenderDelegate>::createConcreteClass(
        "smMeshRenderDelegate"));
}

/// \brief destructor
smMesh::~smMesh()
{
    delete [] triangles;
    delete [] texCoord;
    delete [] triNormals;
    delete [] vertNormals;
    delete [] triTangents;
    delete [] vertTangents;

}

/// \brief
void smMesh::allocateAABBTris()
{
    this->triAABBs.resize(nbrTriangles);
    this->updateTriangleAABB();
}

/// \brief
void CalculateTangentArray(smInt vertexCount, const smVec3d *vertex,
                           const smVec3d *normal, const smTexCoord *texcoord,
                           long triangleCount, const smTriangle *triangle,
                           smVec3d *tangent)
{

    smVec3d *tan1 = new smVec3d[vertexCount * 2];
    smVec3d *tan2 = tan1 + vertexCount;
    memset(tan1, 0, vertexCount * sizeof(smVec3d) * 2);

    for (long a = 0; a < triangleCount; a++)
    {
        long i1 = triangle->vert[0];
        long i2 = triangle->vert[1];
        long i3 = triangle->vert[2];

        const smVec3d& v1 = vertex[i1];
        const smVec3d& v2 = vertex[i2];
        const smVec3d& v3 = vertex[i3];

        const smTexCoord& w1 = texcoord[i1];
        const smTexCoord& w2 = texcoord[i2];
        const smTexCoord& w3 = texcoord[i3];

        float x1 = v2[0] - v1[0];
        float x2 = v3[0] - v1[0];
        float y1 = v2[1] - v1[1];
        float y2 = v3[1] - v1[1];
        float z1 = v2[2] - v1[2];
        float z2 = v3[2] - v1[2];

        float s1 = w2.u - w1.u;
        float s2 = w3.u - w1.u;
        float t1 = w2.v - w1.v;
        float t2 = w3.v - w1.v;

        float r = 1.0F / (s1 * t2 - s2 * t1);
        smVec3d sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                     (t2 * z1 - t1 * z2) * r);
        smVec3d tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                     (s1 * z2 - s2 * z1) * r);

        tan1[i1] += sdir;
        tan1[i2] += sdir;
        tan1[i3] += sdir;

        tan2[i1] += tdir;
        tan2[i2] += tdir;
        tan2[i3] += tdir;

        triangle++;
    }

    for (long a = 0; a < vertexCount; a++)
    {
        smVec3d n = normal[a];
        smVec3d t = tan1[a];
        tangent[a] = (t - n * n.dot(t));
        tangent[a].normalize();
    }

    delete[] tan1;
}

/// \brief calucate the triangle tangents
void smMesh::calcTriangleTangents()
{

    smInt t;

    // First calculate the triangle tangents
    for (t = 0; t < nbrTriangles; t++)
    {
        smTriangle *tmpTri = &triangles[t];
        smVec3d *v0 = &vertices[tmpTri->vert[0]];
        smVec3d *v1 = &vertices[tmpTri->vert[1]];
        smVec3d *v2 = &vertices[tmpTri->vert[2]];
        smTexCoord *t0 = &texCoord[tmpTri->vert[0]];
        smTexCoord *t1 = &texCoord[tmpTri->vert[1]];
        smTexCoord *t2 = &texCoord[tmpTri->vert[2]];

        if (this->meshFileType == SM_FILETYPE_3DS)
        {
            calculateTangent(*v2, *v1, *v0, *t2, *t1, *t0, triTangents[t]);
        }
        else if (this->meshFileType == SM_FILETYPE_OBJ)
        {
            calculateTangent_test(*v0, *v1, *v2, *t0, *t1, *t2, triTangents[t]);
        }
    }

    //calculate the vertex normals
    if (this->meshFileType == SM_FILETYPE_3DS || this->meshFileType == SM_FILETYPE_OBJ)
    {
        for (smInt v = 0; v < nbrVertices; v++)
        {
            vertTangents[v][0] = vertTangents[v][1] = vertTangents[v][2] = 0;

            for (size_t i = 0; i < vertTriNeighbors[v].size(); i++)
            {
                vertTangents[v] += triTangents[vertTriNeighbors[v][i]];
            }

            vertTangents[v].normalized();
            vertTangents[v] = (vertTangents[v] - vertNormals[v] * vertNormals[v].dot(vertTangents[v]));
            vertTangents[v].normalized();
        }
    }
}

/// \brief calucate the triangle tangent for rendering purposes
void smMesh::calculateTangent(smVec3d& p1, smVec3d& p2, smVec3d& p3, smTexCoord& t1, smTexCoord& t2, smTexCoord& t3, smVec3d& t)
{

    smVec3d v1;
    smVec3d v2;

    v1[0] = p2[0] - p1[0];
    v1[1] = p2[1] - p1[1];
    v1[2] = p2[2] - p1[2];

    v2[0] = p3[0] - p1[0];
    v2[1] = p3[1] - p1[1];
    v2[2] = p3[2] - p1[2];

    smFloat bb1 = t2.v - t1.v;
    smFloat bb2 = t3.v - t1.v;

    t[0] = bb2 * v1[0] - bb1 * v2[0];
    t[1] = bb2 * v1[1] - bb1 * v2[1];
    t[2] = bb2 * v1[2] - bb1 * v2[2];

    t.normalize();
}

/// \brief
void smMesh::calculateTangent_test(smVec3d& p1, smVec3d& p2, smVec3d& p3, smTexCoord& t1, smTexCoord& t2, smTexCoord& t3, smVec3d& t)
{

    smVec3d v1;
    smVec3d v2;

    v1[0] = p2[0] - p1[0];
    v1[1] = p2[1] - p1[1];
    v1[2] = p2[2] - p1[2];

    v2[0] = p3[0] - p1[0];
    v2[1] = p3[1] - p1[1];
    v2[2] = p3[2] - p1[2];

    smFloat tt1 = t2.u - t1.u;
    smFloat tt2 = t3.u - t1.u;

    smFloat bb1 = t2.v - t1.v;
    smFloat bb2 = t3.v - t1.v;
    float r = 1.0F / (tt1 * bb2 - tt2 * bb1);
    t[0] = (bb2 * v1[0] - bb1 * v2[0]) * r;
    t[1] = (bb2 * v1[1] - bb1 * v2[1]) * r;
    t[2] = (bb2 * v1[2] - bb1 * v2[2]) * r;
}

/// \brief calculates the normal of the vertex
void smMesh::updateVertexNormals()
{
    smVec3d temp = smVec3d::Zero();

    for (smInt i = 0; i < nbrVertices; i++)
    {
        for (size_t j = 0; j < vertTriNeighbors[i].size(); j++)
        {
            temp += triNormals[vertTriNeighbors[i][j]];
        }

        vertNormals[i] = temp;
        vertNormals[i].normalized();
        temp = smVec3d::Zero();
    }
}

/// \brief updates the normal of all the triangle
void smMesh::updateTriangleNormals()
{

    for (smInt i = 0; i < nbrTriangles; i++)
    {
        triNormals[i] = calculateTriangleNormal(i).normalized();
    }
}

/// \brief calculates the normal of a triangle
smVec3d smMesh::calculateTriangleNormal(smInt triNbr)
{

    smVec3d v[3];
    smTriangle temp = this->triangles[triNbr];

    v[0] = this->vertices[temp.vert[0]];
    v[1] = this->vertices[temp.vert[1]];
    v[2] = this->vertices[temp.vert[2]];

    return (v[1] - v[0]).cross(v[2] - v[0]).normalized();
}

/// \brief allocates vertices and related array
smBool smMesh::initVertexArrays(smInt nbr)
{

    if (nbr < 0)
    {
        return false;
    }

    this->nbrVertices = nbr;
    this->vertices.resize(nbr);
    this->origVerts.resize(nbr);
    this->vertNormals = new smVec3d[nbr];
    this->vertTangents = new smVec3d[nbr];
    this->texCoord = new smTexCoord[nbr];
    return true;
}

/// \brief allocates triangle and related array
smBool smMesh::initTriangleArrays(smInt nbr)
{

    if (nbr < 0)
    {
        return false;
    }

    this->nbrTriangles = nbr;

    this->triangles = new smTriangle[nbr];
    this->triNormals = new smVec3d[nbr];
    this->triTangents = new smVec3d[nbr];
    return true;
}

/// \brief initializes the vertex neighbors
void smMesh::initVertexNeighbors()
{

    smInt i;
    vertTriNeighbors.resize(nbrVertices);

    for (i = 0; i < nbrTriangles; i++)
    {
        vertTriNeighbors[triangles[i].vert[0]].push_back(i);
        vertTriNeighbors[triangles[i].vert[1]].push_back(i);
        vertTriNeighbors[triangles[i].vert[2]].push_back(i);
    }
}

/// \brief initializes the vertex neighbors
void smMesh::calcNeighborsVertices()
{

    smInt i;
    smInt triangleIndex;
    smInt candidate[3];

    vertVertNeighbors.resize(nbrVertices);

    for (i = 0; i < nbrVertices; i++)
    {
        for (size_t j = 0; j < vertTriNeighbors[i].size(); j++)
        {
            triangleIndex = vertTriNeighbors[i][j];
            candidate[0] = triangles[triangleIndex].vert[0];
            candidate[1] = triangles[triangleIndex].vert[1];
            candidate[2] = triangles[triangleIndex].vert[2];

            if (candidate[0] == i)
            {
                candidate[0] = -1;
            }

            if (candidate[1] == i)
            {
                candidate[1] = -1;
            }

            if (candidate[2] == i)
            {
                candidate[2] = -1;
            }

            for (size_t k = 0; k < vertVertNeighbors[i].size(); k++)
            {
                if (vertVertNeighbors[i][k] == candidate[0])
                {
                    candidate[0] = -1;
                }

                if (vertVertNeighbors[i][k] == candidate[1])
                {
                    candidate[1] = -1;
                }

                if (vertVertNeighbors[i][k] == candidate[2])
                {
                    candidate[2] = -1;
                }
            }

            if (candidate[0] != -1)
            {
                vertVertNeighbors[i].push_back(candidate[0]);
            }

            if (candidate[1] != -1)
            {
                vertVertNeighbors[i].push_back(candidate[1]);
            }

            if (candidate[2] != -1)
            {
                vertVertNeighbors[i].push_back(candidate[2]);
            }

        }
    }
}

/// \brief
void smMesh::upadateAABB()
{
    smFloat minx = smMAXFLOAT ;
    smFloat miny = smMAXFLOAT;
    smFloat minz = smMAXFLOAT;
    smFloat maxx = -smMAXFLOAT;
    smFloat maxy = -smMAXFLOAT;
    smFloat maxz = -smMAXFLOAT;

    for (smInt i = 0; i < nbrVertices; i++)
    {
        minx = SIMMEDTK_MIN(vertices[i][0], minx);
        miny = SIMMEDTK_MIN(vertices[i][1], miny);
        minz = SIMMEDTK_MIN(vertices[i][2], minz);

        maxx = SIMMEDTK_MAX(vertices[i][0], maxx);
        maxy = SIMMEDTK_MAX(vertices[i][1], maxy);
        maxz = SIMMEDTK_MAX(vertices[i][2], maxz);
    }

    aabb.aabbMin[0] = minx - (maxx - minx) * SIMMEDTK_MESH_AABBSKINFACTOR;
    aabb.aabbMin[1] = miny - (maxy - miny) * SIMMEDTK_MESH_AABBSKINFACTOR;
    aabb.aabbMin[2] = minz - (maxz - minz) * SIMMEDTK_MESH_AABBSKINFACTOR;

    aabb.aabbMax[0] = maxx + (maxx - minx) * SIMMEDTK_MESH_AABBSKINFACTOR;
    aabb.aabbMax[1] = maxy + (maxy - miny) * SIMMEDTK_MESH_AABBSKINFACTOR;
    aabb.aabbMax[2] = maxz + (maxz - minz) * SIMMEDTK_MESH_AABBSKINFACTOR;
}

/// \brief
void smMesh::calcEdges()
{
    smEdge edge;
    edges.reserve(SIMMEDTK_MESH_RESERVEDMAXEDGES);

    for (smInt i = 0; i < nbrVertices; i++)
    {
        for (size_t j = 0; j < vertVertNeighbors[i].size(); j++)
        {
            if (vertVertNeighbors[i][j] > i)
            {
                edge.vert[0] = i;
                edge.vert[1] = vertVertNeighbors[i][j];
                edges.push_back(edge);
            }
        }
    }
}

/// \brief
void smMesh::translate(smFloat p_offsetX, smFloat p_offsetY, smFloat p_offsetZ)
{

    for (smInt i = 0; i < nbrVertices; i++)
    {
        vertices[i][0] = vertices[i][0] + p_offsetX;
        vertices[i][1] = vertices[i][1] + p_offsetY;
        vertices[i][2] = vertices[i][2] + p_offsetZ;

        origVerts[i][0] = vertices[i][0] + p_offsetX;
        origVerts[i][1] = vertices[i][1] + p_offsetY;
        origVerts[i][2] = vertices[i][2] + p_offsetZ;
    }

    upadateAABB();
}

/// \brief
void smMesh::translate(smVec3d p_offset)
{

    for (smInt i = 0; i < nbrVertices; i++)
    {
        vertices[i] = vertices[i] + p_offset;
        origVerts[i] = origVerts[i] + p_offset;
    }

    upadateAABB();
}

/// \brief
void smMesh::scale(smVec3d p_scaleFactors)
{

    for (smInt i = 0; i < nbrVertices; i++)
    {
        vertices[i][0] = vertices[i][0] * p_scaleFactors[0];
        vertices[i][1] = vertices[i][1] * p_scaleFactors[1];
        vertices[i][2] = vertices[i][2] * p_scaleFactors[2];

        origVerts[i][0] = origVerts[i][0] * p_scaleFactors[0];
        origVerts[i][1] = origVerts[i][1] * p_scaleFactors[1];
        origVerts[i][2] = origVerts[i][2] * p_scaleFactors[2];
    }

    upadateAABB();
}

/// \brief
void smMesh::rotate(const smMatrix33d &p_rot)
{

    for (smInt i = 0; i < nbrVertices; i++)
    {
        vertices[i] = p_rot * vertices[i];
        origVerts[i] = p_rot * origVerts[i];
        vertNormals[i] = p_rot * vertNormals[i];
    }

    for (smInt i = 0; i < nbrTriangles; i++)
    {
        triNormals[i] = p_rot * triNormals[i];
    }

    calcTriangleTangents();
    upadateAABB();
}

/// \brief
void smMesh::updateTriangleAABB()
{
    smAABB tempAABB;
    tempAABB.aabbMin[0] = FLT_MAX;
    tempAABB.aabbMin[1] = FLT_MAX;
    tempAABB.aabbMin[2] = FLT_MAX;

    tempAABB.aabbMax[0] = -FLT_MAX;
    tempAABB.aabbMax[1] = -FLT_MAX;
    tempAABB.aabbMax[2] = -FLT_MAX;

    for (smInt i = 0; i < nbrTriangles; i++)
    {
        // min
        triAABBs[i].aabbMin[0] = SIMMEDTK_MIN(vertices[triangles[i].vert[0]][0], vertices[triangles[i].vert[1]][0]);
        triAABBs[i].aabbMin[0] = SIMMEDTK_MIN(triAABBs[i].aabbMin[0] ,   vertices[triangles[i].vert[2]][0]);
        tempAABB.aabbMin[0] = SIMMEDTK_MIN(tempAABB.aabbMin[0], triAABBs[i].aabbMin[0]);

        triAABBs[i].aabbMin[1] = SIMMEDTK_MIN(vertices[triangles[i].vert[0]][1], vertices[triangles[i].vert[1]][1]);
        triAABBs[i].aabbMin[1] = SIMMEDTK_MIN(triAABBs[i].aabbMin[1] ,   vertices[triangles[i].vert[2]][1]);
        tempAABB.aabbMin[1] = SIMMEDTK_MIN(tempAABB.aabbMin[1], triAABBs[i].aabbMin[1]);

        triAABBs[i].aabbMin[2] = SIMMEDTK_MIN(vertices[triangles[i].vert[0]][2], vertices[triangles[i].vert[1]][2]);
        triAABBs[i].aabbMin[2] = SIMMEDTK_MIN(triAABBs[i].aabbMin[2] ,   vertices[triangles[i].vert[2]][2]);
        tempAABB.aabbMin[2] = SIMMEDTK_MIN(tempAABB.aabbMin[2], triAABBs[i].aabbMin[2]);

        //max
        triAABBs[i].aabbMax[0] = SIMMEDTK_MAX(vertices[triangles[i].vert[0]][0], vertices[triangles[i].vert[1]][0]);
        triAABBs[i].aabbMax[0] = SIMMEDTK_MAX(triAABBs[i].aabbMax[0] ,   vertices[triangles[i].vert[2]][0]);
        tempAABB.aabbMax[0] = SIMMEDTK_MAX(tempAABB.aabbMax[0], triAABBs[i].aabbMax[0]);

        triAABBs[i].aabbMax[1] = SIMMEDTK_MAX(vertices[triangles[i].vert[0]][1], vertices[triangles[i].vert[1]][1]);
        triAABBs[i].aabbMax[1] = SIMMEDTK_MAX(triAABBs[i].aabbMax[1] ,   vertices[triangles[i].vert[2]][1]);
        tempAABB.aabbMax[1] = SIMMEDTK_MAX(tempAABB.aabbMax[1], triAABBs[i].aabbMax[1]);

        triAABBs[i].aabbMax[2] = SIMMEDTK_MAX(triAABBs[i].aabbMax[2],    vertices[triangles[i].vert[2]][2]);
        tempAABB.aabbMax[2] = SIMMEDTK_MAX(tempAABB.aabbMax[2], triAABBs[i].aabbMax[2]);
    }

    aabb = tempAABB;
}

/// \brief
void smMesh::checkCorrectWinding()
{
    smInt x[3];
    smInt p[3];

    for (smInt i = 0; i < nbrTriangles; i++)
    {
        x[0] = triangles[i].vert[0];
        x[1] = triangles[i].vert[1];
        x[2] = triangles[i].vert[2];

        for (smInt j = 0; j < nbrTriangles; j++)
        {
            if (j == i)
            {
                continue;
            }

            p[0] = triangles[j].vert[0];
            p[1] = triangles[j].vert[1];
            p[2] = triangles[j].vert[2];

            if (x[0] == p[0] && x[1] == p[1])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[0] == p[1] && x[1] == p[2])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[0] == p[2] && x[1] == p[0])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[1] == p[0] && x[2] == p[1])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[1] == p[1] && x[2] == p[2])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[1] == p[2] && x[2] == p[0])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[2] == p[0] && x[0] == p[1])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[2] == p[1] && x[0] == p[2])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }

            if (x[2] == p[2] && x[0] == p[0])
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }
        }
    }
}

smTextureAttachment::smTextureAttachment()
{
}

bool smBaseMesh::isMeshTextured()
{
    return isTextureCoordAvailable;
}

void smBaseMesh::assignTexture( int p_textureId )
{
    smTextureAttachment attachment;
    attachment.textureId = p_textureId;

    if ( p_textureId > 0 )
    {
        this->textureIds.push_back( attachment );
    }
}
void smBaseMesh::assignTexture(const smString& p_referenceName)
{
    smInt textureId;
    smTextureAttachment attachment;

    if (smTextureManager::findTextureId(p_referenceName, textureId) == SIMMEDTK_TEXTURE_OK)
    {
        attachment.textureId = textureId;
        this->textureIds.push_back(attachment);
    }
}
smLineMesh::smLineMesh( int p_nbrVertices ) : smBaseMesh()
{
    nbrVertices = p_nbrVertices;
    vertices.reserve( nbrVertices );
    origVerts.reserve( nbrVertices );
    edgeAABBs = new smAABB[nbrVertices - 1];
    texCoord = new smTexCoord[nbrVertices];
    edges = new smEdge[nbrVertices - 1];
    nbrEdges = nbrVertices - 1;
    isTextureCoordAvailable = false;
    createAutoEdges();
}
smLineMesh::smLineMesh( int p_nbrVertices, bool autoEdge ) : smBaseMesh()
{
    nbrVertices = p_nbrVertices;
    vertices.reserve( nbrVertices );
    origVerts.reserve( nbrVertices );
    texCoord = new smTexCoord[nbrVertices];

    /// Edge AABB should be assigned by the instance
    edgeAABBs = NULL;

    /// Edges should be assigned by the instance
    edges = NULL;

    /// Number of edges should be assigned by the instance
    nbrEdges = 0;

    isTextureCoordAvailable = false;

    if ( autoEdge )
    {
        createAutoEdges();
    }
}
void smLineMesh::createAutoEdges()
{
    for ( smInt i = 0; i < nbrEdges; i++ )
    {
        edges[i].vert[0] = i;
        edges[i].vert[1] = i + 1;
    }
}
void smLineMesh::updateAABB()
{
    smAABB tempAABB;
    smVec3d minOffset( -2.0, -2.0, -2.0 );
    smVec3d maxOffset( 1.0, 1.0, 1.0 );
    smVec3d minEdgeOffset( -0.1, -0.1, -0.1 );
    smVec3d maxEdgeOffset( 0.1, 0.1, 0.1 );

    tempAABB.aabbMin[0] = FLT_MAX;
    tempAABB.aabbMin[1] = FLT_MAX;
    tempAABB.aabbMin[2] = FLT_MAX;

    tempAABB.aabbMax[0] = -FLT_MAX;
    tempAABB.aabbMax[1] = -FLT_MAX;
    tempAABB.aabbMax[2] = -FLT_MAX;

    for ( smInt i = 0; i < nbrEdges; i++ )
    {
        ///min
        edgeAABBs[i].aabbMin[0] = SIMMEDTK_MIN( vertices[edges[i].vert[0]][0], vertices[edges[i].vert[1]][0] );
        edgeAABBs[i].aabbMin[1] = SIMMEDTK_MIN( vertices[edges[i].vert[0]][1], vertices[edges[i].vert[1]][1] );
        edgeAABBs[i].aabbMin[2] = SIMMEDTK_MIN( vertices[edges[i].vert[0]][2], vertices[edges[i].vert[1]][2] );
        edgeAABBs[i].aabbMin += minEdgeOffset;
        tempAABB.aabbMin[0] = SIMMEDTK_MIN( tempAABB.aabbMin[0], edgeAABBs[i].aabbMin[0] );
        tempAABB.aabbMin[1] = SIMMEDTK_MIN( tempAABB.aabbMin[1], edgeAABBs[i].aabbMin[1] );
        tempAABB.aabbMin[2] = SIMMEDTK_MIN( tempAABB.aabbMin[2], edgeAABBs[i].aabbMin[2] );

        ///max
        edgeAABBs[i].aabbMax[0] = SIMMEDTK_MAX( vertices[edges[i].vert[0]][0], vertices[edges[i].vert[1]][0] );
        edgeAABBs[i].aabbMax[1] = SIMMEDTK_MAX( vertices[edges[i].vert[0]][1], vertices[edges[i].vert[1]][1] );
        edgeAABBs[i].aabbMax[2] = SIMMEDTK_MAX( vertices[edges[i].vert[0]][2], vertices[edges[i].vert[1]][2] );
        edgeAABBs[i].aabbMax += maxEdgeOffset;
        tempAABB.aabbMax[0] = SIMMEDTK_MAX( tempAABB.aabbMax[0], edgeAABBs[i].aabbMax[0] );
        tempAABB.aabbMax[1] = SIMMEDTK_MAX( tempAABB.aabbMax[1], edgeAABBs[i].aabbMax[1] );
        tempAABB.aabbMax[2] = SIMMEDTK_MAX( tempAABB.aabbMax[2], edgeAABBs[i].aabbMax[2] );
    }

    tempAABB.aabbMin += minOffset;
    tempAABB.aabbMax += maxOffset;
    aabb = tempAABB;
}
void smLineMesh::translate( float p_offsetX, float p_offsetY, float p_offsetZ )
{

    for ( smInt i = 0; i < nbrVertices; i++ )
    {
        vertices[i][0] = vertices[i][0] + p_offsetX;
        vertices[i][1] = vertices[i][1] + p_offsetY;
        vertices[i][2] = vertices[i][2] + p_offsetZ;
    }

    updateAABB();
}
void smLineMesh::translate( smVec3d p_offset )
{

    for ( smInt i = 0; i < nbrVertices; i++ )
    {
        vertices[i] = vertices[i] + p_offset;
        origVerts[i] = origVerts[i] + p_offset;
    }

    updateAABB();
}
void smLineMesh::rotate( smMatrix33d p_rot )
{

    for ( smInt i = 0; i < nbrVertices; i++ )
    {
        vertices[i] = p_rot * vertices[i];
        origVerts[i] = p_rot * origVerts[i];
    }

    updateAABB();
}
void smLineMesh::scale( smVec3d p_scaleFactors )
{

    for ( smInt i = 0; i < nbrVertices; i++ )
    {
        vertices[i][0] = vertices[i][0] * p_scaleFactors[0];
        vertices[i][1] = vertices[i][1] * p_scaleFactors[1];
        vertices[i][2] = vertices[i][2] * p_scaleFactors[2];

        origVerts[i][0] = origVerts[i][0] * p_scaleFactors[0];
        origVerts[i][1] = origVerts[i][1] * p_scaleFactors[1];
        origVerts[i][2] = origVerts[i][2] * p_scaleFactors[2];
    }

    updateAABB();
}

bool smLineMesh::isMeshTextured()
{
    return isTextureCoordAvailable;
}

int smMesh::getNumTriangles() const
{
    return this->nbrTriangles;
}

int smMesh::getNumEdges() const
{
    return this->edges.size();
}

void smMesh::updateSurfaceMeshFromVegaFormat(std::shared_ptr<ObjMesh> vegaSurfaceMesh)
{
    Vec3d p;
    //copy the vertex co-ordinates
    for(smInt i=0; i<this->nbrVertices ; i++)
    {
       p = vegaSurfaceMesh->getPosition(i);
       this->vertices[i][0] = p[0];
       this->vertices[i][1] = p[1];
       this->vertices[i][2] = p[2];
    }
}

bool smMesh::importSurfaceMeshFromVegaFormat(std::shared_ptr<ObjMesh> vegaSurfaceMesh, const bool perProcessingStage)
{

    if(!vegaSurfaceMesh->isTriangularMesh())
    {
        if (this->log != nullptr)
        {
            this->log->addError("Error : SimMedTK supports only triangular surface mesh. Vega mesh is not a triangle mesh!");
            return 0;
        }
    }

    int i, threeI;

    // temporary arrays
    int numVertices(0);
    double* vertices;
    int numTriangles(0);
    int* triangles;
    //smInt * numGroups;
	//smInt ** triangleGroups;

    vertices = nullptr;
    triangles = nullptr;
    vegaSurfaceMesh->exportGeometry(&numVertices, &vertices, &numTriangles , &triangles, nullptr, nullptr);

    this->nbrVertices = numVertices;
    this->nbrTriangles = numTriangles;

    initVertexArrays(numVertices);
    initTriangleArrays(numTriangles);

    /*delete this->triangles;
    this->triangles = new smTriangle[this->nbrTriangles];*/

    //copy the triangle connectivity information
    for(i=0; i<this->nbrTriangles ; i++)
    {
        threeI = 3*i;
        this->triangles[i].vert[0] = triangles[threeI+0];
        this->triangles[i].vert[1] = triangles[threeI+1];
        this->triangles[i].vert[2] = triangles[threeI+2];
    }

    //this->vertices.resize(this->nbrVertices);
    //copy the vertex co-ordinates
    for(i=0; i<this->nbrVertices ; i++)
    {
        this->vertices[i][0] = vertices[3 * i + 0];
        this->vertices[i][1] = vertices[3 * i + 1];
        this->vertices[i][2] = vertices[3 * i + 2];
    }

    if(perProcessingStage){
        updateOriginalVertsWithCurrent();
    }

    //deallocate temporary arrays
    delete [] triangles;
    delete [] vertices;

    return 1;

}
