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
#include "imstkLogger.h"
#include "imstkVecDataArray.h"
#include "imstkGeometryUtilities.h"

namespace imstk
{
SurfaceMesh::SurfaceMesh(const std::string& name) : PointSet(name),
    m_triangleIndices(std::make_shared<VecDataArray<int, 3>>())
{
}

void
SurfaceMesh::initialize(std::shared_ptr<VecDataArray<double, 3>> vertices,
                        std::shared_ptr<VecDataArray<int, 3>> triangleIndices,
                        const bool computeDerivedData)
{
    this->clear();

    PointSet::initialize(vertices);
    this->setTriangleIndices(triangleIndices);

    if (computeDerivedData)
    {
        this->computeVertexNeighborTriangles();
        //this->computeUVSeamVertexGroups();

        this->computeVertexNormals();
        this->computeVertexTangents();
    }
}

void
SurfaceMesh::initialize(std::shared_ptr<VecDataArray<double, 3>> vertices,
                        std::shared_ptr<VecDataArray<int, 3>> triangleIndices,
                        std::shared_ptr<VecDataArray<double, 3>> normals,
                        const bool computeDerivedData)
{
    this->initialize(vertices, triangleIndices, computeDerivedData);

    setVertexAttribute("normals", normals);

    if (computeDerivedData)
    {
        this->computeVertexNeighborTriangles();
        this->computeUVSeamVertexGroups();
        this->computeVertexNormals();
        this->computeVertexTangents();
    }
}

void
SurfaceMesh::clear()
{
    PointSet::clear();
    if (m_triangleIndices != nullptr)
    {
        m_triangleIndices->clear();
    }
    m_vertexNeighborTriangles.clear();
    m_vertexNeighborVertices.clear();
    for (auto i : m_cellAttributes)
    {
        i.second->clear();
    }
}

void
SurfaceMesh::print() const
{
    PointSet::print();

    LOG(INFO) << "Number of triangles: " << this->getNumTriangles();
    LOG(INFO) << "Triangles:";
    for (auto& tri : *m_triangleIndices)
    {
        LOG(INFO) << tri[0] << ", " << tri[1] << ", " << tri[2];
    }
}

double
SurfaceMesh::getVolume()
{
    // Hack to make shared_ptr of this, doesn't delete this
    std::shared_ptr<SurfaceMesh> surfMesh(this, [](SurfaceMesh*) {});
    if (GeometryUtils::isClosed(surfMesh))
    {
        return GeometryUtils::getVolume(surfMesh);
    }
    else
    {
        LOG(WARNING) << "SurfaceMesh not closed";
        return 0.0;
    }
}

void
SurfaceMesh::computeVertexNeighborTriangles()
{
    m_vertexNeighborTriangles.clear();
    m_vertexNeighborTriangles.resize(m_vertexPositions->size());

    size_t triangleId = 0;

    for (const auto& t : *m_triangleIndices)
    {
        m_vertexNeighborTriangles.at(t[0]).insert(triangleId);
        m_vertexNeighborTriangles.at(t[1]).insert(triangleId);
        m_vertexNeighborTriangles.at(t[2]).insert(triangleId);
        triangleId++;
    }
}

void
SurfaceMesh::computeVertexNeighborVertices()
{
    m_vertexNeighborVertices.clear();
    m_vertexNeighborVertices.resize(m_vertexPositions->size());

    if (m_vertexNeighborTriangles.size() != m_vertexPositions->size())
    {
        this->computeVertexNeighborTriangles();
    }

    // For every vertex
    const VecDataArray<int, 3>& indices = *m_triangleIndices;
    for (size_t vertexId = 0; vertexId < m_vertexNeighborVertices.size(); ++vertexId)
    {
        // For every other vertex
        for (const size_t& triangleId : m_vertexNeighborTriangles.at(vertexId))
        {
            for (int i = 0; i < 3; i++)
            {
                const size_t& vertexId2 = indices[triangleId][i];
                if (vertexId2 != vertexId)
                {
                    m_vertexNeighborVertices.at(vertexId).insert(vertexId2);
                }
            }
        }
    }
}

void
SurfaceMesh::computeTrianglesNormals()
{
    // Avoid reallocating if same size
    std::shared_ptr<VecDataArray<double, 3>> triangleNormalsPtr = getCellNormals();
    if (triangleNormalsPtr == nullptr)
    {
        triangleNormalsPtr = std::make_shared<VecDataArray<double, 3>>(m_triangleIndices->size());
    }
    else
    {
        if (m_triangleIndices->size() != triangleNormalsPtr->size())
        {
            triangleNormalsPtr->resize(static_cast<int>(m_triangleIndices->size()));
        }
    }
    VecDataArray<double, 3>& triangleNormals = *triangleNormalsPtr;

    const VecDataArray<double, 3>& vertices = *m_vertexPositions;
    const VecDataArray<int, 3>&    indices  = *m_triangleIndices;
    for (size_t triangleId = 0; triangleId < triangleNormals.size(); ++triangleId)
    {
        const auto& t  = indices[triangleId];
        const auto& p0 = vertices[t[0]];
        const auto& p1 = vertices[t[1]];
        const auto& p2 = vertices[t[2]];

        triangleNormals[triangleId] = ((p1 - p0).cross(p2 - p0)).normalized();
    }
    setCellNormals("normals", triangleNormalsPtr);
}

void
SurfaceMesh::computeTriangleTangents()
{
    std::shared_ptr<VecDataArray<float, 2>> uvsPtr = getVertexTCoords();
    if (uvsPtr != nullptr)
    {
        // Get the tangents, avoid reallocating if possible
        std::shared_ptr<VecDataArray<double, 3>> triangleTangentsPtr = getCellTangents();
        if (triangleTangentsPtr == nullptr)
        {
            triangleTangentsPtr = std::make_shared<VecDataArray<double, 3>>(m_triangleIndices->size());
        }
        else
        {
            if (m_triangleIndices->size() != triangleTangentsPtr->size())
            {
                triangleTangentsPtr->resize(static_cast<int>(m_triangleIndices->size()));
            }
        }
        VecDataArray<double, 3>& triangleTangents = *triangleTangentsPtr;

        // Get the normals, compute if we need too
        std::shared_ptr<VecDataArray<double, 3>> triangleNormalsPtr = getCellNormals();
        if (triangleNormalsPtr == nullptr)
        {
            computeTrianglesNormals();
        }
        triangleNormalsPtr = getCellNormals();
        const VecDataArray<double, 3>& triangleNormals = *triangleNormalsPtr;
        const VecDataArray<float, 2>&  uvs      = *uvsPtr;
        const VecDataArray<double, 3>& vertices = *m_vertexPositions;
        const VecDataArray<int, 3>&    indices  = *m_triangleIndices;
        for (size_t triangleId = 0; triangleId < triangleNormals.size(); ++triangleId)
        {
            const Vec3i& t   = indices[triangleId];
            const Vec3d& p0  = vertices[t[0]];
            const Vec3d& p1  = vertices[t[1]];
            const Vec3d& p2  = vertices[t[2]];
            const Vec2f& uv0 = uvs[t[0]];
            const Vec2f& uv1 = uvs[t[1]];
            const Vec2f& uv2 = uvs[t[2]];

            const Vec3d diffPos1   = p1 - p0;
            const Vec3d diffPos2   = p2 - p0;
            const float diffUV1[2] = { uv1[0] - uv0[0], uv1[1] - uv0[1] };
            const float diffUV2[2] = { uv2[0] - uv0[0], uv2[1] - uv0[1] };

            triangleTangents[triangleId] = (diffPos1 * diffUV2[1] - diffPos2 * diffUV1[0]) /
                                           (diffUV1[0] * diffUV2[1] - diffUV1[1] * diffUV2[0]);
        }
        setCellTangents("tangents", triangleTangentsPtr);
    }
}

void
SurfaceMesh::computeVertexNormals()
{
    // Try to not to reallocate if we don't have too
    std::shared_ptr<VecDataArray<double, 3>> vertexNormalsPtr = getVertexNormals();
    if (vertexNormalsPtr == nullptr)
    {
        vertexNormalsPtr = std::make_shared<VecDataArray<double, 3>>(m_vertexPositions->size());
    }
    else
    {
        if (m_vertexPositions->size() != vertexNormalsPtr->size())
        {
            vertexNormalsPtr->resize(m_vertexPositions->size());
        }
    }
    VecDataArray<double, 3>& vertexNormals = *vertexNormalsPtr;

    // First we must compute per triangle normals
    this->computeTrianglesNormals();

    this->computeVertexNeighborTriangles();

    // Sum them all into temp_normals
    VecDataArray<double, 3>                  temp_normals(vertexNormals.size());
    std::shared_ptr<VecDataArray<double, 3>> triangleNormalsPtr = getCellNormals();
    const VecDataArray<double, 3>&           triangleNormals    = *triangleNormalsPtr;
    for (size_t vertexId = 0; vertexId < vertexNormals.size(); ++vertexId)
    {
        temp_normals[vertexId] = Vec3d(0.0, 0.0, 0.0);
        for (const size_t& triangleId : m_vertexNeighborTriangles.at(vertexId))
        {
            temp_normals[vertexId] += triangleNormals[triangleId];
        }
    }

    // Correct for UV seams
    Vec3d                          normal;
    const VecDataArray<double, 3>& vertices = *m_vertexPositions;
    for (int vertexId = 0; vertexId < vertexNormals.size(); ++vertexId)
    {
        NormalGroup group = { vertices[vertexId], vertexNormals[vertexId] };

        normal = temp_normals[vertexId];

        if (m_UVSeamVertexGroups.find(group) == m_UVSeamVertexGroups.end())
        {
            normal.normalize();
            vertexNormals[vertexId] = normal;
            continue;
        }

        auto seamGroup = *m_UVSeamVertexGroups[group].get();

        for (auto index : seamGroup)
        {
            normal += temp_normals[index];
        }

        normal.normalize();
        vertexNormals[vertexId] = normal;
    }

    setVertexNormals("normals", vertexNormalsPtr);
}

void
SurfaceMesh::computeVertexTangents()
{
    const bool hasUVs = hasVertexAttribute(m_activeVertexTCoords);
    if (hasUVs)
    {
        // Avoid reallocating if possible
        std::shared_ptr<VecDataArray<float, 3>> vertexTangentsPtr = getVertexTangents();
        if (vertexTangentsPtr == nullptr)
        {
            vertexTangentsPtr = std::make_shared<VecDataArray<float, 3>>(m_vertexPositions->size());
        }
        else
        {
            if (m_vertexPositions->size() != vertexTangentsPtr->size())
            {
                vertexTangentsPtr->resize(m_vertexPositions->size());
            }
        }
        VecDataArray<float, 3>& vertexTangents = *vertexTangentsPtr;

        // First we need per triangle tangents
        this->computeTriangleTangents();

        VecDataArray<double, 3>                  temp_vertex_tangents(vertexTangents.size());
        std::shared_ptr<VecDataArray<double, 3>> triangleTangentsPtr = getCellTangents();
        const VecDataArray<double, 3>&           triangleTangents    = *triangleTangentsPtr;
        for (int vertexId = 0; vertexId < vertexTangents.size(); ++vertexId)
        {
            temp_vertex_tangents[vertexId] = Vec3d(0.0, 0.0, 0.0);
            for (const size_t& triangleId : m_vertexNeighborTriangles.at(vertexId))
            {
                temp_vertex_tangents[vertexId] += triangleTangents[triangleId];
            }
        }

        // Correct for UV seams
        Vec3d tangent;
        for (size_t vertexId = 0; vertexId < vertexTangents.size(); ++vertexId)
        {
            tangent = temp_vertex_tangents[vertexId];
            tangent.normalize();
            vertexTangents[vertexId] = tangent.cast<float>();
        }

        setVertexTangents("tangents", vertexTangentsPtr);
    }
}

void
SurfaceMesh::optimizeForDataLocality()
{
    const size_t numVertices  = this->getNumVertices();
    const size_t numTriangles = this->getNumTriangles();

    // First find the list of triangles a given vertex is part of
    std::vector<std::vector<int>> vertexNeighbors;
    vertexNeighbors.resize(this->getNumVertices());
    int                   triangleId      = 0;
    VecDataArray<int, 3>& triangleIndices = *m_triangleIndices;
    for (const auto& tri : triangleIndices)
    {
        vertexNeighbors[tri[0]].push_back(triangleId);
        vertexNeighbors[tri[1]].push_back(triangleId);
        vertexNeighbors[tri[2]].push_back(triangleId);

        triangleId++;
    }

    std::shared_ptr<VecDataArray<int, 3>> optimizedConnectivityPtr = std::make_shared<VecDataArray<int, 3>>();
    VecDataArray<int, 3>&                 optimizedConnectivity    = *optimizedConnectivityPtr;
    std::vector<int>                      optimallyOrderedNodes;
    std::list<int>                        triUnderConsideration;
    std::vector<bool>                     isNodeAdded(numVertices, false);
    std::vector<bool>                     isTriangleAdded(numTriangles, false);
    std::list<int>                        newlyAddedNodes;

    // A. Initialize
    optimallyOrderedNodes.push_back(0);
    isNodeAdded.at(0) = true;
    for (const auto& neighTriId : vertexNeighbors[0])
    {
        triUnderConsideration.push_back(neighTriId);
    }

    // B. Iterate till all the nodes are added to optimized mesh
    int vertId[3];

    while (!triUnderConsideration.empty())
    {
        // B.1 Add new nodes and triangles
        for (const auto& triId : triUnderConsideration)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (!isNodeAdded.at(triangleIndices[triId][i]))
                {
                    optimallyOrderedNodes.push_back(triangleIndices[triId][i]);
                    isNodeAdded.at(triangleIndices[triId][i]) = true;
                    newlyAddedNodes.push_back(triangleIndices[triId][i]);
                }
                vertId[i] = *std::find(optimallyOrderedNodes.begin(),
                    optimallyOrderedNodes.end(),
                    triangleIndices[triId][i]);
            }
            optimizedConnectivity.push_back(Vec3i(vertId[0], vertId[1], vertId[2]));
            isTriangleAdded.at(triId) = true;
        }

        // B.2 Setup triangles to be considered for next iteration
        triUnderConsideration.clear();
        for (const auto& newNodes : newlyAddedNodes)
        {
            for (const auto& neighTriId : vertexNeighbors[newNodes])
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
    std::shared_ptr<VecDataArray<double, 3>> optimallyOrderedNodalPos  = std::make_shared<VecDataArray<double, 3>>();
    std::shared_ptr<VecDataArray<int, 3>>    optConnectivityRenumbered = std::make_shared<VecDataArray<int, 3>>();

    // C.1 Get the positions
    optimallyOrderedNodalPos->reserve(static_cast<int>(optimallyOrderedNodes.size()));
    for (const auto& nodalId : optimallyOrderedNodes)
    {
        optimallyOrderedNodalPos->push_back(this->getInitialVertexPosition(nodalId));
    }

    // C.2 Get the renumbered connectivity
    for (size_t i = 0; i < numTriangles; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            vertId[j] = (std::find(optimallyOrderedNodes.begin(),
                optimallyOrderedNodes.end(),
                optimizedConnectivity[i][j]) -
                         optimallyOrderedNodes.begin());
        }

        Vec3i tmpTriArray = Vec3i(vertId[0], vertId[1], vertId[2]);
        optConnectivityRenumbered->push_back(tmpTriArray);
    }

    // D. Assign the rewired mesh data to the mesh
    this->initialize(optimallyOrderedNodalPos, optConnectivityRenumbered);
}

size_t
SurfaceMesh::getNumTriangles() const
{
    return m_triangleIndices->size();
}

void
SurfaceMesh::flipNormals()
{
    for (auto& tri : *m_triangleIndices)
    {
        std::swap(tri[0], tri[1]);
    }
}

void
SurfaceMesh::correctWindingOrder()
{
    // Enforce consistency in winding of a particular triangle with its neighbor (master)
    VecDataArray<int, 3>& indices = *m_triangleIndices;
    auto                  enforceWindingConsistency =
        [&](const size_t masterTriId, const size_t neighTriId)
        {
            const Vec3i& masterTri = indices[masterTriId];
            Vec3i&       neighTri  = indices[neighTriId];

            for (unsigned int l = 0; l < 3; ++l)
            {
                for (unsigned int k = 0; k < 3; ++k)
                {
                    if (masterTri[k] == neighTri[l] && masterTri[(k + 1) % 3] == neighTri[(l + 1) % 3])
                    {
                        // Flip the order of neighbor triangle
                        auto tempId = neighTri[0];
                        neighTri[0] = neighTri[1];
                        neighTri[1] = tempId;
                        break;
                    }
                }
            }
        };

    // Search for triangle neighbors that share a common edge
    auto getTriangleNeighbors =
        [&](const size_t triID, int* neig)
        {
            const auto& currentTri = indices[triID];
            size_t      currentId  = 0;
            int         numNeigh   = 0;
            for (int j = 0; j < indices.size(); j++)
            {
                Vec3i& tri = indices[j];
                if (triID == currentId)
                {
                    currentId++;
                    continue;
                }

                int numCommon = 0;
                for (int i = 0; i < 3; ++i)
                {
                    if (currentTri[i] == tri[0] || currentTri[i] == tri[1] || currentTri[i] == tri[2])
                    {
                        numCommon++;
                        if (numCommon == 2)
                        {
                            neig[numNeigh] = (int)currentId;
                            numNeigh++;

                            if (numNeigh == 3)
                            {
                                return;
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
                currentId++;
            }
        };

    // Start with a reference triangle and enforce the consistency of its neighbors
    // Keep track of those neighbor triangles whose order is enforced but its neighbors not
    // necessarily enforced (trianglesCorrected). Continue this until there is no
    // triangle left in the list
    std::vector<bool>   trianglesCorrected(this->getNumTriangles(), false);
    std::vector<size_t> correctedTriangles;

    size_t currentTriangle = 0;  // Start with triangle 0
    correctedTriangles.push_back(currentTriangle);
    trianglesCorrected[currentTriangle] = true;
    do
    {
        currentTriangle = correctedTriangles[0];
        int neighborTri[3] = { -1, -1, -1 };
        getTriangleNeighbors(currentTriangle, &neighborTri[0]);

        for (int i = 0; i < 3; ++i)
        {
            if (neighborTri[i] >= 0 && !trianglesCorrected[neighborTri[i]])
            {
                enforceWindingConsistency(currentTriangle, neighborTri[i]);

                correctedTriangles.push_back(neighborTri[i]);
                trianglesCorrected[neighborTri[i]] = true;
            }
        }

        correctedTriangles.erase(
            std::remove(correctedTriangles.begin(), correctedTriangles.end(), currentTriangle),
            correctedTriangles.end());
    }
    while (correctedTriangles.size() > 0);
}

void
SurfaceMesh::computeUVSeamVertexGroups()
{
    // Reset vertex groups
    m_UVSeamVertexGroups.clear();

    std::shared_ptr<VecDataArray<double, 3>> vertexNormalsPtr = getVertexNormals();
    if (m_vertexPositions->size() != vertexNormalsPtr->size())
    {
        return;
    }

    // Initial pass to bin vertices based on positions
    const VecDataArray<double, 3>& vertexNormals = *vertexNormalsPtr;
    const VecDataArray<double, 3>& vertices      = *m_vertexPositions;
    for (size_t i = 0; i < vertices.size(); i++)
    {
        NormalGroup group = { vertices[i], vertexNormals[i] };

        if (m_UVSeamVertexGroups.find(group) == m_UVSeamVertexGroups.end())
        {
            m_UVSeamVertexGroups.insert(
                std::pair<NormalGroup, std::shared_ptr<std::vector<size_t>>>(
                    group, std::make_shared<std::vector<size_t>>()));
        }
        m_UVSeamVertexGroups[group]->push_back(i);
    }
}

void
SurfaceMesh::deepCopy(std::shared_ptr<SurfaceMesh> srcMesh)
{
    // todo: Add deep copies to all geometry classes
    // SurfaceMesh members
    this->m_triangleIndices = std::make_shared<VecDataArray<int, 3>>(*srcMesh->m_triangleIndices);
    this->m_vertexNeighborTriangles = srcMesh->m_vertexNeighborTriangles;
    this->m_vertexNeighborVertices  = srcMesh->m_vertexNeighborVertices;
    for (auto i : srcMesh->m_UVSeamVertexGroups)
    {
        this->m_UVSeamVertexGroups[i.first] = std::make_shared<std::vector<size_t>>(*i.second);
    }
    // todo: abstract DataArray's can't be copied
    for (auto i : srcMesh->m_cellAttributes)
    {
        this->m_cellAttributes[i.first] = i.second;
    }
    this->m_activeCellNormals  = srcMesh->m_activeCellNormals;
    this->m_activeCellScalars  = srcMesh->m_activeCellScalars;
    this->m_activeCellTangents = srcMesh->m_activeCellTangents;
    //this->m_originalNumTriangles = srcMesh->m_originalNumTriangles;
    //this->m_maxNumTriangles      = srcMesh->m_maxNumTriangles;

    // PointSet members
    this->m_initialVertexPositions = std::make_shared<VecDataArray<double, 3>>(*srcMesh->m_initialVertexPositions);
    this->m_vertexPositions = std::make_shared<VecDataArray<double, 3>>(*srcMesh->m_vertexPositions);
    // todo: abstract DataArray's can't be copied
    for (auto i : srcMesh->m_vertexAttributes)
    {
        this->m_vertexAttributes[i.first] = i.second;
    }
    this->m_activeVertexNormals  = srcMesh->m_activeVertexNormals;
    this->m_activeVertexScalars  = srcMesh->m_activeVertexScalars;
    this->m_activeVertexTangents = srcMesh->m_activeVertexTangents;
    this->m_activeVertexTCoords  = srcMesh->m_activeVertexTCoords;
    this->m_loadFactor          = srcMesh->m_loadFactor;
    this->m_maxNumVertices      = srcMesh->m_maxNumVertices;
    this->m_originalNumVertices = srcMesh->m_originalNumVertices;

    // Geometry members
    this->m_name = srcMesh->m_name;
    this->m_geometryIndex    = getUniqueID();
    this->m_transformApplied = srcMesh->m_transformApplied;
    this->m_transform = srcMesh->m_transform;
}

const Vec3i&
SurfaceMesh::getTriangleIndices(const int triangleNum) const
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (triangleNum >= m_triangleIndices->size())) << "Invalid index";
#endif
    return (*m_triangleIndices)[triangleNum];
}

Vec3i&
SurfaceMesh::getTriangleIndices(const int triangleNum)
{
#if defined(DEBUG) || defined(_DEBUG) || !defined(NDEBUG)
    LOG_IF(FATAL, (triangleNum >= m_triangleIndices->size())) << "Invalid index";
#endif
    return (*m_triangleIndices)[triangleNum];
}

void
SurfaceMesh::setCellAttribute(const std::string& arrayName, std::shared_ptr<AbstractDataArray> arr)
{
    m_cellAttributes[arrayName] = arr;
}

std::shared_ptr<AbstractDataArray>
SurfaceMesh::getCellAttribute(const std::string& arrayName) const
{
    auto it = m_cellAttributes.find(arrayName);
    if (it == m_cellAttributes.end())
    {
        LOG(WARNING) << "No array with such name holds any cell data.";
        return nullptr;
    }
    return it->second;
}

bool
SurfaceMesh::hasCellAttribute(const std::string& arrayName) const
{
    return (m_cellAttributes.find(arrayName) != m_cellAttributes.end());
}

void
SurfaceMesh::setCellScalars(const std::string& arrayName, std::shared_ptr<AbstractDataArray> scalars)
{
    m_activeCellScalars = arrayName;
    m_cellAttributes[arrayName] = scalars;
}

void
SurfaceMesh::setCellScalars(const std::string& arrayName)
{
    if (hasCellAttribute(arrayName))
    {
        m_activeCellScalars = arrayName;
    }
}

std::shared_ptr<AbstractDataArray>
SurfaceMesh::getCellScalars() const
{
    if (hasCellAttribute(m_activeCellScalars))
    {
        return m_cellAttributes.at(m_activeCellScalars);
    }
    else
    {
        return nullptr;
    }
}

void
SurfaceMesh::setCellNormals(const std::string& arrayName, std::shared_ptr<VecDataArray<double, 3>> normals)
{
    m_activeCellNormals = arrayName;
    m_cellAttributes[arrayName] = normals;
}

void
SurfaceMesh::setCellNormals(const std::string& arrayName)
{
    if (hasCellAttribute(arrayName))
    {
        m_activeCellNormals = arrayName;
        setActiveAttribute(m_activeCellNormals, arrayName, 3, IMSTK_DOUBLE);
    }
}

std::shared_ptr<VecDataArray<double, 3>>
SurfaceMesh::getCellNormals() const
{
    if (hasCellAttribute(m_activeCellNormals))
    {
        return std::dynamic_pointer_cast<VecDataArray<double, 3>>(m_cellAttributes.at(m_activeCellNormals));
    }
    else
    {
        return nullptr;
    }
}

void
SurfaceMesh::setCellTangents(const std::string& arrayName, std::shared_ptr<VecDataArray<double, 3>> tangents)
{
    m_activeCellTangents = arrayName;
    m_cellAttributes[arrayName] = tangents;
}

void
SurfaceMesh::setCellTangents(const std::string& arrayName)
{
    if (hasCellAttribute(arrayName))
    {
        m_activeCellTangents = arrayName;
        setActiveAttribute(m_activeCellTangents, arrayName, 3, IMSTK_DOUBLE);
    }
}

std::shared_ptr<VecDataArray<double, 3>>
SurfaceMesh::getCellTangents() const
{
    if (hasCellAttribute(m_activeCellTangents))
    {
        return std::dynamic_pointer_cast<VecDataArray<double, 3>>(m_cellAttributes.at(m_activeCellTangents));
    }
    else
    {
        return nullptr;
    }
}
}  // namespace imstk
