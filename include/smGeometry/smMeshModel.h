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

#ifndef SMMESHMODEL_H
#define SMMESHMODEL_H

// STL includes
#include <memory>

// SimMedTK includes
#include "smCore/smModelRepresentation.h"
#include "smMesh/smSurfaceMesh.h"
#include "smMesh/smVolumeMesh.h"

///
/// @brief Mesh representation of a model.
/// Base class used by all models that can be represented by a mesh
///
/// @see smMeshCollisionModel
///
class smMeshModel : public smModelRepresentation
{
public:
    ///
    /// @brief Constructor
    ///
    smMeshModel();

    ///
    /// @brief Destructor
    ///
    virtual ~smMeshModel();

    ///
    /// @brief Loads the mesh and stores it.
    ///
    void load(const std::string& meshName, const smMeshFileType &type);

    ///
    /// @brief Loads the mesh with texture and stores it. Only surface meshes allowed.
    ///
    void load(const std::string& meshFileName, const std::string& textureFileName, const std::string& textureName)
    {
        this->load(meshFileName,SM_FILETYPE_OBJ);
        if(nullptr != this->mesh)
        {
            //Initialize the texture manager
            smTextureManager::init();

            //Load in the texture for the model
            smTextureManager::loadTexture(textureFileName,textureName);
            this->mesh->assignTexture(textureName);
        }
    }

    ///
    /// @brief Set the rendering details for this mesh
    ///
    void setRenderDetail(std::shared_ptr<smRenderDetail> renderDetail)
    {
        this->mesh->setRenderDetail(renderDetail);
    }

    ///
    /// @brief Returns normal vectors for triangles on mesh surface
    ///
    const smVec3d &getNormal(size_t i) const;

    ///
    /// @brief Returns array of vertices for triangle on surface
    ///
    std::array<smVec3d,3> getTrianglePositions(size_t i) const;

    ///
    /// @brief Returns array of vertices
    ///
    const smStdVector3d &getVertices() const;

    ///
    /// @brief Draw this mesh
    ///
    void draw() override;

    ///
    /// @brief Set internal mesh data structure
    ///
    void setModelMesh(std::shared_ptr<smMesh> modelMesh);

    ///
    /// @brief Returns pointer to undelying mesh object.
    ///
    std::shared_ptr<smMesh> getMesh() override;

protected:
    std::shared_ptr<smMesh> mesh; // Underlying mesh
};

#endif // SMMESHMODEL_H
