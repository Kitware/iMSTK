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

#ifndef BASEMESH_H
#define BASEMESH_H

// Eigen includes
#include <Eigen/Geometry>

#include "Core/CoreClass.h"
#include "Core/Vector.h"

class CollisionGroup;
class ErrorLog;

namespace Core {

///
///
/// \brief Base class for all meshes used in the simulation.
///
/// \see \SurfaceMesh and \VolumeMesh
///
class BaseMesh: public CoreClass
{
public:
    struct TextureAttachment;

    ///
    /// \brief designates what purpose/scenario the mesh is used for
    ///
    enum class MeshType
    {
        Deformable,
        DeformableCutable,
        RigidCutable,
        Rigid
    };

    ///
    /// \brief designates input mesh file type
    ///
    enum class MeshFileType
    {
        None,
        Obj,
        ThreeDS,
        Volume,
        Vtk
    };

    ///
    /// \brief constructor
    ///
    BaseMesh();
    ///
    /// \brief Stores the texture locally. It assumes that the texture has been
    ///  already added to the \TextureManager
    ///
    void assignTexture(const int p_textureId);

    ///
    /// \brief assign the texture
    ///
    void assignTexture(const std::string& newTextureFileName, const std::string& referenceName);

    ///
    /// \brief Copy vertices into origVerts the original texture vertices with the current
    ///
    void updateOriginalVertsWithCurrent();

    ///
    /// \brief query if the mesh has textures available for rendering
    ///
    bool isMeshTextured() const;

    ///
    /// \brief Returns vertices coordinates
    ///
    const std::vector<core::Vec3d> &getVertices() const;
    std::vector<core::Vec3d> &getVertices();

    ///
    /// \brief Get the total number of vertices
    ///
    size_t getNumberOfVertices() const;

    ///
    /// \brief Return the collision group this mesh belongs to.
    ///
    std::shared_ptr<CollisionGroup> getCollisionGroup() const;

    ///
    /// \brief Returns the bounding box for this mesh.
    ///
    Eigen::AlignedBox3d getBoundingBox() const;

    ///
    /// \brief Returns the bounding box for this mesh.
    ///
    size_t getRenderingId() const;

    ///
    /// \brief Returns the the name of ith texture.
    ///
    std::string getTextureFileName(const size_t i) const;

    ///
    /// \brief Returns the texture coordinates array.
    ///
    const std::vector<core::Vec2f,
            Eigen::aligned_allocator<core::Vec2f>>
    &getTextureCoordinates() const;
    std::vector<core::Vec2f,
            Eigen::aligned_allocator<core::Vec2f>>
    &getTextureCoordinates();

    ///
    /// \brief Returns the bounding box for this mesh.
    ///
    const std::vector<std::shared_ptr<TextureAttachment>> &getTextures() const;

    ///
    /// \brief Returns the bounding box for this mesh.
    ///
    const int &getTextureId(size_t i) const;

private:
    // Data arrays - Vertices only
    // vertices co-ordinate data at time t
    std::vector<core::Vec3d> vertices;

    // vertices co-ordinate data at time t=0
    std::vector<core::Vec3d> origVerts;

    // Collision group this mesh belongs to.
    std::shared_ptr<CollisionGroup> collisionGroup;

    // Logger
    std::shared_ptr<ErrorLog> log;

    // Bounding Box
    Eigen::AlignedBox3d aabb;

    // Render identification, only used in \StylusRenderDelegate.
    size_t renderingID;

    // Textures attached to this mesh.
    std::vector<std::shared_ptr<TextureAttachment>> textures;

    // Texture coordinates
    std::vector<core::Vec2f,
                Eigen::aligned_allocator<core::Vec2f>> textureCoord;
};

}// namespace Core
#endif //
