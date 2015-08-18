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

#include "Mesh/SurfaceMesh.h"
#include "Core/CollisionConfig.h"
#include "Core/RenderDelegate.h"

///
/// \brief Texture management structure
///
struct SurfaceMesh::TextureAttachment
{
    TextureAttachment() :
    textureId(-1),
    textureName("")
    {
    }

    // Texture id
    int textureId;

    // Texture internal name
    std::string textureName;
};

SurfaceMesh::SurfaceMesh() : useThreeDSTexureCoordinates(false), useOBJDSTexureCoordinates(false)
{
    this->setRenderDelegate(
      Factory<RenderDelegate>::createConcreteClassForGroup("MeshRenderDelegate",300));
}
SurfaceMesh::~SurfaceMesh()
{
}
core::Vec3d SurfaceMesh::computeTriangleNormal(int triangle)
{
    auto t = this->triangleArray[triangle];

    const core::Vec3d &v0 = this->vertices[t[0]];

    return (this->vertices[t[1]]-v0).cross(this->vertices[t[2]]-v0).normalized();
}
void SurfaceMesh::computeTriangleNormals()
{
    this->triangleNormals.resize(this->triangleArray.size());

    for(size_t i = 0, end = this->triangleArray.size(); i < end; ++i)
    {
        const auto &t = this->triangleArray[i];
        const auto &v0 = this->vertices[t[0]];

        this->triangleNormals[i] = (this->vertices[t[1]]-v0).cross(this->vertices[t[2]]-v0).normalized();
    }
}
void SurfaceMesh::computeVertexNormals()
{
    this->vertexNormals.resize(this->vertices.size(),core::Vec3d::Zero());

    for(size_t i = 0, end = this->vertices.size(); i < end; ++i)
    {
        for(auto const &j : this->vertexTriangleNeighbors[i])
        {
            this->vertexNormals[i] += this->triangleNormals[j];
        }

        this->vertexNormals[i].normalize();
    }
}
void SurfaceMesh::computeVertexNeighbors()
{
    this->vertexNeighbors.resize(this->vertices.size());

    if(vertexTriangleNeighbors.size() == 0)
    {
        this->computeVertexTriangleNeighbors();
    }

    for(size_t i = 0, end = this->vertices.size(); i < end; ++i)
    {
        for(auto const &j : this->vertexTriangleNeighbors[i])
        {
            for(auto const &vertex : this->triangleArray[j])
            {
                if(vertex != i)
                {
                    this->vertexNeighbors[i].push_back(vertex);
                }
            }
        }
    }
}
void SurfaceMesh::computeVertexTriangleNeighbors()
{
    this->vertexTriangleNeighbors.resize(this->vertices.size());

    int triangle = 0;

    for(auto const &t : this->triangleArray)
    {
        this->vertexTriangleNeighbors[t[0]].push_back(triangle);
        this->vertexTriangleNeighbors[t[1]].push_back(triangle);
        this->vertexTriangleNeighbors[t[2]].push_back(triangle);
        triangle++;
    }
}
void SurfaceMesh::computeTriangleTangents()
{
    // Check if there are texture coordinates
    if(!this->hasTextureCoordinates())
    {
        std::cerr << "Can't compute tangents without texture coordinates." << std::endl;
        return;
    }

    this->triangleTangents.resize(this->triangleArray.size());

    // First, calculate the triangle tangents
    for(size_t t = 0, end = this->triangleArray.size(); t < end; ++t)
    {
        const auto &triangle = this->triangleArray[t];

        // Get triangle vertices
        const auto &v0 = this->vertices[triangle[0]];
        const auto &v1 = this->vertices[triangle[1]];
        const auto &v2 = this->vertices[triangle[2]];

        // Get texture coordinates for triangle
        const auto &t0 = this->textureCoord[triangle[0]];
        const auto &t1 = this->textureCoord[triangle[1]];
        const auto &t2 = this->textureCoord[triangle[2]];

        // COMMENT: I am not sure why two different types of tangent calculations are used here.
        this->triangleTangents[t] = ((t1[1] - t0[1])*(v1 - v0) - (t2[1] - t0[1])*(v2 - v0));

        if(this->useThreeDSTexureCoordinates)
        {
            float r = 1.0/((t1[0]-t0[0])*(t2[1]-t0[1]) - (t1[1]-t0[1])*(t2[0]-t0[0]));
            this->triangleTangents[t] *= r;
        }

        this->triangleTangents[t].normalize();
    }

    // Calculate the vertex tangents
    if(this->useThreeDSTexureCoordinates || this->useOBJDSTexureCoordinates)
    {
        for(size_t v = 0, end = this->vertices.size(); v < end; ++v)
        {
            this->vertexTangents[v][0] = this->vertexTangents[v][1] = this->vertexTangents[v][2] = 0;

            for(size_t i = 0; i < this->vertexTriangleNeighbors[v].size(); i++)
            {
                this->vertexTangents[v] += this->triangleTangents[this->vertexTriangleNeighbors[v][i]];
            }

            this->vertexTangents[v].normalize();
            this->vertexTangents[v] -= this->vertexNormals[v]*this->vertexNormals[v].dot(this->vertexTangents[v]);
            this->vertexTangents[v].normalize();
        }
    }
}
void SurfaceMesh::checkTriangleOrientation()
{
    for(int i = 0, end_i = this->triangleArray.size(); i < end_i; ++i)
    {
        auto const &x = this->triangleArray[i];

        for(int j = 0, end_j = this->triangleArray.size(); j < end_j && j != i; ++j)
        {
            auto const &y = this->triangleArray[j];

            if((x[0] == y[0] && x[1] == y[1]) ||
                    (x[0] == y[1] && x[1] == y[2]) ||
                    (x[0] == y[1] && x[1] == y[2]) ||
                    (x[0] == y[2] && x[1] == y[0]) ||
                    (x[1] == y[0] && x[2] == y[1]) ||
                    (x[1] == y[1] && x[2] == y[2]) ||
                    (x[1] == y[2] && x[2] == y[0]) ||
                    (x[2] == y[0] && x[0] == y[1]) ||
                    (x[2] == y[1] && x[0] == y[2]) ||
                    (x[2] == y[2] && x[0] == y[0])
              )
            {
                std::cout << "Wrong Winding Triangles:" << i << "," << j << "\n";
            }
        }
    }
}
const std::string& SurfaceMesh::getTextureFileName(const size_t i) const
{
    return TextureManager::getTexture(textures[i]->textureId)->textureFileName;
}
const std::vector< Eigen::Matrix< float, int(2), int(1) >, Eigen::aligned_allocator< Eigen::Matrix< float, int(2), int(1) > > >& SurfaceMesh::getTextureCoordinates() const
{
    return this->textureCoord;
}
std::vector< Eigen::Matrix< float, int(2), int(1) >, Eigen::aligned_allocator< Eigen::Matrix< float, int(2), int(1) > > >& SurfaceMesh::getTextureCoordinates()
{
    return this->textureCoord;
}
const std::vector< std::shared_ptr< SurfaceMesh::TextureAttachment > >& SurfaceMesh::getTextures() const
{
    return this->textures;
}
const int& SurfaceMesh::getTextureId(size_t i) const
{
    return this->textures[i]->textureId;
}
void SurfaceMesh::addTextureCoordinate(const core::Vec2f& coord)
{
    this->textureCoord.push_back(coord);
}
bool SurfaceMesh::hasTextureCoordinates() const
{
    return !this->textureCoord.empty();
}
void SurfaceMesh::addTextureCoordinate(const float& x, const float& y)
{
    this->textureCoord.emplace_back(x,y);
}
void SurfaceMesh::assignTexture(const std::string& referenceName)
{
    int id;
    TextureManager::findTextureId(referenceName,id);

    if(id < 0)
    {
        std::cerr << "The texture " << referenceName
            << " cant be attached because it has not been processed by the manager." << std::endl;
        return;
    }

    auto attachment = std::make_shared<TextureAttachment>();
    attachment->textureName = referenceName;
    attachment->textureId = id;
    this->textures.push_back(attachment);
}
bool SurfaceMesh::isMeshTextured() const
{
    return !this->textures.empty();
}
void SurfaceMesh::setUseOBJTexture(bool use)
{
    this->useOBJDSTexureCoordinates = use;
}
void SurfaceMesh::setUseThreDSTexture(bool use)
{
    this->useThreeDSTexureCoordinates = use;
}
void SurfaceMesh::print() const
{
    std::cout << "----------------------------\n";
    std::cout << "Mesh Info for   : " << this->getName() <<"\n\t";
    std::cout << "Num. vertices   : " << this->getNumberOfVertices() <<"\n\t";
    std::cout << "Num. triangles  : " << this->getTriangles().size() << "\n\t";
    std::cout << "Is mesh textured: " << this->isMeshTextured() << "\n";
    std::cout << "----------------------------\n";
}
const core::Vec3d& SurfaceMesh::getTriangleNormal(size_t i) const
{
    return this->triangleNormals[i];
}
const core::Vec3d& SurfaceMesh::getTriangleTangent(size_t i) const
{
    return this->triangleTangents[i];
}
const core::Vec3d& SurfaceMesh::getVertexNormal(size_t i) const
{
    return this->vertexNormals[i];
}
const core::Vec3d& SurfaceMesh::getVertexTangent(size_t i) const
{
    return this->vertexTangents[i];
}
const std::vector<core::Vec3d>& SurfaceMesh::getVertexTangents() const
{
    return this->vertexTangents;
}
std::vector<core::Vec3d>& SurfaceMesh::getVertexTangents()
{
    return this->vertexTangents;
}
size_t SurfaceMesh::getNumberOfTriangles() const
{
    return this->triangleArray.size();
}

