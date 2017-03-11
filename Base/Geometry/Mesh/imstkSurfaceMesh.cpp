/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   =========================================================================*/

#include "imstkSurfaceMesh.h"

namespace imstk
{

void
SurfaceMesh::initialize(const StdVectorOfVec3d& vertices,
                        const std::vector<TriangleArray>& triangles,
                        const bool computeDerivedData)
{
    this->clear();

    Mesh::initialize(vertices);
    setTrianglesVertices(triangles);

    if (computeDerivedData)
    {
        computeVerticesNormals();
    }
}

void
SurfaceMesh::clear()
{
    Mesh::clear();
    m_trianglesVertices.clear();
    m_verticesNeighborTriangles.clear();
    m_verticesNeighborVertices.clear();
    m_trianglesNormals.clear();
    m_verticesNormals.clear();
}

void
SurfaceMesh::print() const
{
    Mesh::print();

    LOG(INFO) << "Number of triangles: " << this->getNumTriangles();
    LOG(INFO) << "Triangles:";
    for (auto &tri : m_trianglesVertices)
    {
        LOG(INFO) << tri.at(0) << ", " << tri.at(1) << ", " << tri.at(2);
    }
}

double
SurfaceMesh::getVolume() const
{
    // TODO
    // 1. Check for water tightness
    // 2. Compute volume based on signed distance

    LOG(WARNING) << "SurfaceMesh::getVolume error: not implemented.";
    return 0.0;
}

void
SurfaceMesh::computeVerticesNeighborTriangles()
{
    m_verticesNeighborTriangles.resize(m_verticesPositions.size());

    size_t triangleId = 0;

    for (const auto& t : m_trianglesVertices)
    {
        m_verticesNeighborTriangles.at(t.at(0)).insert(triangleId);
        m_verticesNeighborTriangles.at(t.at(1)).insert(triangleId);
        m_verticesNeighborTriangles.at(t.at(2)).insert(triangleId);
        triangleId++;
    }
}

void
SurfaceMesh::computeVerticesNeighborVertices()
{
    m_verticesNeighborVertices.resize(m_verticesPositions.size());

    if (m_verticesNeighborTriangles.empty())
    {
        this->computeVerticesNeighborTriangles();
    }

    for (size_t vertexId = 0; vertexId < m_verticesNeighborVertices.size(); ++vertexId)
    {
        for (const size_t& triangleId : m_verticesNeighborTriangles.at(vertexId))
        {
            for (const size_t& vertexId2 : m_trianglesVertices.at(triangleId))
            {
                if (vertexId2 != vertexId)
                {
                    m_verticesNeighborVertices.at(vertexId).insert(vertexId2);
                }
            }
        }
    }
}

void
SurfaceMesh::computeTrianglesNormals()
{
    m_trianglesNormals.resize(m_trianglesVertices.size());

    for (size_t triangleId = 0; triangleId < m_trianglesNormals.size(); ++triangleId)
    {
        const auto& t  = m_trianglesVertices.at(triangleId);
        const auto& p0 = m_verticesPositions.at(t.at(0));
        const auto& p1 = m_verticesPositions.at(t.at(1));
        const auto& p2 = m_verticesPositions.at(t.at(2));

        m_trianglesNormals.at(triangleId) = ((p1 - p0).cross(p2 - p0)).normalized();
    }
}

void
SurfaceMesh::computeVerticesNormals()
{
    m_verticesNormals.resize(m_verticesPositions.size());

    if (m_verticesNeighborTriangles.empty())
    {
        this->computeVerticesNeighborTriangles();
    }

    if (m_trianglesNormals.empty())
    {
        this->computeTrianglesNormals();
    }

    for (size_t vertexId = 0; vertexId < m_verticesNormals.size(); ++vertexId)
    {
        for (const size_t& triangleId : m_verticesNeighborTriangles.at(vertexId))
        {
            m_verticesNormals.at(vertexId) += m_trianglesNormals.at(triangleId);
        }

        m_verticesNormals.at(vertexId).normalize();
    }
}

void
SurfaceMesh::optimizeForDataLocality()
{
    const int numVertices = this->getNumVertices();
    const int numTriangles = this->getNumTriangles();

    // First find the list of triangles a given vertex is part of
    std::vector<std::vector<int>> vertexNeighbors;
    vertexNeighbors.resize(this->getNumVertices());
    int triId = 0;
    for (const auto &tri : this->getTrianglesVertices())
    {
        vertexNeighbors[tri[0]].push_back(triId);
        vertexNeighbors[tri[1]].push_back(triId);
        vertexNeighbors[tri[2]].push_back(triId);

        triId++;
    }

    std::vector<TriangleArray> optimizedConnectivity;
    std::vector<int> optimallyOrderedNodes;
    std::list<int> triUnderConsideration;
    std::vector<bool> isNodeAdded(numVertices, false);
    std::vector<bool> isTriangleAdded(numTriangles, false);
    std::list<int> newlyAddedNodes;

    // A. Initialize
    optimallyOrderedNodes.push_back(0);
    isNodeAdded.at(0) = true;
    for (const auto &neighTriId : vertexNeighbors[0])
    {
        triUnderConsideration.push_back(neighTriId);
    }

    // B. Iterate till all the nodes are added to optimized mesh
    size_t vertId[3];
    auto connectivity = this->getTrianglesVertices();
    while (triUnderConsideration.size() != 0)
    {
        // B.1 Add new nodes and triangles
        for (const auto &triId : triUnderConsideration)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (!isNodeAdded.at(connectivity.at(triId)[i]))
                {
                    optimallyOrderedNodes.push_back(connectivity.at(triId)[i]);
                    isNodeAdded.at(connectivity.at(triId)[i]) = true;
                    newlyAddedNodes.push_back(connectivity.at(triId)[i]);
                }
                vertId[i] = *std::find(optimallyOrderedNodes.begin(),
                                       optimallyOrderedNodes.end(),
                                       connectivity.at(triId)[i]);
            }
            TriangleArray tmpTri = { { vertId[0], vertId[1], vertId[2] } };
            optimizedConnectivity.push_back(tmpTri);
            isTriangleAdded.at(triId) = true;
        }

        // B.2 Setup triangles to be considered for next iteration
        triUnderConsideration.clear();
        for (const auto &newNodes : newlyAddedNodes)
        {
            for (const auto &neighTriId : vertexNeighbors[newNodes])
            {
                if (!isTriangleAdded[neighTriId])
                {
                    triUnderConsideration.push_back(neighTriId);
                }
            }
        }
        triUnderConsideration.sort();
        triUnderConsideration.unique();

        newlyAddedNodes.clear();
    }

    // C. Initialize this mesh with the newly computed ones
    StdVectorOfVec3d optimallyOrderedNodalPos;
    std::vector<TriangleArray> optConnectivityRenumbered;

    // C.1 Get the positions
    for (const auto &nodalId : optimallyOrderedNodes)
    {
        optimallyOrderedNodalPos.push_back(this->getInitialVertexPosition(nodalId));
    }

    // C.2 Get the renumbered connectivity
    for (size_t i = 0; i < numTriangles; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            vertId[j] = (std::find(optimallyOrderedNodes.begin(),
                                   optimallyOrderedNodes.end(),
                                   optimizedConnectivity.at(i)[j]) -
                                   optimallyOrderedNodes.begin()) ;
        }

        TriangleArray tmpTriArray = { { vertId[0], vertId[1], vertId[2] } };
        optConnectivityRenumbered.push_back(tmpTriArray);
    }

    // D. Assign the rewired mesh data to the mesh
    this->initialize(optimallyOrderedNodalPos, optConnectivityRenumbered);
}

const std::vector<SurfaceMesh::TriangleArray>&
SurfaceMesh::getTrianglesVertices() const
{
    return m_trianglesVertices;
}

void
SurfaceMesh::setTrianglesVertices(const std::vector<TriangleArray>& triangles)
{
    m_trianglesVertices = triangles;
}

const StdVectorOfVec3d&
SurfaceMesh::getTrianglesNormals() const
{
    return m_trianglesNormals;
}

const Vec3d&
SurfaceMesh::getTriangleNormal(size_t i) const
{
    return m_trianglesNormals.at(i);
}

void
SurfaceMesh::setVerticesNormals(const StdVectorOfVec3d& normals)
{
    m_verticesNormals = normals;
}

const StdVectorOfVec3d&
SurfaceMesh::getVerticesNormals() const
{
    return m_verticesNormals;
}

void
SurfaceMesh::setVerticesTangents(const StdVectorOfVec3d& tangents)
{
    m_verticesTangents = tangents;
}

const StdVectorOfVec3d&
SurfaceMesh::getVerticesTangents() const
{
    return m_verticesTangents;
}

void
SurfaceMesh::setVerticesBitangents(const StdVectorOfVec3d& bitangents)
{
    m_verticesBitangents = bitangents;
}

const StdVectorOfVec3d&
SurfaceMesh::getVerticesBitangents() const
{
    return m_verticesBitangents;
}

const Vec3d&
SurfaceMesh::getVerticeNormal(size_t i) const
{
    return m_verticesNormals.at(i);
}

int
SurfaceMesh::getNumTriangles() const
{
    return this->m_trianglesVertices.size();
}

void
SurfaceMesh::setDefaultTCoords(std::string arrayName)
{
    m_defaultTCoords = arrayName;
}

std::string
SurfaceMesh::getDefaultTCoords()
{
    return m_defaultTCoords;
}

void
SurfaceMesh::addTexture(std::string tFileName, std::string tCoordsName)
{
    if (tCoordsName == "")
    {
        tCoordsName = m_defaultTCoords;
        if (tCoordsName == "")
        {
            LOG(WARNING) << "Can not add texture without default texture coordinates. ";
            return;
        }
    }

    if (!m_pointDataMap.count(tCoordsName))
    {
        LOG(WARNING) << "Mesh does not hold any array named " << tCoordsName << ". "
                     << "Can not add texture.";
        return;
    }

    m_textureMap[tCoordsName] = tFileName;
}
const std::map<std::string, std::string>&
SurfaceMesh::getTextureMap() const
{
    return m_textureMap;
}

std::string
SurfaceMesh::getTexture(std::string tCoordsName) const
{
    if (!m_textureMap.count(tCoordsName))
    {
        LOG(WARNING) << "No texture filename associated with coordinates " << tCoordsName << ".";
        return "";
    }

    return m_textureMap.at(tCoordsName);
}

} // imstk
