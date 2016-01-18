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

//---------------------------------------------------------------------------
VegaVolumetricMesh::~VegaVolumetricMesh() {}

//---------------------------------------------------------------------------
std::shared_ptr<Graph> VegaVolumetricMesh::getMeshGraph()
{
    return this->meshGraph;
}

//---------------------------------------------------------------------------
size_t VegaVolumetricMesh::getNumberOfVertices() const
{
    return this->mesh->getNumVertices();
}

//---------------------------------------------------------------------------
size_t VegaVolumetricMesh::getNumberOfElements() const
{
    return this->mesh->getNumElements();
}

//---------------------------------------------------------------------------
void VegaVolumetricMesh::attachSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh,
                                           const double &radius,
                                           bool useForRendering)
{
    // Keep copy of the mesh pointer
    this->attachedMeshes.push_back(surfaceMesh);

    // The first attached mesh is always the conforming mesh
    if(this->attachedMeshes.size() == 1)
    {
        return;
    }
    this->generateWeigths(surfaceMesh,radius);

    // If this surface mesh is the rendering mesh then pass along its delegate to this
    // volumetric mesh.
    if(useForRendering)
    {
        this->setRenderDelegate(surfaceMesh->getRenderDelegate());
    }
}

//---------------------------------------------------------------------------
void VegaVolumetricMesh::attachSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh,
                                           const std::string& fileName,
                                           const double &radius,
                                           bool useForRendering
                                          )
{
    // Keep copy of the mesh pointer
    this->attachedMeshes.push_back(surfaceMesh);

    // The first attached mesh is always the conforming mesh
    if(this->attachedMeshes.size() == 1)
    {
        return;
    }
    this->readWeights(surfaceMesh,fileName,radius);

    // If this surface mesh is the rendering mesh then pass along its delegate to this
    // volumetric mesh.
    if(useForRendering)
    {
        this->setRenderDelegate(surfaceMesh->getRenderDelegate());
    }
}

//---------------------------------------------------------------------------
std::shared_ptr<VolumetricMesh> VegaVolumetricMesh::getVegaMesh()
{
    return this->mesh;
}

//---------------------------------------------------------------------------
void VegaVolumetricMesh::setVegaMesh(std::shared_ptr<VolumetricMesh> newMesh)
{
    this->mesh = newMesh;

    if(nullptr != this->mesh && this->generateGraph)
    {
        meshGraph.reset();
        meshGraph = std::make_shared<Graph>(*GenerateMeshGraph::Generate(this->mesh.get()));
    }
}

//---------------------------------------------------------------------------
void VegaVolumetricMesh::interpolate(const core::Vectord &x,
                                     std::shared_ptr< SurfaceMesh > mesh)
{
    if(size_t(x.size()) != 3*this->getNumberOfVertices())
    {
        // TODO: Log this
        return;
    }

    auto verticesPerElement = size_t(this->mesh->getNumElementVertices());

    // Create maps for the interpolation data.
    auto vertices = core::Matrix<int>::Map(this->attachedVertices.at(mesh).data(),
                                           verticesPerElement,
                                           mesh->getNumberOfVertices());

    auto weights = core::Matrix<double>::Map(this->attachedWeights.at(mesh).data(),
                                             verticesPerElement,
                                             mesh->getNumberOfVertices());

    // Create maps to the mesh data. Each column represents a node on the mesh.
    auto displacements = core::Matrix<double>::Map(x.data(),3,this->getNumberOfVertices());
    auto &targets = mesh->getVertices();
    const auto &initialTargets = mesh->getOrigVertices();

    if(targets.size() != size_t(vertices.cols()) ||
       targets.size() != size_t(weights.cols()))
    {
        // TODO: Log this error
        return;
    }

    std::vector<core::Vec3d> interpolatedDisplacement(mesh->getNumberOfVertices(),
                                                     core::Vec3d::Zero());

    for(size_t i = 0, end = targets.size(); i < end; ++i)
    {
        auto vertexIndices = vertices.col(i);
        auto vertexWeights = weights.col(i);
        for(size_t j = 0; j < verticesPerElement; ++j)
        {
            auto idx = vertexIndices(j);
            interpolatedDisplacement[i] += displacements.col(idx) * vertexWeights(j);
        }
    }

    for(size_t i = 0, end = targets.size(); i < end; ++i)
    {
        targets[i] = initialTargets[i] + interpolatedDisplacement[i];
    }
}

//---------------------------------------------------------------------------
void VegaVolumetricMesh::updateAttachedMeshes(const core::Vectord &x)
{
    auto renderingMesh = this->getRenderingMesh();
    if(renderingMesh)
    {
        this->interpolate(x,renderingMesh);
        renderingMesh->computeTriangleNormals();
        renderingMesh->getRenderDelegate()->modified();
    }

    auto collisionMesh = this->getCollisionMesh();
    if(collisionMesh)
    {
        auto displacementMap = core::Matrix<double>::Map(x.data(),
                                                         3,this->mesh->getNumVertices());
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

//---------------------------------------------------------------------------
const std::unordered_map<size_t,size_t>& VegaVolumetricMesh::getVertexMap() const
{
    return this->vertexMap;
}

//---------------------------------------------------------------------------
void VegaVolumetricMesh::setVertexMap(const std::unordered_map<size_t,size_t>& map)
{
    this->vertexMap = map;
}

//---------------------------------------------------------------------------
const std::vector< size_t >& VegaVolumetricMesh::getFixedVertices() const
{
    return this->fixedVertices;
}

//---------------------------------------------------------------------------
void VegaVolumetricMesh::setFixedVertices(const std::vector< size_t >& dofs)
{
    this->fixedVertices.clear();
    this->fixedVertices = dofs;
}

//---------------------------------------------------------------------------
std::shared_ptr<SurfaceMesh> VegaVolumetricMesh::getAttachedMesh(const size_t i)
{
    return i < this->attachedMeshes.size() ?
            this->attachedMeshes.at(i) :
            nullptr;
}

//---------------------------------------------------------------------------
void VegaVolumetricMesh::
setRenderDetail(int i, std::shared_ptr< RenderDetail > newRenderDetail)
{
    this->attachedMeshes.at(i)->setRenderDetail(newRenderDetail);
}

//---------------------------------------------------------------------------
std::shared_ptr<SurfaceMesh> VegaVolumetricMesh::getRenderingMesh()
{
    return attachedMeshes.size() > 1
           ? this->attachedMeshes.at(attachedMeshes.size() - 1)
           : nullptr;
}

//---------------------------------------------------------------------------
std::shared_ptr<SurfaceMesh> VegaVolumetricMesh::getCollisionMesh()
{
    return attachedMeshes.size() > 0
           ? this->attachedMeshes.at(0)
           : nullptr;
}

//---------------------------------------------------------------------------
void VegaVolumetricMesh::
saveWeights(std::shared_ptr<SurfaceMesh> surfaceMesh, const std::string& filename) const
{
    auto vertices = this->attachedVertices.at(surfaceMesh);
    auto weights = this->attachedWeights.at(surfaceMesh);

    int verticesPerElement = this->mesh->getNumElementVertices();
    std::ofstream fileStream(filename.c_str(), std::ofstream::out);

    for(size_t i = 0, end = weights.size() / verticesPerElement; i < end; ++i)
    {
        auto index = i * verticesPerElement;
        fileStream << i
                   << " " << vertices[index] << " " << weights[index]
                   << " " << vertices[index + 1] << " " << weights[index + 1]
                   << " " << vertices[index + 2] << " " << weights[index + 2]
                   << " " << vertices[index + 3] << " " << weights[index + 3] << std::endl;
    }

    fileStream << std::endl;
    fileStream.close();
}

//---------------------------------------------------------------------------
void VegaVolumetricMesh::
readWeights(std::shared_ptr<SurfaceMesh> surfaceMesh,
            const std::string& filename,
            const double radius)
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

    auto verticesPerElement = this->mesh->getNumElementVertices();

    int index;
    int v[verticesPerElement];
    double w[verticesPerElement];

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

//---------------------------------------------------------------------------
void VegaVolumetricMesh::
generateWeigths(std::shared_ptr<SurfaceMesh> surfaceMesh,
                double radius,
                const bool saveToDisk,
                const std::string& filename)
{
    std::cerr << "Generating weights..." << std::endl;
    const std::vector<core::Vec3d> &meshVertices = surfaceMesh->getVertices();

    int verticesPerElement = this->mesh->getNumElementVertices();
    size_t surfaceMeshSize = meshVertices.size();

    std::vector<int> &vertices = this->attachedVertices[surfaceMesh];
    std::vector<double> &weigths = this->attachedWeights[surfaceMesh];

    std::vector<double> baryCentricWeights(verticesPerElement);

    vertices.resize(verticesPerElement * surfaceMeshSize);
    weigths.resize(verticesPerElement * surfaceMeshSize);

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

            for(int k = 0; k < verticesPerElement; ++k)
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
                for(int k = 0; k < verticesPerElement; ++k)
                {
                    baryCentricWeights[k] = 0.0;
                }

                continue;
            }
        }

        for(int k = 0; k < verticesPerElement; ++k)
        {
            vertices[verticesPerElement * i + k] = this->mesh->getVertexIndex(element, k);
            weigths[verticesPerElement * i + k] = baryCentricWeights[k];
        }
    }

    if(saveToDisk)
    {
        this->saveWeights(surfaceMesh, filename);
    }
}

//---------------------------------------------------------------------------
const std::vector< double >& VegaVolumetricMesh::
getAttachedWeights(std::shared_ptr<SurfaceMesh> surfaceMesh) const
{
    return this->attachedWeights.at(surfaceMesh);
}

//---------------------------------------------------------------------------
const std::vector< int >& VegaVolumetricMesh::
getAttachedVertices(std::shared_ptr<SurfaceMesh> surfaceMesh) const
{
    return this->attachedVertices.at(surfaceMesh);
}

//---------------------------------------------------------------------------
void VegaVolumetricMesh::
translate(const Eigen::Translation3d& translation, bool setInitialPoints)
{
    for(auto meshes : this->attachedMeshes)
    {
        meshes->translate(translation, setInitialPoints);
    }

}

//---------------------------------------------------------------------------
void VegaVolumetricMesh::computeGravity(const core::Vec3d& gravity,
                                        core::Vectord& gravityForce)
{
    auto verticesPerElement = this->mesh->getNumElementVertices();
    auto invVerticesPerElement = 1.0/verticesPerElement;

    auto mg = core::Matrixd::Map(gravityForce.data(),3,this->getNumberOfVertices());

    for(size_t e = 0; e < this->getNumberOfElements(); ++e)
    {
        double mass = invVerticesPerElement * this->mesh->getElementDensity(e) *
            this->mesh->getElementVolume(e);
        for(int j = 0; j < verticesPerElement; ++j)
            mg.col(this->mesh->getVertexIndex(e,j)) += mass * gravity;
    }
}
