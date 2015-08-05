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
#include "Core/Quaternion.h"

class CollisionGroup;
class ErrorLog;

namespace Core {

///
///
/// \brief Base class for all meshes used in the simulation.
///
/// \see \SurfaceMesh
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
        Volume,
        Surface,
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
        Vtk
    };

    ///
    /// \brief constructor/Destructor
    ///
    BaseMesh();
    virtual ~BaseMesh();
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
    /// \brief Accessors for vertex coordinates
    ///
    const std::vector<core::Vec3d> &getVertices() const;
    std::vector<core::Vec3d> &getVertices();

    ///
    /// \brief Topology data accessors
    ///
    const std::vector<std::array<size_t,3>> &getTriangles() const;
    std::vector<std::array<size_t,3>> &getTriangles();
    void setTriangles(const std::vector<std::array<size_t,3>> &triangles);

    const std::vector<std::array<size_t,4>> &getTetrahedrons() const;
    std::vector<std::array<size_t,4>> &getTetrahedrons();
    void setTetrahedrons(const std::vector<std::array<size_t,4>> &tetrahedrons);

    const std::vector<std::array<size_t,8>> &getHexahedrons() const;
    std::vector<std::array<size_t,8>> &getHexahedrons();
    void setHexahedrons(const std::vector<std::array<size_t,8>> &hexahedrons);

    ///
    /// \brief Returns vertex ith coordinate
    ///
    const core::Vec3d &getVertex(const size_t i) const;
    core::Vec3d &getVertex(const size_t i);

    ///
    /// \brief Returns original vertex coordinates
    ///
    const std::vector<core::Vec3d> &getOrigVertices() const;
    std::vector<core::Vec3d> &getOrigVertices();

    ///
    /// \brief Get the total number of vertices
    ///
    size_t getNumberOfVertices() const;

    ///
    /// \brief Return the collision group this mesh belongs to.
    ///
    std::shared_ptr<CollisionGroup> &getCollisionGroup();

    ///
    /// \brief Set the rendering id
    ///
    size_t getRenderingId() const;

    ///
    /// \brief Set the rendering id
    ///
    void setRenderingId(size_t id);

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

    ///
    /// \brief Return true if it contains texture coordinates
    ///
    bool hasTextureCoordinates() const;

    ///
    /// \brief Add texture coordinates
    ///
    void addTextureCoordinate(const core::Vec2f &coord);
    void addTextureCoordinate(const float &x, const float &y);

    ///
    /// \brief Set/get bounding box
    ///
    void setBoundingBox(const Eigen::AlignedBox3d &box);
    const Eigen::AlignedBox3d &getBoundingBox() const;

    ///
    /// \brief Apply a tranlation to the vertices
    ///
    void translate(const Eigen::Translation3d &translation);

    ///
    /// \brief Apply a scaling to the vertices
    ///
    void scale(const Eigen::UniformScaling<double> &scaling);

    ///
    /// \brief Apply a rotation to the vertices
    ///
    void rotate(const Quaterniond &rotation);

    ///
    /// \brief Apply any affine tranformation to the vertices.
    ///
    void transform(const Eigen::Transform<double,3,Eigen::Affine> &transformation);

protected:
    // Data arrays - Vertices only
    // vertices co-ordinate data at time t
    std::vector<core::Vec3d> vertices;

    // vertices co-ordinate data at time t=0
    std::vector<core::Vec3d> origVerts;

    // Topology arrays
    std::vector<std::array<size_t,3>> triangleArray;
    std::vector<std::array<size_t,4>> tetrahedraArray;
    std::vector<std::array<size_t,8>> hexahedraArray;

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
