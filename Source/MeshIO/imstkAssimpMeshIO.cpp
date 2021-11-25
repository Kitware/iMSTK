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
#include "imstkLineMesh.h"
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
std::shared_ptr<PointSet>
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
        LOG(WARNING) << "Error: file type not supported for input " << filePath;
        return nullptr;
        break;
    }
}

std::shared_ptr<PointSet>
AssimpMeshIO::readMeshData(const std::string& filePath)
{
    // Import mesh(es) and apply some clean-up operations
    Assimp::Importer importer;
    auto             scene = importer.ReadFile(filePath, AssimpMeshIO::getDefaultPostProcessSteps());

    // Check if there is actually a mesh or if the file can be read
    CHECK(scene != nullptr && scene->HasMeshes()) << "Error: could not read with Assimp reader for input " << filePath;

    // Get first mesh
    auto importedMesh = scene->mMeshes[0];

    auto surfMesh = AssimpMeshIO::convertAssimpMesh(importedMesh);
    if (!surfMesh)
    {
        LOG(WARNING) << "Error: Invalid surface mesh. Input: " << filePath;
    }

    return surfMesh;
}

std::shared_ptr<PointSet>
AssimpMeshIO::convertAssimpMesh(aiMesh* importedMesh)
{
    // Get mesh information
    auto numVertices = importedMesh->mNumVertices;
    auto numFaces    = importedMesh->mNumFaces;

    if (numVertices == 0)
    {
        LOG(WARNING) << "Error: mesh has no vertices.";
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

    // Count cell types
    int numLines = 0;
    int numTris  = 0;
    int numQuads = 0;
    for (unsigned int i = 0; i < numFaces; i++)
    {
        aiFace cell = importedMesh->mFaces[i];
        numTris  += static_cast<int>(cell.mNumIndices == 3);
        numLines += static_cast<int>(cell.mNumIndices == 2);
        numQuads += static_cast<int>(cell.mNumIndices == 4);
    }

    // If there are no cells in this mesh, read PointSet (assimp does not support)
    /*if (importedMesh->mNumFaces == 0)
    {
        auto ptMesh = std::make_shared<PointSet>(std::string(importedMesh->mName.C_Str()));
        ptMesh->initialize(verticesPtr);
        return ptMesh;
    }*/
    // If no triangles or quads, but lines, read LineMesh
    if (numTris == 0 && numQuads == 0 && numLines > 0)
    {
        auto                  cellsPtr = std::make_shared<VecDataArray<int, 2>>(numLines);
        VecDataArray<int, 2>& cells    = *cellsPtr;

        unsigned int j = 0;
        for (unsigned int i = 0; i < numFaces; i++)
        {
            aiFace        triangle = importedMesh->mFaces[i];
            unsigned int* indices  = triangle.mIndices;
            if (triangle.mNumIndices == 2)
            {
                cells[j++] = Vec2i(indices[0], indices[1]);
            }
        }
        auto lineMesh = std::make_shared<LineMesh>(std::string(importedMesh->mName.C_Str()));
        lineMesh->initialize(verticesPtr, cellsPtr);
        return lineMesh;
    }
    // Otherwise just read SurfaceMesh
    else
    {
        auto                  cellsPtr = std::make_shared<VecDataArray<int, 3>>(numTris);
        VecDataArray<int, 3>& cells    = *cellsPtr;

        unsigned int j = 0;
        for (unsigned int i = 0; i < numFaces; i++)
        {
            aiFace        triangle = importedMesh->mFaces[i];
            unsigned int* indices  = triangle.mIndices;
            if (triangle.mNumIndices == 3)
            {
                cells[j++] = Vec3i(indices[0], indices[1], indices[2]);
            }
        }

        // Vertex normals, tangents, and bitangents
        std::shared_ptr<VecDataArray<double, 3>> normalsPtr    = std::make_shared<VecDataArray<double, 3>>(numVertices);
        VecDataArray<double, 3>&                 normals       = *normalsPtr;
        std::shared_ptr<VecDataArray<float, 3>>  tangentsPtr   = std::make_shared<VecDataArray<float, 3>>(numVertices);
        VecDataArray<float, 3>&                  tangents      = *tangentsPtr;
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

        auto surfMesh = std::make_shared<SurfaceMesh>(std::string(importedMesh->mName.C_Str()));
        surfMesh->initialize(verticesPtr, cellsPtr, normalsPtr, false);
        surfMesh->setVertexNormals("normals", normalsPtr);

        if (importedMesh->HasTangentsAndBitangents() && importedMesh->HasTextureCoords(0))
        {
            for (unsigned int i = 0; i < numVertices; i++)
            {
                auto tangentX = importedMesh->mTangents[i].x;
                auto tangentY = importedMesh->mTangents[i].y;
                auto tangentZ = importedMesh->mTangents[i].z;
                tangents[i] = Vec3f(tangentX, tangentY, tangentZ);

                auto bitangentX = importedMesh->mBitangents[i].x;
                auto bitangentY = importedMesh->mBitangents[i].y;
                auto bitangentZ = importedMesh->mBitangents[i].z;
                bitangents[i] = Vec3d(bitangentX, bitangentY, bitangentZ);
            }
            surfMesh->setVertexTangents("tangents", tangentsPtr);
        }

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
            surfMesh->setVertexTCoords("tCoords", UVs);
        }
        return surfMesh;
    }
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
