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
#include <array>

// SimMedTK includes
#include "Core/ModelRepresentation.h"
#include "Mesh/SurfaceMesh.h"
#include "Mesh/VolumeMesh.h"

///
/// @brief Mesh representation of a model.
/// Base class used by all models that can be represented by a mesh
///
/// @see MeshCollisionModel
///
class MeshModel : public ModelRepresentation
{
public:
    ///
    /// @brief Constructor
    ///
    MeshModel();

    ///
    /// @brief Destructor
    ///
    virtual ~MeshModel();

    ///
    /// @brief Loads the mesh and stores it.
    ///
    void load(const std::string& meshName, const Core::BaseMesh::MeshFileType &type);

    ///
    /// @brief Loads the mesh with texture and stores it. Only surface meshes allowed.
    ///
    void load(const std::string& meshFileName, const std::string& textureFileName, const std::string& textureName);

    ///
    /// @brief Set the rendering details for this mesh
    ///
    void setRenderDetail(std::shared_ptr<RenderDetail> renderDetail);

    ///
    /// @brief Returns normal vectors for triangles on mesh surface
    ///
    const core::Vec3d &getNormal(size_t i) const;

    ///
    /// @brief Returns array of vertices for triangle on surface
    ///
    std::array<core::Vec3d,3> getTrianglePositions(size_t i) const;

    ///
    /// @brief Returns array of vertices
    ///
    const std::vector<core::Vec3d> &getVertices() const;

    ///
    /// @brief Draw this mesh
    ///
    void draw() override;

    ///
    /// @brief Set internal mesh data structure
    ///
    void setModelMesh(std::shared_ptr<Mesh> modelMesh);

    ///
    /// @brief Returns pointer to undelying mesh object.
    ///
    std::shared_ptr<Mesh> getMesh() override;

protected:
    std::shared_ptr<Mesh> mesh; // Underlying mesh
};

#endif // SMMESHMODEL_H
