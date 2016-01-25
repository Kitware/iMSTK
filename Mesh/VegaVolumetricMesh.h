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

#ifndef MESH_VEGAMESH_H
#define MESH_VEGAMESH_H

// STD includes
#include <fstream>

// iMSTK includes
#include "Core/BaseMesh.h"

// STL includes
#include <iostream>
#include <memory>
#include <unordered_map>

class Graph;
class SurfaceMesh;
class VolumetricMesh;

//
// Interface to VegaFEM's volumetric mesh class
//
class VegaVolumetricMesh : public Core::BaseMesh
{
public:
    ///
    /// \brief Constructor
    ///
    /// \param generateMeshGraph True if you want to generate a mesh graph. The mesh
    ///     graphs is used by the time stepping method to apply Lagrangian damping.
    ///
    VegaVolumetricMesh(bool generateMeshGraph = true);

    ///
    /// \brief Destructor
    ///
    ~VegaVolumetricMesh();

    ///
    /// \brief Returns graph
    /// Note that this does not check if the graph is valid
    ///
    std::shared_ptr<Graph> getMeshGraph();

    ///
    /// \brief Returns the total number of vertices in the mesh
    ///
    size_t getNumberOfVertices() const;

    ///
    /// \brief Returns the total number of elements in the mesh
    ///
    size_t getNumberOfElements() const;

    ///
    /// \brief Attach surface mesh to the volume mesh and stores interpolation weights
    ///
    void attachSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh,
                           const double &radius = -1.0);

    ///
    /// \brief Get attached surface mesh
    ///
    void attachSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh,
                           const std::string &fileName,
                           const double &radius = 5.0);

    ///
    /// \brief Return underlying Vega mesh data structure.
    ///
    std::shared_ptr<VolumetricMesh> getVegaMesh();

    ///
    /// \brief Sets the Vega underlying mesh.
    ///
    void setVegaMesh(std::shared_ptr<VolumetricMesh> newMesh);

    ///
    /// \brief Update nodes to local arrays
    ///
    void updateAttachedMeshes(const core::Vectord &q);

    ///
    /// \brief Return the vertex map
    ///
    const std::unordered_map<size_t,size_t> &getVertexMap() const;

    ///
    /// \brief Sets the vertex map
    ///
    void setVertexMap(const std::unordered_map<size_t,size_t> &map);

    ///
    /// \brief Set/Get fixed degree of freedom array.
    ///
    void setFixedVertices(const std::vector<size_t> &dofs);
    const std::vector<size_t> &getFixedVertices() const;

    ///
    /// \brief Get fattached surface meshes
    ///
    std::shared_ptr<SurfaceMesh> getAttachedMesh(const size_t i);

    ///
    /// Set render detail for surface meshes.
    ///
    void setRenderDetail(int i, std::shared_ptr<RenderDetail> newRenderDetail);

    ///
    /// \brief Get attached rendering mesh
    ///
    std::shared_ptr<SurfaceMesh> getRenderingMesh();

    ///
    /// \brief Get attached collision mesh
    ///
    std::shared_ptr<SurfaceMesh> getCollisionMesh();

    ///
    /// \brief Returns weigths associated with attached ith surface mesh
    ///
    const std::vector<double> &getAttachedWeights(std::shared_ptr<SurfaceMesh> surfaceMesh) const;

    ///
    /// \brief Return vertices for interpolation weights for ith surface mesh
    ///
    const std::vector<int> &getAttachedVertices(std::shared_ptr<SurfaceMesh> surfaceMesh) const;

    ///
    /// \brief Save interpolation weights for surfaceMesh into filename
    ///
    void saveWeights(std::shared_ptr<SurfaceMesh> surfaceMesh, const std::string &filename) const;

    ///
    /// \brief Read interpolation weights for surfaceMesh
    ///
    void readWeights(std::shared_ptr<SurfaceMesh> surfaceMesh, const std::string &filename, const double radius = 1.0);

    ///
    /// \brief Generate interpolation weights for surfaceMesh and volume mesh
    ///
    void generateWeigths(std::shared_ptr<SurfaceMesh> surfaceMesh,
                         double radius = 1.0,
                         const bool saveToDisk = false,
                         const std::string &filename = "mesh.interp");

    ///
    /// \brief Apply a tranlation to all the vertices, including attached meshes.
    ///
    void translate(const Eigen::Translation3d &translation, bool setInitialPoints = false);

    ///
    /// \brief Interpolate the displacements on to the given mesh.
    ///
    /// \param x Displacements.
    /// \param mesh Mesh where the interpolation will be computed.
    ///
    void interpolate(const core::Vectord &x,
                     std::shared_ptr< SurfaceMesh > mesh);

    ///
    /// \brief Compute gravity force
    ///
    void computeGravity(const core::Vec3d &gravity, core::Vectord &gravityForce);

private:
    // Vega mesh base object
    std::shared_ptr<VolumetricMesh> mesh;

    // Vega mesh graph. The mesh graphs is used by the time stepping method to
    // apply Lagrangian damping.
    std::shared_ptr<Graph> meshGraph;

    // Generate graph for this mesh
    bool generateGraph;

    // Store pointer to the surface meshes
    std::vector<std::shared_ptr<SurfaceMesh>> attachedMeshes;

    // Store map of vertices
    std::map<std::shared_ptr<SurfaceMesh>,std::vector<int>> attachedVertices;

    // Store map of  weigths
    std::map<std::shared_ptr<SurfaceMesh>,std::vector<double>> attachedWeights;

    // Store map of conforming surface vertex indices
    std::unordered_map<size_t,size_t> vertexMap;

    // Fixed DOFs
    std::vector<size_t> fixedVertices;

};


#endif // SMVEGAMESH_H
