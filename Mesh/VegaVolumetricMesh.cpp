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
#include "Core/RenderDelegate.h"

// VEGA includes
#include "volumetricMesh.h"
#include "generateMeshGraph.h"
#include "cubicMesh.h"
#include "tetMesh.h"
#include "graph.h"
#include "vec3d.h"

VegaVolumetricMesh::VegaVolumetricMesh(bool generateMeshGraph) : generateGraph(generateMeshGraph)
{
}
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
    // Keep copy of the mesh pointer
    this->attachedMeshes.push_back(surfaceMesh);

    // The first attached mesh is always the conforming mesh
    if(this->attachedMeshes.size() == 1)
    {
        return;
    }
    this->generateWeigths(surfaceMesh,radius);
}
void VegaVolumetricMesh::attachSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh, const std::string& fileName, const double &radius)
{
    // Keep copy of the mesh pointer
    this->attachedMeshes.push_back(surfaceMesh);

    // The first attached mesh is always the conforming mesh
    if(this->attachedMeshes.size() == 1)
    {
        return;
    }
    this->readWeights(surfaceMesh,fileName,radius);
}
std::shared_ptr<VolumetricMesh> VegaVolumetricMesh::getVegaMesh()
{
    return this->mesh;
}
void VegaVolumetricMesh::setVegaMesh(std::shared_ptr<VolumetricMesh> newMesh)
{
    this->mesh = newMesh;

    if(nullptr != this->mesh && this->generateGraph)
    {
        meshGraph.reset();
        meshGraph = std::make_shared<Graph>(*GenerateMeshGraph::Generate(this->mesh.get()));
    }
}
void VegaVolumetricMesh::updateAttachedMeshes(double *q)
{
    auto renderingMesh = this->getRenderingMesh();
    if(renderingMesh)
    {
        std::vector<core::Vec3d> displacements(renderingMesh->getNumberOfVertices(),core::Vec3d::Zero());
        VolumetricMesh::interpolate(q,
                                    displacements.data()->data(),
                                    renderingMesh->getNumberOfVertices(),
                                    this->mesh->getNumElementVertices(),
                                    this->attachedVertices.at(renderingMesh).data(),
                                    this->attachedWeights.at(renderingMesh).data());


        auto &restPositions = renderingMesh->getOrigVertices();
        if(restPositions.size() != displacements.size())
        {
            std::cerr << "Error: rest positions not set!" << std::endl;
            return;
        }

        auto &vertices = renderingMesh->getVertices();
        for(size_t i = 0, end = displacements.size(); i < end; ++i)
        {
            vertices[i] = restPositions[i] + displacements[i];
        }
        renderingMesh->computeTriangleNormals();
        renderingMesh->getRenderDelegate()->modified();

    }

    auto collisionMesh = this->getCollisionMesh();
    if(collisionMesh)
    {
        core::Matrix3MapType<double> displacementMap(q,3,this->mesh->getNumVertices());
        auto &restPositions = collisionMesh->getOrigVertices();
        auto &vertices = collisionMesh->getVertices();

        if(restPositions.size() != vertices.size())
        {
            std::cerr << "Error: rest positions not set!" << std::endl;
            return;
        }

        for(size_t i = 0, end = collisionMesh->getNumberOfVertices(); i < end; ++i)
        {
            vertices[i] = restPositions[i] + displacementMap.col(vertexMap[i]);
        }
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
const std::vector< size_t >& VegaVolumetricMesh::getFixedVertices() const
{
    return this->fixedVertices;
}
void VegaVolumetricMesh::setFixedVertices(const std::vector< size_t >& dofs)
{
    this->fixedVertices.clear();
    this->fixedVertices = dofs;
}
std::shared_ptr<SurfaceMesh> VegaVolumetricMesh::getAttachedMesh(const size_t i)
{
    return i < this->attachedMeshes.size() ?
            this->attachedMeshes.at(i) :
            nullptr;
}
void VegaVolumetricMesh::setRenderDetail(int i, std::shared_ptr< RenderDetail > newRenderDetail)
{
    this->attachedMeshes.at(i)->setRenderDetail(newRenderDetail);
}
std::shared_ptr<SurfaceMesh> VegaVolumetricMesh::getRenderingMesh()
{
    return attachedMeshes.size() > 1
           ? this->attachedMeshes.at(attachedMeshes.size() - 1)
           : nullptr;
}
std::shared_ptr<SurfaceMesh> VegaVolumetricMesh::getCollisionMesh()
{
    return attachedMeshes.size() > 0
           ? this->attachedMeshes.at(0)
           : nullptr;
}
void VegaVolumetricMesh::saveWeights(std::shared_ptr<SurfaceMesh> surfaceMesh, const std::string& filename) const
{
    auto vertices = this->attachedVertices.at(surfaceMesh);
    auto weights = this->attachedWeights.at(surfaceMesh);

    int numElementVertices = this->mesh->getNumElementVertices();
    std::ofstream fileStream(filename.c_str(), std::ofstream::out);

    for(size_t i = 0, end = weights.size() / numElementVertices; i < end; ++i)
    {
        auto index = i * numElementVertices;
        fileStream << i
                   << " " << vertices[index] << " " << weights[index]
                   << " " << vertices[index + 1] << " " << weights[index + 1]
                   << " " << vertices[index + 2] << " " << weights[index + 2]
                   << " " << vertices[index + 3] << " " << weights[index + 3] << std::endl;
    }

    fileStream << std::endl;
    fileStream.close();
}
void VegaVolumetricMesh::readWeights(std::shared_ptr<SurfaceMesh> surfaceMesh, const std::string& filename, const double radius)
{
    std::ifstream fileStream(filename.c_str());

    if(!fileStream)
    {
        std::cerr << "Unable to open weigths file: " << filename << std::endl;
        this->generateWeigths(surfaceMesh, radius, true, filename);
        return;
    }

    std::cout << "Reading rendering mesh weights from: " << filename << std::endl;

    std::vector<int> &vertices = this->attachedVertices[surfaceMesh];
    std::vector<double> &weigths = this->attachedWeights[surfaceMesh];
    vertices.clear();
    weigths.clear();

    int index;
    std::array<int, 4> v;
    std::array<double, 4> w;

    while(fileStream >> index
            >> v[0] >> w[0]
            >> v[1] >> w[1]
            >> v[2] >> w[2]
            >> v[3] >> w[3])
    {
        vertices.push_back(v[0]);
        weigths.push_back(w[0]);
        vertices.push_back(v[1]);
        weigths.push_back(w[1]);
        vertices.push_back(v[2]);
        weigths.push_back(w[2]);
        vertices.push_back(v[3]);
        weigths.push_back(w[3]);
    }

    std::cout << "\tTotal # of weights read: " << weigths.size() / this->mesh->getNumElementVertices() << std::endl;
}
void VegaVolumetricMesh::generateWeigths(std::shared_ptr<SurfaceMesh> surfaceMesh, double radius, const bool saveToDisk, const std::string& filename)
{
    std::cerr << "Generating weights..." << std::endl;
    const std::vector<core::Vec3d> &meshVertices = surfaceMesh->getVertices();

    int numElementVertices = this->mesh->getNumElementVertices();
    size_t surfaceMeshSize = meshVertices.size();

    std::vector<int> &vertices = this->attachedVertices[surfaceMesh];
    std::vector<double> &weigths = this->attachedWeights[surfaceMesh];

    std::vector<double> baryCentricWeights(numElementVertices);

    vertices.resize(numElementVertices * surfaceMeshSize);
    weigths.resize(numElementVertices * surfaceMeshSize);

    for(size_t i = 0; i < surfaceMeshSize; ++i)
    {
        Vec3d vegaPosition(meshVertices[i][0], meshVertices[i][1], meshVertices[i][2]);
        int element = this->mesh->getContainingElement(vegaPosition);

        if(element < 0)
        {
            element = this->mesh->getClosestElement(vegaPosition);
        }

        this->mesh->computeBarycentricWeights(element, vegaPosition, baryCentricWeights.data());

        if(radius > 0)
        {
            double minDistance = std::numeric_limits<double>::max();

            for(int k = 0; k < numElementVertices; ++k)
            {
                Vec3d &p = *this->mesh->getVertex(element, k);
                double l = len(p - vegaPosition);

                if(l < minDistance)
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

    if(saveToDisk)
    {
        this->saveWeights(surfaceMesh, filename);
    }
}
const std::vector< double >& VegaVolumetricMesh::getAttachedWeights(std::shared_ptr<SurfaceMesh> surfaceMesh) const
{
    return this->attachedWeights.at(surfaceMesh);
}
const std::vector< int >& VegaVolumetricMesh::getAttachedVertices(std::shared_ptr<SurfaceMesh> surfaceMesh) const
{
    return this->attachedVertices.at(surfaceMesh);
}
void VegaVolumetricMesh::translate(const Eigen::Translation3d& translation, bool setInitialPoints)
{
    for(auto meshes : this->attachedMeshes)
    {
        meshes->translate(translation, setInitialPoints);
    }

}
