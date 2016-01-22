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

#ifndef IO_MESHDELEGATE_H
#define IO_MESHDELEGATE_H

#include <cstdint>
#include <memory>

#include "IO/IOMesh.h"
#include "Core/Config.h"
#include "Mesh/SurfaceMesh.h"
#include "Mesh/VegaVolumetricMesh.h"

// VegaFEM includes
#include "tetMesh.h"
#include "cubicMesh.h"
#include "generateSurfaceMesh.h"
#include "objMesh.h"

///
/// \brief Base class for mesh IO delegates.
///
class IOMeshDelegate
{
public:
    typedef std::shared_ptr<IOMeshDelegate> Ptr;

public:
    IOMeshDelegate();
    void setSource(IOMesh *src);

    ///
    /// \brief Performs the actual reading of the mesh.
    ///     It populates the local mesh datastructure.
    ///
    virtual void read();

    ///
    /// \brief Write mesh.
    ///  You can use one of the boundled writers or any customized one.
    ///
    virtual void write();

    /// \brief Mesh type and property variable
    enum MeshType
    {
        Unknown         = 0,
        Tri             = 1,
        Tetra           = 1 << 1,
        Hexa            = 1 << 2,
        hasMaterials    = 1 << 3,
        hasBDConditions = 1 << 4,
        hasDensity      = 1 << 4,
        hasPoisson      = 1 << 5,
        hasYoung        = 1 << 6,
        hasTcoords      = 1 << 7

    };

    // Used to know what type of mesh is being read.
    unsigned int meshProps;

protected:
    ///
    /// \brief Creates a new surface mesh with vertices and triangleArray and stores
    /// it in the meshIO.
    ///
    void setSurfaceMesh(const std::vector<core::Vec3d> &vertices,
                        const std::vector<std::array<size_t,3>> &triangleArray);

    ///
    /// \brief Creates a new vega volumetric mesh with vertices and tetraArray and stores
    /// it in the meshIO. Uses the vega mesh constructor that takes boundary conditions
    /// and material properties.
    ///
    void setVegaTetraMesh(const std::vector<core::Vec3d> &vertices,
                          const std::vector<std::array<size_t,4>> &tetraArray,
                          const std::vector<size_t> &bdConditions,
                          const core::Vec3d &material);

    ///
    /// \brief Creates a new vega volumetric mesh with vertices and tetraArray and stores
    /// it in the meshIO.
    ///
    void setVegaTetraMesh(const std::vector<core::Vec3d> &vertices,
                          const std::vector<std::array<size_t,4>> &tetraArray);

    ///
    /// \brief Creates a new vega volumetric mesh with vertices and hexaArray and stores
    /// it in the meshIO. Uses the vega mesh constructor that takes boundary conditions
    /// and material properties.
    ///
    void setVegaHexaMesh(const std::vector<core::Vec3d> &vertices,
                         const std::vector<std::array<size_t,8>> &hexaArray,
                         const std::vector<size_t> &bdConditions,
                         const std::array<double,3> &material);

    ///
    /// \brief Creates a new vega volumetric mesh with vertices and hexaArray and stores
    /// it in the meshIO.
    /// \param vertices Vertex array
    /// \param hexaArray Hexahedron array
    ///
    void setVegaHexaMesh(const std::vector<core::Vec3d> &vertices,
                         const std::vector<std::array<size_t,8>> &hexaArray);

    ///
    /// \brief Utility function to help extract a surface mesh from a volume mesh.
    /// \param triangleArray Triangle array of the surface of the volumetric mesh. This
    ///     array will be sorted and updated with new indices.
    /// \param vertices Entire volumetric mesh vertex array.
    /// \param surfaceVertices Ouput array containing the vertex array for the surface
    ///     mesh.
    /// \param uniqueVertexArray Output map of the vertices. Maps indices from the
    ///     surfaceVertices to vertices.
    ///
    void reorderSurfaceTopology(const std::vector<core::Vec3d> &vertices,
                                std::vector<core::Vec3d> &surfaceVertices,
                                std::vector<std::array<size_t,3>> &triangleArray,
                                std::unordered_map<size_t,size_t> &uniqueVertexArray
                               );

protected:
    IOMesh *meshIO;
};
#endif // READERDELEGATE_H
