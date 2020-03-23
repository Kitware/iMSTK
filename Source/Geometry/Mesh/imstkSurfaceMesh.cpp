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
#include "imstkTimer.h"
//#include "imstkGraph.h"

namespace imstk
{
void
SurfaceMesh::initialize(const StdVectorOfVec3d& vertices,
                        const std::vector<TriangleArray>& triangles, const bool computeDerivedData)
{
    this->clear();

    PointSet::initialize(vertices);
    this->setTrianglesVertices(triangles);

    if (computeDerivedData)
    {
        this->computeVertexNormals();
    }
}

void
SurfaceMesh::initialize(const StdVectorOfVec3d& vertices,
                        const std::vector<TriangleArray>& triangles,
                        const StdVectorOfVec3d& normals, const bool computeDerivedData)
{
    this->initialize(vertices, triangles, computeDerivedData);

    this->m_vertexNormals = normals;
    this->computeUVSeamVertexGroups();

    this->computeVertexNormals();
}

void
SurfaceMesh::clear()
{
    PointSet::clear();
    m_trianglesVertices.clear();
    m_vertexNeighborTriangles.clear();
    m_vertexNeighborVertices.clear();
    m_triangleNormals.clear();
    m_vertexNormals.clear();
    m_triangleTangents.clear();
}

void
SurfaceMesh::print() const
{
    PointSet::print();

    LOG(INFO) << "Number of triangles: " << this->getNumTriangles();
    LOG(INFO) << "Triangles:";
    for (auto& tri : m_trianglesVertices)
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
SurfaceMesh::computeVertexNeighborTriangles()
{
    m_vertexNeighborTriangles.clear();
    m_vertexNeighborTriangles.resize(m_vertexPositions.size());

    size_t triangleId = 0;

    for (const auto& t : m_trianglesVertices)
    {
        m_vertexNeighborTriangles.at(t.at(0)).insert(triangleId);
        m_vertexNeighborTriangles.at(t.at(1)).insert(triangleId);
        m_vertexNeighborTriangles.at(t.at(2)).insert(triangleId);
        triangleId++;
    }
}

void
SurfaceMesh::computeVertexNeighborVertices()
{
    m_vertexNeighborVertices.clear();
    m_vertexNeighborVertices.resize(m_vertexPositions.size());

    if (m_vertexNeighborTriangles.size() != m_vertexPositions.size())
    {
        this->computeVertexNeighborTriangles();
    }

    for (size_t vertexId = 0; vertexId < m_vertexNeighborVertices.size(); ++vertexId)
    {
        for (const size_t& triangleId : m_vertexNeighborTriangles.at(vertexId))
        {
            for (const size_t& vertexId2 : m_trianglesVertices.at(triangleId))
            {
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
    m_triangleNormals.resize(m_trianglesVertices.size());
    m_triangleTangents.resize(m_trianglesVertices.size());

    for (size_t triangleId = 0; triangleId < m_triangleNormals.size(); ++triangleId)
    {
        const auto& t  = m_trianglesVertices.at(triangleId);
        const auto& p0 = m_vertexPositions.at(t.at(0));
        const auto& p1 = m_vertexPositions.at(t.at(1));
        const auto& p2 = m_vertexPositions.at(t.at(2));

        m_triangleNormals.at(triangleId) = ((p1 - p0).cross(p2 - p0)).normalized();
    }

    bool                      hasUVs = this->hasPointDataArray(m_defaultTCoords);
    const StdVectorOfVectorf* UVs    = nullptr;

    if (hasUVs)
    {
        UVs = this->getPointDataArray(m_defaultTCoords);
    }

    if (hasUVs)
    {
        for (size_t triangleId = 0; triangleId < m_triangleNormals.size(); ++triangleId)
        {
            const auto& t   = m_trianglesVertices.at(triangleId);
            const auto& p0  = m_vertexPositions.at(t.at(0));
            const auto& p1  = m_vertexPositions.at(t.at(1));
            const auto& p2  = m_vertexPositions.at(t.at(2));
            const auto& uv0 = (*UVs)[t.at(0)];
            const auto& uv1 = (*UVs)[t.at(1)];
            const auto& uv2 = (*UVs)[t.at(2)];

            auto  diffPos1   = p1 - p0;
            auto  diffPos2   = p2 - p0;
            float diffUV1[2] = { uv1[0] - uv0[0], uv1[1] - uv0[1] };
            float diffUV2[2] = { uv2[0] - uv0[0], uv2[1] - uv0[1] };

            m_triangleTangents.at(triangleId) = (diffPos1 * diffUV2[1] - diffPos2 * diffUV1[0]) /
                                                (diffUV1[0] * diffUV2[1] - diffUV1[1] * diffUV2[0]);
        }
    }
}

void
SurfaceMesh::computeVertexNormals()
{
    if (m_topologyChanged)
    {
        this->computeVertexNeighborTriangles();
        this->computeUVSeamVertexGroups();
        m_topologyChanged = false;
    }

    m_vertexNormals.resize(m_vertexPositions.size());
    m_vertexTangents.resize(m_vertexPositions.size());

    this->computeTrianglesNormals();

    StdVectorOfVec3d temp_normals(m_vertexNormals.size());
    StdVectorOfVec3d temp_tangents(m_vertexTangents.size());
    for (size_t vertexId = 0; vertexId < m_vertexNormals.size(); ++vertexId)
    {
        temp_normals[vertexId]  = Vec3d(0, 0, 0);
        temp_tangents[vertexId] = Vec3d(0, 0, 0);
        for (const size_t& triangleId : m_vertexNeighborTriangles.at(vertexId))
        {
            temp_normals[vertexId]  += m_triangleNormals[triangleId];
            temp_tangents[vertexId] += m_triangleTangents[triangleId];
        }
    }

    // Correct for UV seams
    Vec3d                     normal, tangent;
    bool                      hasUVs = this->hasPointDataArray(m_defaultTCoords);
    const StdVectorOfVectorf* UVs;

    if (hasUVs)
    {
        UVs = this->getPointDataArray(m_defaultTCoords);
    }

    for (size_t vertexId = 0; vertexId < m_vertexNormals.size(); ++vertexId)
    {
        NormalGroup group = { m_vertexPositions[vertexId], m_vertexNormals[vertexId] };

        normal = temp_normals[vertexId];

        if (m_UVSeamVertexGroups.find(group) == m_UVSeamVertexGroups.end())
        {
            normal.normalize();
            m_vertexNormals[vertexId] = normal;
            continue;
        }

        auto seamGroup = *m_UVSeamVertexGroups[group].get();

        for (auto index : seamGroup)
        {
            normal += temp_normals[index];
        }

        normal.normalize();
        m_vertexNormals[vertexId] = normal;

        if (hasUVs)
        {
            tangent = temp_tangents[vertexId];
            tangent.normalize();
            m_vertexTangents[vertexId] = tangent;
        }
    }
}

void
SurfaceMesh::optimizeForDataLocality()
{
    const size_t numVertices  = this->getNumVertices();
    const size_t numTriangles = this->getNumTriangles();

    // First find the list of triangles a given vertex is part of
    std::vector<std::vector<size_t>> vertexNeighbors;
    vertexNeighbors.resize(this->getNumVertices());
    size_t triangleId = 0;
    for (const auto& tri : this->getTrianglesVertices())
    {
        vertexNeighbors[tri[0]].push_back(triangleId);
        vertexNeighbors[tri[1]].push_back(triangleId);
        vertexNeighbors[tri[2]].push_back(triangleId);

        triangleId++;
    }

    std::vector<TriangleArray> optimizedConnectivity;
    std::vector<size_t>        optimallyOrderedNodes;
    std::list<size_t>          triUnderConsideration;
    std::vector<bool>          isNodeAdded(numVertices, false);
    std::vector<bool>          isTriangleAdded(numTriangles, false);
    std::list<size_t>          newlyAddedNodes;

    // A. Initialize
    optimallyOrderedNodes.push_back(0);
    isNodeAdded.at(0) = true;
    for (const auto& neighTriId : vertexNeighbors[0])
    {
        triUnderConsideration.push_back(neighTriId);
    }

    // B. Iterate till all the nodes are added to optimized mesh
    size_t vertId[3];
    auto   connectivity = this->getTrianglesVertices();
    while (triUnderConsideration.size() != 0)
    {
        // B.1 Add new nodes and triangles
        for (const auto& triId : triUnderConsideration)
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
    StdVectorOfVec3d           optimallyOrderedNodalPos;
    std::vector<TriangleArray> optConnectivityRenumbered;

    // C.1 Get the positions
    for (const auto& nodalId : optimallyOrderedNodes)
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
                         optimallyOrderedNodes.begin());
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
    if (m_originalNumTriangles == 0)
    {
        m_originalNumTriangles = triangles.size();
        m_maxNumTriangles      = (size_t)(m_originalNumTriangles * m_loadFactor);
        m_trianglesVertices.reserve(m_maxNumTriangles);
        m_vertexNormals.reserve(m_maxNumVertices);
        m_vertexTangents.reserve(m_maxNumVertices);
        m_topologyChanged = true;
    }

    if (triangles.size() <= m_maxNumTriangles)
    {
        m_topologyChanged   = true;
        m_trianglesVertices = triangles;
    }
    else
    {
        LOG(WARNING) << "Triangles not set, exceeded maximum number of triangles";
    }
}

const StdVectorOfVec3d&
SurfaceMesh::getTriangleNormals() const
{
    return m_triangleNormals;
}

const Vec3d&
SurfaceMesh::getTriangleNormal(size_t i) const
{
    return m_triangleNormals.at(i);
}

void
SurfaceMesh::setVertexNormals(const StdVectorOfVec3d& normals)
{
    if (normals.size() <= m_maxNumVertices)
    {
        m_vertexNormals = normals;
    }
    else
    {
        LOG(WARNING) << "Normals not set, exceeded maximum number of vertices";
    }
}

const StdVectorOfVec3d&
SurfaceMesh::getVertexNormals() const
{
    return m_vertexNormals;
}

StdVectorOfVec3d&
SurfaceMesh::getVertexNormalsNotConst()
{
    return m_vertexNormals;
}

void
SurfaceMesh::setVertexTangents(const StdVectorOfVec3d& tangents)
{
    if (tangents.size() <= m_maxNumVertices)
    {
        m_vertexTangents = tangents;
    }
    else
    {
        LOG(WARNING) << "Tangents not set, exceeded maximum number of vertices";
    }
}

const StdVectorOfVec3d&
SurfaceMesh::getVertexTangents() const
{
    return m_vertexTangents;
}

size_t
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
SurfaceMesh::flipNormals()
{
    for (auto& tri : m_trianglesVertices)
    {
        auto temp = tri[0];
        tri[0] = tri[1];
        tri[1] = temp;
    }
}

void
SurfaceMesh::correctWindingOrder()
{
    // Enforce consistency in winding of a particular triangle with its neighbor (master)
    auto enforceWindingConsistency = [this](const size_t masterTriId, const size_t neighTriId) {
                                         const auto& masterTri = m_trianglesVertices[masterTriId];
                                         auto&       neighTri  = m_trianglesVertices[neighTriId];

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
    auto getTriangleNeighbors = [this](const size_t triID, int* neig) {
                                    const auto& currentTri = m_trianglesVertices[triID];
                                    size_t      currentId  = 0;
                                    int         numNeigh   = 0;
                                    for (auto& tri : m_trianglesVertices)
                                    {
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

    if (m_vertexPositions.size() != m_vertexNormals.size())
    {
        return;
    }

    // Initial pass to bin vertices based on positions
    for (size_t i = 0; i < m_vertexPositions.size(); i++)
    {
        NormalGroup group = { m_vertexPositions[i], m_vertexNormals[i] };

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
SurfaceMesh::setLoadFactor(double loadFactor)
{
    m_loadFactor      = loadFactor;
    m_maxNumVertices  = (size_t)(m_originalNumVertices * m_loadFactor);
    m_maxNumTriangles = (size_t)(m_originalNumTriangles * m_loadFactor);
    m_trianglesVertices.reserve(m_maxNumTriangles);
    m_vertexNormals.reserve(m_maxNumVertices);
    m_vertexTangents.reserve(m_maxNumVertices);
}

size_t
SurfaceMesh::getMaxNumTriangles()
{
    return m_maxNumTriangles;
}

std::vector<bool>
SurfaceMesh::markPointsInsideAndOut(const StdVectorOfVec3d& coords)
{
    std::vector<bool> isInside;
    isInside.resize(coords.size(), false);

    Vec3d aabbMin, aabbMax;
    this->computeBoundingBox(aabbMin, aabbMax, 1.);

    auto genRandomDirection = [](Vec3d& dir)
                              {
                                  for (int i = 0; i < 3; ++i)
                                  {
                                      dir[i] = rand();
                                  }
                                  double mag = dir.norm();

                                  for (int i = 0; i < 3; ++i)
                                  {
                                      dir[i] /= mag;
                                  }
                                  return;
                              };

    auto intersectTriangle = [](const Vec3d& xyz, const Vec3d& xyz0, const Vec3d& xyz1, const Vec3d& xyz2, const Vec3d& dir)
                             {
                                 // const double eps = 1e-15;
                                 constexpr const double eps   = std::numeric_limits<double>::epsilon();
                                 Vec3d                  edge0 = xyz1 - xyz0;
                                 Vec3d                  edge1 = xyz2 - xyz0;
                                 Vec3d                  pvec  = dir.cross(edge1);
                                 double                 det   = edge0.dot(pvec);

                                 if (det > -eps && det < eps)
                                 {
                                     return false;
                                 }
                                 double inv_det = 1.0 / det;
                                 Vec3d  tvec    = xyz - xyz0;
                                 double u       = tvec.dot(pvec) * inv_det;
                                 if (u < 0.0 || u > 1.0)
                                 {
                                     return false;
                                 }
                                 Vec3d  qvec = tvec.cross(edge0);
                                 double v    = dir.dot(qvec) * inv_det;
                                 if (v < 0.0 || u + v > 1.0)
                                 {
                                     return false;
                                 }

                                 double t = edge1.dot(qvec) * inv_det;
                                 if (t > 0.0)
                                 {
                                     return true;
                                 }
                                 else
                                 {
                                     return false;
                                 }
                             };

    std::vector<Vec3d> bBoxMin;
    std::vector<Vec3d> bBoxMax;

    bBoxMin.resize(this->getNumTriangles());
    bBoxMax.resize(this->getNumTriangles());

    for (size_t idx = 0; idx < this->getNumTriangles(); ++idx)
    {
        const auto& verts = m_trianglesVertices.at(idx);
        const auto& xyz0  = m_vertexPositions[verts[0]];
        const auto& xyz1  = m_vertexPositions[verts[1]];
        const auto& xyz2  = m_vertexPositions[verts[2]];

        bBoxMin[idx][0] = xyz0[0];
        bBoxMin[idx][1] = xyz0[1];
        bBoxMin[idx][2] = xyz0[2];
        bBoxMax[idx][0] = xyz0[0];
        bBoxMax[idx][1] = xyz0[1];
        bBoxMax[idx][2] = xyz0[2];

        bBoxMin[idx][0] = std::min(bBoxMin[idx][0], xyz1[0]);
        bBoxMin[idx][1] = std::min(bBoxMin[idx][1], xyz1[1]);
        bBoxMin[idx][2] = std::min(bBoxMin[idx][2], xyz1[2]);
        bBoxMin[idx][0] = std::min(bBoxMin[idx][0], xyz2[0]);
        bBoxMin[idx][1] = std::min(bBoxMin[idx][1], xyz2[1]);
        bBoxMin[idx][2] = std::min(bBoxMin[idx][2], xyz2[2]);

        bBoxMax[idx][0] = std::max(bBoxMax[idx][0], xyz1[0]);
        bBoxMax[idx][1] = std::max(bBoxMax[idx][1], xyz1[1]);
        bBoxMax[idx][2] = std::max(bBoxMax[idx][2], xyz1[2]);
        bBoxMax[idx][0] = std::max(bBoxMax[idx][0], xyz2[0]);
        bBoxMax[idx][1] = std::max(bBoxMax[idx][1], xyz2[1]);
        bBoxMax[idx][2] = std::max(bBoxMax[idx][2], xyz2[2]);
    }

    auto rayTracingFunc = [&coords, &aabbMin, &aabbMax, &bBoxMin, &bBoxMax, &isInside, &intersectTriangle, &genRandomDirection, this](const size_t i) {
                              bool outBox = coords[i][0] < aabbMin[0] || coords[i][0] > aabbMax[0]
                                            || coords[i][1] < aabbMin[1] || coords[i][1] > aabbMax[1]
                                            || coords[i][2] < aabbMin[2] || coords[i][2] > aabbMax[2];
                              if (outBox)
                              {
                                  return;
                              }

                              // TODO: generate a random direction?
                              const Vec3d direction = { 0.0, 0.0, 1.0 };
                              // Vec3d direction;
                              // genRandomDirection(direction);
                              // std::cout << direction << std::endl;
                              int         numIntersections = 0;
                              const auto& xyz = m_vertexPositions;

                              for (size_t j = 0; j < this->getNumTriangles(); ++j)
                              {
                                  const auto& verts = m_trianglesVertices[j];

                                  // consider directed ray
                                  if (coords[i][2] > bBoxMax[j][2])
                                  {
                                      continue;
                                  }

                                  if (coords[i][0] > bBoxMax[j][0])
                                  {
                                      continue;
                                  }
                                  if (coords[i][0] < bBoxMin[j][0])
                                  {
                                      continue;
                                  }
                                  if (coords[i][1] > bBoxMax[j][1])
                                  {
                                      continue;
                                  }
                                  if (coords[i][1] < bBoxMin[j][1])
                                  {
                                      continue;
                                  }

                                  auto intersected = intersectTriangle(coords[i],
                                                 xyz[verts[0]],
                                                 xyz[verts[1]],
                                                 xyz[verts[2]],
                                                 direction);
                                  if (intersected)
                                  {
                                      ++numIntersections;
                                  }
                              }

                              if (numIntersections % 2 == 1)
                              {
                                  isInside[i] = true;
                              }

                              return;
                          };

    // for (size_t i = 0; i < coords.size(); ++i)
    // {
    //     rayTracingFunc(i);
    // }

    // CpuTimer timer;
    StopWatch timer;
    timer.start();
    ParallelUtils::parallelFor(coords.size(), rayTracingFunc);
    timer.stop();
    timer.printTimeElapsed();

    return isInside;
}

std::vector<bool>
SurfaceMesh::markPointsInsideAndOut(const StdVectorOfVec3d& coords, const size_t nx, const size_t ny, const size_t nz)
{
    std::vector<bool> isInside;
    isInside.resize(coords.size(), false);

    Vec3d aabbMin, aabbMax;
    this->computeBoundingBox(aabbMin, aabbMax, 1.);
    // space between two adjacent points
    Vec3d h = { coords[1][0] - coords[0][0], coords[nx][1] - coords[0][1], coords[nx * ny][2] - coords[0][2] };

    /// \brief if a ray intersects with a triangle.
    /// \param xyz starting point of ray
    /// \param xyz0 xyz0, xyz1, xyz2 are 3 vertices of the triangle
    /// \param dir direction of the ray
    /// \param dist on return it is the distance of the point and the triangle
    auto intersectTriangle = [](const Vec3d& xyz, const Vec3d& xyz0, const Vec3d& xyz1, const Vec3d& xyz2, const Vec3d& dir, double& dist)
                             {
                                 // const double eps = 1e-15;
                                 const double eps   = std::numeric_limits<double>::epsilon();
                                 Vec3d        edge0 = xyz1 - xyz0;
                                 Vec3d        edge1 = xyz2 - xyz0;
                                 Vec3d        pvec  = dir.cross(edge1);
                                 double       det   = edge0.dot(pvec);

                                 if (det > -eps && det < eps)
                                 {
                                     return false;
                                 }
                                 double inv_det = 1.0 / det;
                                 Vec3d  tvec    = xyz - xyz0;
                                 double u       = tvec.dot(pvec) * inv_det;
                                 if (u < 0.0 || u > 1.0)
                                 {
                                     return false;
                                 }
                                 Vec3d  qvec = tvec.cross(edge0);
                                 double v    = dir.dot(qvec) * inv_det;
                                 if (v < 0.0 || u + v > 1.0)
                                 {
                                     return false;
                                 }

                                 dist = edge1.dot(qvec) * inv_det;
                                 if (dist > 0.0)
                                 {
                                     return true;
                                 }
                                 else
                                 {
                                     return false;
                                 }
                             };

    std::vector<Vec3d> bBoxMin;
    std::vector<Vec3d> bBoxMax;

    bBoxMin.resize(this->getNumTriangles());
    bBoxMax.resize(this->getNumTriangles());

    /// \brief find the bounding boxes of each surface triangle
    auto findBoundingBox = [&](const size_t idx)
                           {
                               const auto& verts = m_trianglesVertices.at(idx);
                               const auto& xyz0  = m_vertexPositions[verts[0]];
                               const auto& xyz1  = m_vertexPositions[verts[1]];
                               const auto& xyz2  = m_vertexPositions[verts[2]];

                               bBoxMin[idx][0] = xyz0[0];
                               bBoxMin[idx][1] = xyz0[1];
                               bBoxMin[idx][2] = xyz0[2];
                               bBoxMax[idx][0] = xyz0[0];
                               bBoxMax[idx][1] = xyz0[1];
                               bBoxMax[idx][2] = xyz0[2];

                               bBoxMin[idx][0] = std::min(bBoxMin[idx][0], xyz1[0]);
                               bBoxMin[idx][1] = std::min(bBoxMin[idx][1], xyz1[1]);
                               bBoxMin[idx][2] = std::min(bBoxMin[idx][2], xyz1[2]);
                               bBoxMin[idx][0] = std::min(bBoxMin[idx][0], xyz2[0]);
                               bBoxMin[idx][1] = std::min(bBoxMin[idx][1], xyz2[1]);
                               bBoxMin[idx][2] = std::min(bBoxMin[idx][2], xyz2[2]);

                               bBoxMax[idx][0] = std::max(bBoxMax[idx][0], xyz1[0]);
                               bBoxMax[idx][1] = std::max(bBoxMax[idx][1], xyz1[1]);
                               bBoxMax[idx][2] = std::max(bBoxMax[idx][2], xyz1[2]);
                               bBoxMax[idx][0] = std::max(bBoxMax[idx][0], xyz2[0]);
                               bBoxMax[idx][1] = std::max(bBoxMax[idx][1], xyz2[1]);
                               bBoxMax[idx][2] = std::max(bBoxMax[idx][2], xyz2[2]);
                           };

    ParallelUtils::parallelFor(this->getNumTriangles(), findBoundingBox);

    // ray tracing for all points in the x-axis. These points are those start with indices (0,j,k)
    // and jk = j + k*ny
    auto rayTracingLine = [&](const size_t jk)
                          {
                              size_t idx0   = jk * nx;
                              bool   outBox = coords[idx0][0] < aabbMin[0] || coords[idx0][0] > aabbMax[0]
                                              || coords[idx0][1] < aabbMin[1] || coords[idx0][1] > aabbMax[1]
                                              || coords[idx0][2] < aabbMin[2] || coords[idx0][2] > aabbMax[2];
                              if (outBox)
                              {
                                  return;
                              }

                              const Vec3d direction = { 1.0, 0.0, 0.0 };
                              const auto& xyz       = m_vertexPositions;

                              size_t i = 0;
                              while (i < nx)
                              {
                                  size_t idx = idx0 + i;
                                  int    numIntersections = 0;
                                  double dist    = 0.0;
                                  double distMin = h[0] * (nz + 1);

                                  for (size_t j = 0; j < this->getNumTriangles(); ++j)
                                  {
                                      const auto& verts = m_trianglesVertices[j];

                                      // consider directed ray
                                      if (coords[idx][0] > bBoxMax[j][0])
                                      {
                                          continue;
                                      }

                                      if (coords[idx][1] > bBoxMax[j][1])
                                      {
                                          continue;
                                      }
                                      if (coords[idx][1] < bBoxMin[j][1])
                                      {
                                          continue;
                                      }
                                      if (coords[idx][2] > bBoxMax[j][2])
                                      {
                                          continue;
                                      }
                                      if (coords[idx][2] < bBoxMin[j][2])
                                      {
                                          continue;
                                      }

                                      auto intersected = intersectTriangle(coords[idx],
                                                     xyz[verts[0]],
                                                     xyz[verts[1]],
                                                     xyz[verts[2]],
                                                     direction,
                                                     dist);
                                      if (intersected)
                                      {
                                          ++numIntersections;
                                          distMin = std::min(dist, distMin);
                                      }
                                  }

                                  // core of the algorithm: points between the current one and iEnd share the same label so we can skip them.
                                  size_t iEnd = i + static_cast<size_t>(distMin / h[0]) + 1;
                                  iEnd = std::min(iEnd, nx);

                                  if (numIntersections % 2 == 1)
                                  {
                                      for (size_t ii = idx; ii < idx0 + iEnd; ++ii)
                                      {
                                          isInside[ii] = true;
                                      }
                                  }

                                  i = iEnd;
                              }
                          };

    ParallelUtils::parallelFor(ny * nz, rayTracingLine);

    return isInside;
}

}  // namespace imstk
