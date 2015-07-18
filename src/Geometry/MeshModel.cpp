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

#include "MeshModel.h"
#include "Rendering/TextureManager.h"

smMeshModel::smMeshModel() {}
smMeshModel::~smMeshModel() {}
void smMeshModel::load(const std::string& meshName, const smMeshFileType& type)
{
    this->mesh.reset();

    switch(type)
    {
        case SM_FILETYPE_OBJ:
        {
            this->mesh = std::make_shared<smSurfaceMesh>();
            break;
        }

        case SM_FILETYPE_3DS: // TODO: Is this a surface or volume mesh?
        {
            this->mesh = std::make_shared<smSurfaceMesh>();
            break;
        }

        case SM_FILETYPE_VOLUME:
        {
            this->mesh = std::make_shared<smVolumeMesh>();
            break;
        }

        default:
        {
            std::cerr << "Unknown mesh type." << std::endl;
        }
    }

    this->mesh->loadMesh(meshName, type);
}
const smVec3d& smMeshModel::getNormal(size_t i) const
{
    return this->mesh->triNormals[i];
}
std::array<smVec3d,3> smMeshModel::getTrianglePositions(size_t i) const
{
    std::array<smVec3d, 3> vertices;
    vertices[0] = this->mesh->vertices[this->mesh->triangles[i].vert[0]];
    vertices[1] = this->mesh->vertices[this->mesh->triangles[i].vert[1]];
    vertices[2] = this->mesh->vertices[this->mesh->triangles[i].vert[2]];

    return vertices;
}
const smStdVector3d& smMeshModel::getVertices() const
{
    return mesh->getVertices();
}
void smMeshModel::draw()
{
    smRenderDelegate::Ptr delegate = this->mesh->getRenderDelegate();
    if (delegate)
      delegate->draw();
}
void smMeshModel::setModelMesh(std::shared_ptr< smMesh > modelMesh)
{
    this->mesh.reset();
    this->mesh = modelMesh;
}
std::shared_ptr< smMesh > smMeshModel::getMesh()
{
    return this->mesh;
}
void smMeshModel::load(const std::string& meshFileName, const std::string& textureFileName, const std::string& textureName)
{
    this->load(meshFileName, SM_FILETYPE_OBJ);

    if(nullptr != this->mesh)
    {
        //Initialize the texture manager
        smTextureManager::init();

        //Load in the texture for the model
        smTextureManager::loadTexture(textureFileName, textureName);
        this->mesh->assignTexture(textureName);
    }
}
void smMeshModel::setRenderDetail(std::shared_ptr< smRenderDetail > renderDetail)
{
    this->mesh->setRenderDetail(renderDetail);
}
