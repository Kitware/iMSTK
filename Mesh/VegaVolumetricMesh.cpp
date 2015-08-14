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

#include "Mesh/VegaVolumetricMesh.h"

// SimMedTK includes
#include "Mesh/SurfaceMesh.h"

// VEGA includes
#include "volumetricMesh.h"
#include "generateMeshGraph.h"
#include "cubicMesh.h"
#include "tetMesh.h"
#include "graph.h"
#include "vec3d.h"

VegaVolumetricMesh::VegaVolumetricMesh(bool generateMeshGraph) : generateGraph(generateMeshGraph) {}
VegaVolumetricMesh::~VegaVolumetricMesh() {}
std::shared_ptr<Graph> VegaVolumetricMesh::getMeshGraph()
{
    return this->meshGraph;
}
size_t VegaVolumetricMesh::getNumberOfVertices() const
{
    return this->mesh->getNumVertices();
}
size_t VegaVolumetricMesh::getNumberOfElements() const
{
    return this->mesh->getNumElements();
}
void VegaVolumetricMesh::attachSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh, const double &radius)
{
    const std::vector<core::Vec3d> &meshVertices = surfaceMesh->getVertices();

    int numElementVertices = this->mesh->getNumElementVertices();
    size_t surfaceMeshSize = meshVertices.size();

    // Allocate arrays
    this->attachedMeshes.push_back(surfaceMesh);
    std::vector<int> &vertices = this->attachedVertices[surfaceMesh];
    std::vector<double> &weigths = this->attachedWeights[surfaceMesh];

    std::vector<double> baryCentricWeights(numElementVertices);

    vertices.resize(numElementVertices * surfaceMeshSize);
    weigths.resize(numElementVertices * surfaceMeshSize);

    for(size_t i = 0; i <surfaceMeshSize; ++i)
    {
        Vec3d vegaPosition(meshVertices[i][0], meshVertices[i][1], meshVertices[i][2]);
        int element = this->mesh->getContainingElement(vegaPosition);

        if(element < 0)
        {
            std::cerr <<"Containing element not found for: " << meshVertices[i] <<std::endl;
            continue;
        }

        this->mesh->computeBarycentricWeights(element, vegaPosition, baryCentricWeights.data());

        if(radius> 0)
        {
            double minDistance = std::numeric_limits<double>::max();
            for(int k = 0; k < numElementVertices; ++k)
            {
                Vec3d &p = *this->mesh->getVertex(element, k);
                double l = len(p - vegaPosition);
                if(l <minDistance)
                {
                    minDistance = l;
                }
            }
            if(minDistance > radius)
            {
                for(int k = 0; k < numElementVertices; ++k)
                {
                    baryCentricWeights[k] = 0.0;
                }
                continue;
            }
        }
        for(int k = 0; k < numElementVertices; ++k)
        {
            vertices[numElementVertices * i + k] = this->mesh->getVertexIndex(element, k);
            weigths[numElementVertices * i + k] = baryCentricWeights[k];
        }
    }
}
const std::vector<double> &VegaVolumetricMesh::getAttachedWeights(const size_t &i) const
{
    return this->attachedWeights.at(attachedMeshes[i]);
}
const std::vector<int> &VegaVolumetricMesh::getAttachedVertices(const size_t &i) const
{
    return this->attachedVertices.at(attachedMeshes[i]);
}
std::shared_ptr<VolumetricMesh> VegaVolumetricMesh::getVegaMesh()
{
    return this->mesh;
}
void VegaVolumetricMesh::setVegaMesh(std::shared_ptr<VolumetricMesh> newMesh)
{
    this->mesh = newMesh;

    if(nullptr != this->mesh && generateGraph)
    {
        meshGraph = std::make_shared<Graph>(*GenerateMeshGraph::Generate(this->mesh.get()));
    }
}
void VegaVolumetricMesh::updateAttachedMeshes(double *q)
{
    for(auto surfaceMesh : this->attachedMeshes)
    {
        std::vector<core::Vec3d> displacements(surfaceMesh->getNumberOfVertices());
        VolumetricMesh::interpolate(q,
                                    displacements.data()->data(),
                                    surfaceMesh->getNumberOfVertices(),
                                    this->mesh->getNumElementVertices(),
                                    this->attachedVertices.at(surfaceMesh).data(),
                                    this->attachedWeights.at(surfaceMesh).data());
        surfaceMesh->resetVertices();
        auto &vertices = surfaceMesh->getVertices();
        for(size_t i = 0, end = displacements.size(); i < end; ++i)
        {
            vertices[i] += displacements[i];
        }
        surfaceMesh->computeTriangleNormals();
    }
}
const std::unordered_map<size_t,size_t>& VegaVolumetricMesh::getVertexMap() const
{
    return this->vertexMap;
}
void VegaVolumetricMesh::setVertexMap(const std::unordered_map<size_t,size_t>& map)
{
    this->vertexMap = map;
}
