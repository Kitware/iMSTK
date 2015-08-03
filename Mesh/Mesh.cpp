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

#include "Mesh/Mesh.h"

#include <limits>

// VEgaFEM include
#include "objMesh.h"

// SimMedTK includes
#include "Core/Factory.h"
#include "Rendering/OpenGLRenderer.h"
#include "Rendering/TextureManager.h"
#include "Rendering/OpenGLViewer.h"

#include "VtkRendering/VtkRenderDelegate.h"

/// \brief constructor
Mesh::Mesh()
{
    triNormals = 0;
    vertNormals = 0;
    triTangents = 0;
    vertTangents = 0;
    nbrTriangles = 0;
    nbrTexCoordForTrainglesOBJ = 0;
    type = core::ClassType::Mesh;
    tangentChannel = false;
    auto delegate = Factory<VtkRenderDelegate>::createConcreteClass(
        "MeshRenderDelegate");
    this->setRenderDelegate(delegate);
}

/// \brief destructor
Mesh::~Mesh()
{
    delete [] triNormals;
    delete [] vertNormals;
    delete [] triTangents;
    delete [] vertTangents;

}

/// \brief
void Mesh::allocateAABBTris()
{
    this->triAABBs.resize(nbrTriangles);
    this->updateTriangleAABB();
}

/// \brief
void CalculateTangentArray(int vertexCount, const core::Vec3d *vertex,
                           const core::Vec3d *normal, const TexCoord *texcoord,
                           long triangleCount, const Triangle *triangle,
                           core::Vec3d *tangent)
{

    core::Vec3d *tan1 = new core::Vec3d[vertexCount * 2];
    core::Vec3d *tan2 = tan1 + vertexCount;
    memset(tan1, 0, vertexCount * sizeof(core::Vec3d) * 2);

    for (long a = 0; a < triangleCount; a++)
    {
        long i1 = triangle->vert[0];
        long i2 = triangle->vert[1];
        long i3 = triangle->vert[2];

        const core::Vec3d& v1 = vertex[i1];
        const core::Vec3d& v2 = vertex[i2];
        const core::Vec3d& v3 = vertex[i3];

        const TexCoord& w1 = texcoord[i1];
        const TexCoord& w2 = texcoord[i2];
        const TexCoord& w3 = texcoord[i3];

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
        core::Vec3d sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                     (t2 * z1 - t1 * z2) * r);
        core::Vec3d tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
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
        core::Vec3d n = normal[a];
        core::Vec3d t = tan1[a];
        tangent[a] = (t - n * n.dot(t));
        tangent[a].normalize();
    }

    delete[] tan1;
}

/// \brief calucate the triangle tangents
void Mesh::calcTriangleTangents()
{
    int t;

    // First calculate the triangle tangents
    auto vertexArray = this->getVertices();
    auto texCoordArray = this->getTextureCoordinates();
    for (t = 0; t < nbrTriangles; t++)
    {
        auto &v0 = vertexArray[triangles[t].vert[0]];
        auto &v1 = vertexArray[triangles[t].vert[1]];
        auto &v2 = vertexArray[triangles[t].vert[2]];
        auto &t0 = texCoordArray[triangles[t].vert[0]];
        auto &t1 = texCoordArray[triangles[t].vert[1]];
        auto &t2 = texCoordArray[triangles[t].vert[2]];

        if (this->meshFileType == BaseMesh::MeshFileType::ThreeDS)
        {
            triTangents[t] = calculateTangent(v2, v1, v0, t2, t1, t0);
        }
        else if (this->meshFileType == BaseMesh::MeshFileType::Obj)
        {
            triTangents[t] = calculateTangent_test(v0, v1, v2, t0, t1, t2);
        }
    }

    //calculate the vertex normals
    if (this->meshFileType == BaseMesh::MeshFileType::ThreeDS || this->meshFileType == BaseMesh::MeshFileType::Obj)
    {
        for (int v = 0, end = vertexArray.size(); v < end; ++v)
        {
            vertTangents[v][0] = vertTangents[v][1] = vertTangents[v][2] = 0;

            for (size_t i = 0; i < vertTriNeighbors[v].size(); i++)
            {
                vertTangents[v] += triTangents[vertTriNeighbors[v][i]];
            }

            vertTangents[v].normalize();
            vertTangents[v] = (vertTangents[v] - vertNormals[v] * vertNormals[v].dot(vertTangents[v]));
            vertTangents[v].normalize();
        }
    }
}

/// \brief calucate the triangle tangent for rendering purposes
core::Vec3d Mesh::calculateTangent(const core::Vec3d& p1,
                                   const core::Vec3d& p2,
                                   const core::Vec3d& p3,
                                   const core::Vec2f& t1,
                                   const core::Vec2f& t2,
                                   const core::Vec2f& t3)
{
    core::Vec3d v1 = p2-p1;
    core::Vec3d v2 = p3-p1;

    float bb1 = t2(1) - t1(1);
    float bb2 = t3(1) - t1(1);

    core::Vec3d tangent = bb2 * v1 - bb1 * v2;

    return tangent.normalized();
}

/// \brief
core::Vec3d Mesh::calculateTangent_test(const core::Vec3d& p1,
                                        const core::Vec3d& p2,
                                        const core::Vec3d& p3,
                                        const core::Vec2f& t1,
                                        const core::Vec2f& t2,
                                        const core::Vec2f& t3)
{
    core::Vec3d v1 = p2-p1;
    core::Vec3d v2 = p3-p1;

    float tt1 = t2(0)-t1(0);
    float tt2 = t3(0)-t1(0);

    float bb1 = t2(1)-t1(1);
    float bb2 = t3(1)-t1(1);
    float r = 1.0f/(tt1 * bb2 - tt2 * bb1);

    core::Vec3d tangent = (bb2*v1 - bb1*v2)*r;

    return tangent.normalized();
}

/// \brief calculates the normal of the vertex
void Mesh::updateVertexNormals()
{
    for (size_t i = 0, end = this->getNumberOfVertices(); i < end; ++i)
    {
        core::Vec3d temp = core::Vec3d::Zero();
        for (size_t j = 0; j < vertTriNeighbors[i].size(); j++)
        {
            temp += triNormals[vertTriNeighbors[i][j]];
        }

        vertNormals[i] = temp;
        vertNormals[i].normalize();
    }
}

/// \brief updates the normal of all the triangle
void Mesh::updateTriangleNormals()
{

    for (int i = 0; i < nbrTriangles; i++)
    {
        triNormals[i] = calculateTriangleNormal(i).normalized();
    }
}

/// \brief calculates the normal of a triangle
core::Vec3d Mesh::calculateTriangleNormal(int triNbr)
{
    Triangle t = this->triangles[triNbr];

    const core::Vec3d &v = this->getVertex(t.vert[0]);

    return (this->getVertex(t.vert[1]) - v).cross(this->getVertex(t.vert[2]) - v).normalized();
}

/// \brief allocates vertices and related array
bool Mesh::initVertexArrays(int nbr)
{

    if (nbr < 0)
    {
        return false;
    }

    this->getVertices().resize(nbr);
    this->getOrigVertices().resize(nbr);
    this->getTextureCoordinates().resize(nbr);

    this->vertNormals = new core::Vec3d[nbr];
    this->vertTangents = new core::Vec3d[nbr];
    return true;
}

/// \brief allocates triangle and related array
bool Mesh::initTriangleArrays(int nbr)
{

    if (nbr < 0)
    {
        return false;
    }

    this->nbrTriangles = nbr;

    this->triangles.resize(nbr);
    this->triNormals = new core::Vec3d[nbr];
    this->triTangents = new core::Vec3d[nbr];
    return true;
}

/// \brief initializes the vertex neighbors
void Mesh::initVertexNeighbors()
{
    vertTriNeighbors.resize(this->getNumberOfVertices());

    for (int i = 0; i < nbrTriangles; ++i)
    {
        vertTriNeighbors[triangles[i].vert[0]].push_back(i);
        vertTriNeighbors[triangles[i].vert[1]].push_back(i);
        vertTriNeighbors[triangles[i].vert[2]].push_back(i);
    }
}

/// \brief initializes the vertex neighbors
void Mesh::calcNeighborsVertices()
{
    int candidate[3];

    vertVertNeighbors.resize(this->getNumberOfVertices());

    for (size_t i = 0, end = vertVertNeighbors.size(); i < end; ++i)
    {
        int ii = int(i);
        for (size_t j = 0, end_j = vertTriNeighbors[i].size(); j < end_j; ++j)
        {
            auto triangleIndex = vertTriNeighbors[i][j];
            candidate[0] = triangles[triangleIndex].vert[0];
            candidate[1] = triangles[triangleIndex].vert[1];
            candidate[2] = triangles[triangleIndex].vert[2];

            if (candidate[0] == ii)
            {
                candidate[0] = -1;
            }

            if (candidate[1] == ii)
            {
                candidate[1] = -1;
            }

            if (candidate[2] == ii)
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
void Mesh::upadateAABB()
{
    core::Vec3d minVector(
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max());
    core::Vec3d maxVector(
        std::numeric_limits<double>::min(),
        std::numeric_limits<double>::min(),
        std::numeric_limits<double>::min());

    auto &vertices = this->getVertices();
    for (size_t i = 0, end = this->getNumberOfVertices(); i < end; i++)
    {
        minVector = minVector.array().min(vertices[i].array());
        maxVector = maxVector.array().max(vertices[i].array());
    }

    core::Vec3d skinOffset = (maxVector-minVector)*SIMMEDTK_MESH_AABBSKINFACTOR;

    Eigen::AlignedBox3d bbox(minVector-skinOffset,
                             maxVector+skinOffset);
    this->setBoundingBox(bbox);
}

/// \brief
void Mesh::calcEdges()
{
    Edge edge;
    edges.reserve(SIMMEDTK_MESH_RESERVEDMAXEDGES);

    for (size_t i = 0, end = this->getNumberOfVertices(); i < end; i++)
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
void Mesh::translate(core::Vec3d p_offset)
{
    auto &vertices = this->getVertices();
    auto &origVerts = this->getOrigVertices();
    for (size_t i = 0, end = this->getNumberOfVertices(); i < end; ++i)
    {
        vertices[i] = vertices[i] + p_offset;
        origVerts[i] = origVerts[i] + p_offset;
    }

    upadateAABB();
}

void Mesh::scale(core::Vec3d p_scaleFactors)
{
    auto &vertices = this->getVertices();
    auto &origVerts = this->getOrigVertices();
    for (size_t i = 0, end = this->getNumberOfVertices(); i < end; ++i)
    {
        vertices[i].array() *= p_scaleFactors.array();
        origVerts[i].array() *= p_scaleFactors.array();
    }

    upadateAABB();
}

void Mesh::rotate(const Quaterniond &R)
{
    BaseMesh::rotate(R);
    for (size_t i = 0, end = this->getNumberOfVertices(); i < end; ++i)
    {
        vertNormals[i] = R * vertNormals[i];
    }

    for (int i = 0; i < nbrTriangles; i++)
    {
        triNormals[i] = R * triNormals[i];
    }

    calcTriangleTangents();
    upadateAABB();
}

/// \brief
void Mesh::updateTriangleAABB()
{
    auto const &vertices = this->getVertices();
    for (int i = 0; i < nbrTriangles; i++)
    {
        triAABBs[i].setEmpty();
        triAABBs[i].extend(vertices[triangles[i].vert[0]]);
        triAABBs[i].extend(vertices[triangles[i].vert[1]]);
        triAABBs[i].extend(vertices[triangles[i].vert[2]]);
    }
}

/// \brief
void Mesh::checkCorrectWinding()
{
    for (int i = 0; i < nbrTriangles; i++)
    {
        auto &x = triangles[i].vert;

        for (int j = 0; j < nbrTriangles; j++)
        {
            if (j == i)
            {
                continue;
            }

            auto p = triangles[j].vert;

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

void Mesh::updateSurfaceMeshFromVegaFormat(std::shared_ptr<ObjMesh> vegaSurfaceMesh)
{
    Vec3d p;
    //copy the vertex co-ordinates
    auto &vertices = this->getVertices();
    for (size_t i = 0, end = this->getNumberOfVertices(); i < end; ++i)
    {
       p = vegaSurfaceMesh->getPosition(i);
       vertices[i][0] = p[0];
       vertices[i][1] = p[1];
       vertices[i][2] = p[2];
    }
}

bool Mesh::importSurfaceMeshFromVegaFormat(std::shared_ptr<ObjMesh> vegaSurfaceMesh, const bool perProcessingStage)
{
    if (!vegaSurfaceMesh)
        return false;

    if(!vegaSurfaceMesh->isTriangularMesh())
    {
//         if (this->log != nullptr)
//         {
//             this->log->addError("Error : SimMedTK supports only triangular surface mesh. Vega mesh is not a triangle mesh!");
            return false;
//         }
    }


    // temporary arrays
    int numVertices(0);
    int numTriangles(0);

    double* vegaVertices = nullptr;
    int* vegaTriangles = nullptr;
    vegaSurfaceMesh->exportGeometry(&numVertices, &vegaVertices, &numTriangles , &vegaTriangles, nullptr, nullptr);

    this->nbrTriangles = numTriangles;

    initVertexArrays(numVertices);
    initTriangleArrays(numTriangles);

    //copy the triangle connectivity information
    for(int i = 0, fastIndex = 0; i < this->nbrTriangles; ++i, fastIndex+=3)
    {
        this->triangles[i].vert[0] = vegaTriangles[fastIndex+0];
        this->triangles[i].vert[1] = vegaTriangles[fastIndex+1];
        this->triangles[i].vert[2] = vegaTriangles[fastIndex+2];
    }

    auto &vertexArray = this->getVertices();
    for(int i = 0, fastIndex = 0; i < numVertices; ++i, fastIndex+=3)
    {
        vertexArray[i][0] = vegaVertices[fastIndex+0];
        vertexArray[i][1] = vegaVertices[fastIndex+1];
        vertexArray[i][2] = vegaVertices[fastIndex+2];
    }

    if(perProcessingStage)
    {
        updateOriginalVertsWithCurrent();
    }

    //deallocate temporary arrays
    delete [] vegaTriangles;
    delete [] vegaVertices;

    return 1;

}
