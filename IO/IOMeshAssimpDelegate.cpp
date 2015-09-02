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


#include "Core/Factory.h"
#include "IO/IOMeshDelegate.h"
#include "Mesh/SurfaceMesh.h"

// Assimp includes
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class IOMeshAssimpDelegate : public IOMeshDelegate
{
public:
    void read()
    {
        auto fileName = this->meshIO->getFileName().c_str();
        // Tell Assimp to not import any of the following from the mesh it loads
        Assimp::Importer importer;
        importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
                                    aiComponent_CAMERAS | aiComponent_LIGHTS |
                                    aiComponent_MATERIALS | aiComponent_TEXTURES |
                                    aiComponent_BONEWEIGHTS | aiComponent_COLORS |
                                    aiComponent_TANGENTS_AND_BITANGENTS |
                                    aiComponent_NORMALS | aiComponent_ANIMATIONS);

        // Import the file, and do some post-processing
        const aiScene* scene = importer.ReadFile(fileName,
                                                 aiProcess_Triangulate |            //Triangulate any polygons that are not triangular
                                                 aiProcess_JoinIdenticalVertices |  //Ensures indexed vertices from faces
                                                 aiProcess_RemoveComponent |        //Removes the components in AI_CONFIG_PP_RVC_FLAGS
                                                 aiProcess_ImproveCacheLocality);   //Reorders triangles for better vertex cache locality

        if (scene == nullptr)
        {
            std::cerr << "Error: Error loading mesh: " + std::string(fileName) << std::endl;
            return;
        }

        // Extract the information from the aiScene's mesh objects
        aiMesh *mesh = scene->mMeshes[0]; //Guarenteed to have atleast one mesh

        std::vector<core::Vec3d> vertices;
        vertices.reserve(mesh->mNumVertices);

        // Get indexed vertex data
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            vertices.emplace_back(mesh->mVertices[i][0],
                                  mesh->mVertices[i][1],
                                  mesh->mVertices[i][2]);
        }

        std::vector<std::array<size_t,3>> triangleArray;
        triangleArray.reserve(mesh->mNumFaces);

        // Setup triangle/face data
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            //Make sure that the face is triangular
            if (mesh->mFaces[i].mNumIndices != 3)
            {
                std::cerr << "Error: Error loading mesh, non-triangular face found." << std::endl;
                return;
            }

            std::array<size_t,3> t = {mesh->mFaces[i].mIndices[0],
                                   mesh->mFaces[i].mIndices[1],
                                   mesh->mFaces[i].mIndices[2]};
            triangleArray.emplace_back(t);
        }

        this->setSurfaceMesh(vertices,triangleArray);

        // Get indexed texture coordinate data
        if (mesh->HasTextureCoords(0))
        {
            //Assimp supports 3D texture coords, but we only support 2D
            if (mesh->mNumUVComponents[0] != 2)
            {
                std::cerr << "Error: Error loading mesh, non-two dimensional texture coordinate found." << std::endl;
                return;
            }

            auto surfaceMesh = std::static_pointer_cast<SurfaceMesh>(this->meshIO->getMesh());
            auto &textureCoordinates = surfaceMesh->getTextureCoordinates();
            // Extract the texture data
            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                textureCoordinates.emplace_back(mesh->mTextureCoords[0][i][0],
                                                mesh->mTextureCoords[0][i][1]);
            }
        }
    }
    void write(){}
};


RegisterFactoryClass(IOMeshDelegate,IOMeshAssimpDelegate,IOMesh::ReaderGroup::Assimp);
