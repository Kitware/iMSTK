// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#ifndef BASEMESH_H
#define BASEMESH_H

// STD includes
#include <array>

// Eigen includes
#include <Eigen/Geometry>

#include "Core/CoreClass.h"
#include "Core/Vector.h"
#include "Core/Quaternion.h"

namespace Core {

///
/// \brief Base class for all meshes used in the simulation.
///
/// \see SurfaceMesh VegaVolumetricMesh
///
class BaseMesh: public CoreClass
{
public:
    using TransformType = Eigen::Transform<double, 3, Eigen::Affine>;

public:
    ///
    /// \brief designates what purpose/scenario the mesh is used for
    ///
    enum class MeshType
    {
        Surface,
        Volume,
        Rigid,
        Deformable,
        DeformableCutable,
        RigidCutable
    };

    ///
    /// \brief constructor/Destructor
    ///
    BaseMesh();
    virtual ~BaseMesh();

    ///
    /// \brief Copy vertices into origVerts
    ///
    void updateInitialVertices();

    ///
    /// \brief Copy vertices into origVerts
    ///
    void resetVertices();

    ///
    /// \brief Accessors for vertex coordinates
    ///
    const std::vector<core::Vec3d> &getVertices() const;
    std::vector<core::Vec3d> &getVertices();
    void setVertices(const std::vector<core::Vec3d> &vertices);

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
    /// \brief Set the rendering id
    ///
    void setRenderingId(size_t id);

    ///
    /// \brief Set the rendering id
    ///
    size_t getRenderingId() const;

    ///
    /// \brief Apply a tranlation to the vertices
    ///
    virtual void translate(const Eigen::Translation3d &translation, bool setInitialPoints = false);

    ///
    /// \brief Apply a scaling to the vertices
    ///
    void scale(const Eigen::UniformScaling<double> &scaling);

    ///
    /// \brief Apply a rotation to the vertices
    ///
    void rotate(const core::Quaterniond &rotation);

    ///
    /// \brief Apply any affine tranformation to the vertices.
    ///
    void transform(const TransformType &transformation);

    ///
    /// \brief Apply any affine tranformation to the vertices.
    ///
    std::array<core::Vec3d,3> getTriangleVertices(size_t i) const;

	int getMeshType();
	void setMeshType(int);

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

    // Bounding Box
    Eigen::AlignedBox3d aabb;

    // Render identification, only used in \StylusRenderDelegate.
    // TODO: Remove this
    size_t renderingID;

	///mesh type set by the loader
	int  meshType;
};

}// namespace Core
#endif //
