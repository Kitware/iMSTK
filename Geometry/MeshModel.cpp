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

#include "Geometry/MeshModel.h"
#include "IO/IOMesh.h"
#include "Core/RenderDelegate.h"

MeshModel::MeshModel()
{
    this->setRenderDelegate(
        Factory<RenderDelegate>::createConcreteClassForGroup(
            "MeshRenderDelegate",RenderDelegate::RendererType::VTK));
}
MeshModel::~MeshModel() {}
void MeshModel::load(const std::string& meshName)
{
    this->mesh.reset();

    std::shared_ptr<IOMesh> reader = std::make_shared<IOMesh>();
    reader->read(meshName);

    this->mesh = reader->getMesh();
	this->mesh->setMeshType ((int)reader->getFileType());
}

const std::vector<core::Vec3d>& MeshModel::getVertices() const
{
    return mesh->getVertices();
}
const std::vector<std::array<size_t,3>>& MeshModel::getTriangles() const
{
    return this->mesh->getTriangles();
}
void MeshModel::draw()
{
    RenderDelegate::Ptr delegate = this->mesh->getRenderDelegate();
    if (delegate)
      delegate->draw();
}
void MeshModel::setModelMesh(std::shared_ptr< Core::BaseMesh > modelMesh)
{
    this->mesh.reset();
    this->mesh = modelMesh;
}
std::shared_ptr<Core::BaseMesh> MeshModel::getMesh()
{
    return this->mesh;
}
void MeshModel::setRenderDetail(std::shared_ptr< RenderDetail > renderDetail)
{
    this->mesh->setRenderDetail(renderDetail);
}
void MeshModel::addTexture(const std::string& textureFileName, const std::string& textureName)
{
    std::shared_ptr<SurfaceMesh> surfaceMesh = std::dynamic_pointer_cast<SurfaceMesh>(this->mesh);
    if(!surfaceMesh)
    {
        std::cerr << "Cant assign texture to non-surface mesh." << std::endl;
        return;
    }

    if(surfaceMesh->getRenderDetail())
    {
        surfaceMesh->getRenderDetail()->setTextureFilename(textureFileName);
    }
    else
    {
        auto renderDetail = std::make_shared<RenderDetail>();
        renderDetail->setTextureFilename(textureFileName);
        surfaceMesh->setRenderDetail(renderDetail);
    }

    surfaceMesh->assignTexture(textureName);
}
