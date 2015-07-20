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

MeshModel::MeshModel() {}
MeshModel::~MeshModel() {}
void MeshModel::load(const std::string& meshName, const BaseMesh::MeshFileType& type)
{
    this->mesh.reset();

    switch(type)
    {
        case BaseMesh::MeshFileType::Obj:
        {
            this->mesh = std::make_shared<SurfaceMesh>();
            break;
        }

        case BaseMesh::MeshFileType::ThreeDS: // TODO: Is this a surface or volume mesh?
        {
            this->mesh = std::make_shared<SurfaceMesh>();
            break;
        }

        case BaseMesh::MeshFileType::Volume:
        {
            this->mesh = std::make_shared<VolumeMesh>();
            break;
        }

        default:
        {
            std::cerr << "Unknown mesh type." << std::endl;
        }
    }

    this->mesh->loadMesh(meshName, type);
}
const core::Vec3d& MeshModel::getNormal(size_t i) const
{
    return this->mesh->triNormals[i];
}
std::array<core::Vec3d,3> MeshModel::getTrianglePositions(size_t i) const
{
    std::array<core::Vec3d, 3> vertices;
    vertices[0] = this->mesh->vertices[this->mesh->triangles[i].vert[0]];
    vertices[1] = this->mesh->vertices[this->mesh->triangles[i].vert[1]];
    vertices[2] = this->mesh->vertices[this->mesh->triangles[i].vert[2]];

    return vertices;
}
const core::StdVector3d& MeshModel::getVertices() const
{
    return mesh->getVertices();
}
void MeshModel::draw()
{
    RenderDelegate::Ptr delegate = this->mesh->getRenderDelegate();
    if (delegate)
      delegate->draw();
}
void MeshModel::setModelMesh(std::shared_ptr< Mesh > modelMesh)
{
    this->mesh.reset();
    this->mesh = modelMesh;
}
std::shared_ptr< Mesh > MeshModel::getMesh()
{
    return this->mesh;
}
void MeshModel::load(const std::string& meshFileName, const std::string& textureFileName, const std::string& textureName)
{
    this->load(meshFileName, BaseMesh::MeshFileType::Obj);

    if(nullptr != this->mesh)
    {
        //Initialize the texture manager
        TextureManager::init();

        //Load in the texture for the model
        TextureManager::loadTexture(textureFileName, textureName);
        this->mesh->assignTexture(textureName);
    }
}
void MeshModel::setRenderDetail(std::shared_ptr< RenderDetail > renderDetail)
{
    this->mesh->setRenderDetail(renderDetail);
}
