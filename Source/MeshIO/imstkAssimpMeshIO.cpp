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

#include "imstkAssimpMeshIO.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace imstk
{
std::shared_ptr<SurfaceMesh>
AssimpMeshIO::read(
    const std::string& filePath,
    MeshFileType       type)
{
    switch (type)
    {
    case MeshFileType::OBJ:
    case MeshFileType::DAE:
    case MeshFileType::FBX:
    case MeshFileType::_3DS:
        return AssimpMeshIO::readMeshData(filePath);
        break;
    default:
        LOG(WARNING) << "AssimpMeshIO::read error: file type not supported";
        return nullptr;
        break;
    }
}

std::shared_ptr<SurfaceMesh>
AssimpMeshIO::readMeshData(const std::string& filePath)
{
    // Import mesh(es) and apply some clean-up operations
    Assimp::Importer importer;
    auto             scene = importer.ReadFile(filePath, AssimpMeshIO::getDefaultPostProcessSteps());

    // Check if there is actually a mesh or if the file can be read
    CHECK(scene != nullptr && scene->HasMeshes()) << "AssimpMeshIO::readMeshData error: could not read with reader.";

    // Get first mesh
    auto importedMesh = scene->mMeshes[0];

    return AssimpMeshIO::convertAssimpMesh(importedMesh);
}

std::shared_ptr<SurfaceMesh>
AssimpMeshIO::convertAssimpMesh(aiMesh* importedMesh)
{
    // Build SurfaceMesh
    auto mesh = std::make_shared<SurfaceMesh>(std::string(importedMesh->mName.C_Str()));

    // Get mesh information
    auto numVertices  = importedMesh->mNumVertices;
    auto numTriangles = importedMesh->mNumFaces;

    if (numVertices == 0)
    {
        LOG(WARNING) << "AssimpMeshIO::readMeshData error: mesh has no vertices.";
        return nullptr;
    }

    // Vertex positions
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = std::make_shared<VecDataArray<double, 3>>(numVertices);
    VecDataArray<double, 3>&                 vertices    = *verticesPtr;

    for (unsigned int i = 0; i < numVertices; i++)
    {
        auto positionX = importedMesh->mVertices[i].x;
        auto positionY = importedMesh->mVertices[i].y;
        auto positionZ = importedMesh->mVertices[i].z;
        vertices[i] = Vec3d(positionX, positionY, positionZ);
    }

    // Triangles
    std::shared_ptr<VecDataArray<int, 3>> trianglesPtr = std::make_shared<VecDataArray<int, 3>>(numTriangles);
    VecDataArray<int, 3>&                 triangles    = *trianglesPtr;

    for (unsigned int i = 0; i < numTriangles; i++)
    {
        auto triangle = importedMesh->mFaces[i];
        auto indices  = triangle.mIndices;
        triangles[i][0] = indices[0];
        triangles[i][1] = indices[1];
        triangles[i][2] = indices[2];
    }

    // Vertex normals, tangents, and bitangents
    std::shared_ptr<VecDataArray<double, 3>> normalsPtr    = std::make_shared<VecDataArray<double, 3>>(numVertices);
    VecDataArray<double, 3>&                 normals       = *normalsPtr;
    std::shared_ptr<VecDataArray<double, 3>> tangentsPtr   = std::make_shared<VecDataArray<double, 3>>(numVertices);
    VecDataArray<double, 3>&                 tangents      = *tangentsPtr;
    std::shared_ptr<VecDataArray<double, 3>> bitangentsPtr = std::make_shared<VecDataArray<double, 3>>(numVertices);
    VecDataArray<double, 3>&                 bitangents    = *bitangentsPtr;

    if (importedMesh->HasNormals())
    {
        for (unsigned int i = 0; i < numVertices; i++)
        {
            auto normalX = importedMesh->mNormals[i].x;
            auto normalY = importedMesh->mNormals[i].y;
            auto normalZ = importedMesh->mNormals[i].z;
            normals[i] = Vec3d(normalX, normalY, normalZ);
        }
    }

    if (importedMesh->HasTangentsAndBitangents() && importedMesh->HasTextureCoords(0))
    {
        for (unsigned int i = 0; i < numVertices; i++)
        {
            auto tangentX = importedMesh->mTangents[i].x;
            auto tangentY = importedMesh->mTangents[i].y;
            auto tangentZ = importedMesh->mTangents[i].z;
            tangents[i] = Vec3d(tangentX, tangentY, tangentZ);

            auto bitangentX = importedMesh->mBitangents[i].x;
            auto bitangentY = importedMesh->mBitangents[i].y;
            auto bitangentZ = importedMesh->mBitangents[i].z;
            bitangents[i] = Vec3d(bitangentX, bitangentY, bitangentZ);
        }
    }

    mesh->initialize(verticesPtr, trianglesPtr, normalsPtr, false);

    mesh->setVertexNormals("normals", normalsPtr);
    mesh->setVertexTangents("tangents", tangentsPtr);

    // UV coordinates
    if (importedMesh->HasTextureCoords(0))
    {
        std::shared_ptr<VecDataArray<float, 2>> UVs    = std::make_shared<VecDataArray<float, 2>>(numVertices);
        VecDataArray<float, 2>&                 UVData = *UVs;

        auto texcoords = importedMesh->mTextureCoords[0];
        for (unsigned int i = 0; i < numVertices; i++)
        {
            UVData[i][0] = texcoords[i].x;
            UVData[i][1] = texcoords[i].y;
        }
        mesh->setVertexTCoords("tCoords", UVs);
    }
    return mesh;
}

unsigned int
AssimpMeshIO::getDefaultPostProcessSteps()
{
    unsigned int postProcessSteps =
        aiPostProcessSteps::aiProcess_GenSmoothNormals |
        aiPostProcessSteps::aiProcess_CalcTangentSpace |
        aiPostProcessSteps::aiProcess_JoinIdenticalVertices |
        aiPostProcessSteps::aiProcess_Triangulate |
        aiPostProcessSteps::aiProcess_ImproveCacheLocality;

    return postProcessSteps;
}
}
