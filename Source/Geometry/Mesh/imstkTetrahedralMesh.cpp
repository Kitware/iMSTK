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

#include "imstkTetrahedralMesh.h"
#include "imstkPointSet.h"
#include "imstkSurfaceMesh.h"
//#include "imstkGraph.h"

namespace imstk
{
void
TetrahedralMesh::initialize(const StdVectorOfVec3d&        vertices,
                            const std::vector<TetraArray>& tetrahedra,
                            bool                           computeAttachedSurfaceMesh)
{
    PointSet::initialize(vertices);
    this->setTetrahedraVertices(tetrahedra);

    if (computeAttachedSurfaceMesh)
    {
        this->computeAttachedSurfaceMesh();
    }

    m_removedMeshElems.resize(tetrahedra.size(), false);
}

void
TetrahedralMesh::clear()
{
    PointSet::clear();
    m_tetrahedraVertices.clear();
}

void
TetrahedralMesh::print() const
{
    PointSet::print();

    LOG(INFO) << "Number of tetrahedra: " << this->getNumTetrahedra();
    LOG(INFO) << "Tetrahedra:";
    for (auto& tet : m_tetrahedraVertices)
    {
        LOG(INFO) << tet.at(0) << ", " << tet.at(1) << ", " << tet.at(2) << ", " << tet.at(3);
    }
}

double
TetrahedralMesh::getVolume() const
{
    Vec3d  v[4];
    Mat4d  A;
    double volume = 0.0;
    for (const TetraArray& tetVertices : m_tetrahedraVertices)
    {
        for (int i = 0; i < 4; ++i)
        {
            v[i] = m_vertexPositions[tetVertices[i]];
        }

        A << v[0][0], v[0][1], v[0][2], 1, v[1][0], v[1][1], v[1][2], 1, v[2][0], v[2][1], v[2][2],
            1, v[3][0], v[3][1], v[3][2], 1;

        double det = A.determinant();
        if (det < 0)
        {
            LOG(WARNING) << "Tetrahedron is inverted, has negative volume!";
        }

        volume += std::abs(det) / 6;
    }

    return volume;
}

void
TetrahedralMesh::computeAttachedSurfaceMesh()
{
    this->m_attachedSurfaceMesh = std::make_shared<imstk::SurfaceMesh>();

    CHECK(this->extractSurfaceMesh(this->m_attachedSurfaceMesh))
        << "TetrahedralMesh::computeAttachedSurfaceMesh error: surface mesh was not extracted.";
}

bool
TetrahedralMesh::extractSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh,
                                    const bool                   enforceWindingConsistency /* = false*/)
{
    CHECK(surfaceMesh) 
            << "TetrahedralMesh::extractSurfaceMesh error: the surface mesh provided is not instantiated.";
 
    using triArray = SurfaceMesh::TriangleArray;
    const std::vector<triArray> facePattern = {
        triArray { { 0, 1, 2 } }, triArray { { 0, 1, 3 } }, triArray { { 0, 2, 3 } }, triArray { { 1, 2, 3 } }
    };

    // Find and store the tetrahedral faces that are unique
    auto                  vertArray = this->getTetrahedraVertices();
    std::vector<triArray> surfaceTri;
    std::vector<size_t>   surfaceTriTet;
    std::vector<size_t>   tetRemainingVert;
    bool                  unique = true;
    size_t                foundAt = 0, tetId = 0;
    size_t                a, b, c;

    for (auto& tetVertArray : vertArray)
    {
        // std::cout << "tet: " << tetId << std::endl;

        for (int t = 0; t < 4; ++t)
        {
            unique  = true;
            foundAt = 0;
            a       = tetVertArray[facePattern[t][0]];
            b       = tetVertArray[facePattern[t][1]];
            c       = tetVertArray[facePattern[t][2]];

            // search in reverse
            for (auto it = surfaceTri.rbegin(); it != surfaceTri.rend(); ++it)
            {
                if ((((*it)[0] == a)
                     && (((*it)[1] == b && (*it)[2] == c) || ((*it)[1] == c && (*it)[2] == b)))
                    || (((*it)[1] == a)
                        && (((*it)[0] == b && (*it)[2] == c) || ((*it)[0] == c && (*it)[2] == b)))
                    || (((*it)[2] == a)
                        && (((*it)[1] == b && (*it)[0] == c) || ((*it)[1] == c && (*it)[0] == b))))
                {
                    unique  = false;
                    foundAt = surfaceTri.size() - 1 - (it - surfaceTri.rbegin());
                    break;
                }
            }

            if (unique)
            {
                surfaceTri.push_back(triArray { { a, b, c } });
                surfaceTriTet.push_back(tetId);
                tetRemainingVert.push_back(3 - t);
            }
            else
            {
                surfaceTri.erase(surfaceTri.begin() + foundAt);
            }
        }
        tetId++;
    }

    // Arrange the surface triangle faces found in order
    Vec3d v0, v1, v2;
    Vec3d centroid;
    Vec3d normal;
    for (size_t faceId = 0; faceId < surfaceTri.size(); ++faceId)
    {
        v0 = this->getVertexPosition(surfaceTri.at(faceId)[0]);
        v1 = this->getVertexPosition(surfaceTri.at(faceId)[1]);
        v2 = this->getVertexPosition(surfaceTri.at(faceId)[2]);

        centroid = (v0 + v1 + v2) / 3;

        normal = ((v0 - v1).cross(v0 - v2));

        if (normal.dot(centroid - this->getVertexPosition(tetRemainingVert.at(faceId))) > 0)
        {
            std::swap(surfaceTri[faceId][2], surfaceTri[faceId][1]);
        }
    }

    // Renumber the vertices
    std::list<size_t> uniqueVertIdList;
    for (const auto& face : surfaceTri)
    {
        uniqueVertIdList.push_back(face[0]);
        uniqueVertIdList.push_back(face[1]);
        uniqueVertIdList.push_back(face[2]);
    }
    uniqueVertIdList.sort();
    uniqueVertIdList.unique();

    size_t                      vertId;
    std::list<size_t>::iterator it;
    StdVectorOfVec3d            vertPositions;
    for (vertId = 0, it = uniqueVertIdList.begin(); it != uniqueVertIdList.end(); ++vertId, it++)
    {
        vertPositions.push_back(this->getVertexPosition(*it));
        for (auto& face : surfaceTri)
        {
            for (size_t i = 0; i < 3; ++i)
            {
                if (face[i] == *it)
                {
                    face[i] = vertId;
                }
            }
        }
    }

    // Create and attach surface mesh
    surfaceMesh->initialize(vertPositions, surfaceTri);

    if (enforceWindingConsistency)
    {
        surfaceMesh->correctWindingOrder();
    }

    return true;
}

void
TetrahedralMesh::computeBarycentricWeights(const size_t& tetId, const Vec3d& pos,
                                           WeightsArray& weights) const
{
    const TetraArray& tetVertices = m_tetrahedraVertices.at(tetId);
    Vec3d             v[4];
    double            det;

    for (int i = 0; i < 4; ++i)
    {
        v[i] = m_vertexPositions[tetVertices[i]];
    }

    Mat4d A;
    A << v[0][0], v[0][1], v[0][2], 1, v[1][0], v[1][1], v[1][2], 1, v[2][0], v[2][1], v[2][2], 1,
        v[3][0], v[3][1], v[3][2], 1;

    det = A.determinant();

    for (int i = 0; i < 4; ++i)
    {
        Mat4d B = A;
        B(i, 0)    = pos[0];
        B(i, 1)    = pos[1];
        B(i, 2)    = pos[2];
        weights[i] = B.determinant() / det;
    }
}

void
TetrahedralMesh::computeTetrahedronBoundingBox(const size_t& tetId, Vec3d& min, Vec3d& max) const
{
    auto v1 = m_vertexPositions[m_tetrahedraVertices.at(tetId)[0]];
    auto v2 = m_vertexPositions[m_tetrahedraVertices.at(tetId)[1]];
    auto v3 = m_vertexPositions[m_tetrahedraVertices.at(tetId)[2]];
    auto v4 = m_vertexPositions[m_tetrahedraVertices.at(tetId)[3]];

    std::array<double, 4> arrayx = { v1[0], v2[0], v3[0], v4[0] };
    std::array<double, 4> arrayy = { v1[1], v2[1], v3[1], v4[1] };
    std::array<double, 4> arrayz = { v1[2], v2[2], v3[2], v4[2] };

    min[0] = *std::min_element(arrayx.begin(), arrayx.end());
    min[1] = *std::min_element(arrayy.begin(), arrayy.end());
    min[2] = *std::min_element(arrayz.begin(), arrayz.end());

    max[0] = *std::max_element(arrayx.begin(), arrayx.end());
    max[1] = *std::max_element(arrayy.begin(), arrayy.end());
    max[2] = *std::max_element(arrayz.begin(), arrayz.end());
}

void
TetrahedralMesh::setTetrahedraVertices(const std::vector<TetraArray>& tetrahedra)
{
    m_tetrahedraVertices = tetrahedra;
}

const std::vector<TetrahedralMesh::TetraArray>&
TetrahedralMesh::getTetrahedraVertices() const
{
    return m_tetrahedraVertices;
}

const TetrahedralMesh::TetraArray&
TetrahedralMesh::getTetrahedronVertices(const size_t& tetId) const
{
    return m_tetrahedraVertices.at(tetId);
}

size_t
TetrahedralMesh::getNumTetrahedra() const
{
    return m_tetrahedraVertices.size();
}

// std::unique_ptr<TetrahedralMesh>
std::shared_ptr<TetrahedralMesh>
TetrahedralMesh::createUniformMesh(const Vec3d& aabbMin, const Vec3d& aabbMax, const size_t nx,
                                   const size_t ny, const size_t nz)
{
    Vec3d h = { (aabbMax[0] - aabbMin[0]) / nx,
                (aabbMax[1] - aabbMin[1]) / ny,
                (aabbMax[2] - aabbMin[2]) / nz };
    LOG_IF(FATAL, (h[0] <= 0.0 || h[1] <= 0.0 || h[2] <= 0.0)) << "Invalid bounding box";

    size_t numVertices = (nx + 1) * (ny + 1) * (nz + 1);

    // std::vector<Vec3d> coords;
    StdVectorOfVec3d coords;
    coords.resize(numVertices);
    size_t cnt = 0;

    for (size_t k = 0; k < nz + 1; ++k)
    {
        double z = aabbMin[2] + k * h[2];
        for (size_t j = 0; j < ny + 1; ++j)
        {
            double y = aabbMin[1] + j * h[1];
            for (size_t i = 0; i < nx + 1; ++i)
            {
                double x = aabbMin[0] + i * h[0];
                coords[cnt] = { x, y, z };
                ++cnt;
            }
        }
    }

    const size_t numDiv  = 6;
    size_t       numTets = numDiv * nx * ny * nz;

    std::vector<TetrahedralMesh::TetraArray> vertices;
    vertices.resize(numTets);
    cnt = 0;
    std::vector<size_t> indx(8);

    for (size_t k = 0; k < nz; ++k)
    {
        for (size_t j = 0; j < ny; ++j)
        {
            for (size_t i = 0; i < nx; ++i)
            {
                indx[3] = i + j * (nx + 1) + k * (nx + 1) * (ny + 1);
                indx[2] = indx[3] + 1;
                indx[0] = indx[3] + nx + 1;
                indx[1] = indx[0] + 1;
                indx[4] = indx[0] + (nx + 1) * (ny + 1);
                indx[5] = indx[1] + (nx + 1) * (ny + 1);
                indx[6] = indx[2] + (nx + 1) * (ny + 1);
                indx[7] = indx[3] + (nx + 1) * (ny + 1);
                vertices[cnt + 0] = { indx[0], indx[2], indx[3], indx[6] };
                vertices[cnt + 1] = { indx[0], indx[3], indx[7], indx[6] };
                vertices[cnt + 2] = { indx[0], indx[7], indx[4], indx[6] };
                vertices[cnt + 3] = { indx[0], indx[5], indx[6], indx[4] };
                vertices[cnt + 4] = { indx[1], indx[5], indx[6], indx[0] };
                vertices[cnt + 5] = { indx[1], indx[6], indx[2], indx[0] };
                cnt += numDiv;
            }
        }
    }

    auto mesh = std::make_shared<TetrahedralMesh>();
    // auto mesh = std::unique_ptr<TetrahedralMesh>(new TetrahedralMesh());

    mesh->initialize(coords, vertices);
    return mesh;
}

#if 0
std::shared_ptr<TetrahedralMesh>
TetrahedralMesh::createEnclosingMesh(const SurfaceMesh& surfMesh, const size_t nx, const size_t ny,
                                     const size_t nz)
{
    // Given the index of a tet, return the corresponding index of the hex
    auto tetIdToHexId = [](const size_t tetId) { return tetId / 5; }

                        Vec3d aabbMin, aabbMax;
    const double              paddingPerc = 10.0;
    PointSet.computeBoundingBox(aabbMin, aabbMax, paddingPerc);
    const Vec3d h = { (aabbMax[0] - aabbMin[0]) / nx,
                      (aabbMax[1] - aabbMin[1]) / ny,
                      (aabbMax[2] - aabbMin[2]) / nz };

    auto findHexId = [&aabbMin, &aabbMax, &h](const Vec3d& xyz) {
                         size_t idX = (xyz[0] - aabbMin[0]) / h[0];
                         size_t idY = (xyz[1] - aabbMin[1]) / h[1];
                         size_t idZ = (xyz[2] - aabbMin[2]) / h[2];
                         return { idX, idY, idZ };
                     };

    auto uniformMesh = createUniformMesh(aabbMin, aabbMax, nx, ny, nz);

    std::vector<bool> enclosePoint(uniformMesh.getNumHexahedra(), false);

    // ParallelUtils::parallelFor(surfMesh.getNumVertices(), [&](const size_t vid) {
    //     auto xyz = surfMesh.getVertexPosition(vid);
    //     size_t idX = (xyz[0] - aabbMin[0]) / h[0];
    //     size_t idY = (xyz[1] - aabbMin[1]) / h[1];
    //     size_t idZ = (xyz[2] - aabbMin[2]) / h[2];
    //     size_t hexId = idX + idY *nx + idZ * nx*ny;
    //     size_t tetId0 = 5*hexId;
    //     size_t tetId1 = tetId0 + 5;
    //
    //
    //     static ParallelUtils::SpinLock lock;
    //     lock.lock();
    //     for (size_t id = tetId0; id<tetId1; ++id)
    //     {
    //         enclosePoint[id] = true;
    //     }
    //     lock.unlock();
    // });

    for (size_t vid = 0; vid < surfMesh.getNumVertices(); ++vid)
    {
        auto   xyz    = surfMesh.getVertexPosition(vid);
        size_t idX    = (xyz[0] - aabbMin[0]) / h[0];
        size_t idY    = (xyz[1] - aabbMin[1]) / h[1];
        size_t idZ    = (xyz[2] - aabbMin[2]) / h[2];
        size_t hexId  = idX + idY * nx + idZ * nx * ny;
        size_t tetId0 = 5 * hexId;
        size_t tetId1 = tetId0 + 5;

        for (size_t i = tetId0; i < tetId1; ++i)
        {
            enclosePoint[i] = true;
        }
    }
}

#endif

std::shared_ptr<TetrahedralMesh>
TetrahedralMesh::createTetrahedralMeshCover(std::shared_ptr<SurfaceMesh> surfMesh, const size_t nx, const size_t ny,
                                            const size_t nz)
{
    Vec3d aabbMin, aabbMax;

    // create a background mesh
    surfMesh->computeBoundingBox(aabbMin, aabbMax, 1. /*percentage padding*/);
    auto uniformMesh = createUniformMesh(aabbMin, aabbMax, nx, ny, nz);

    // ray-tracing
    const auto& coords = uniformMesh->getVertexPositions();
    auto        insideSurfMesh = surfMesh->markPointsInsideAndOut(coords);

    // label elements
    std::vector<bool> validTet(uniformMesh->getNumTetrahedra(), false);
    std::vector<bool> validVtx(uniformMesh->getNumVertices(), false);

    // TetrahedralMesh::WeightsArray weights = {0.0, 0.0, 0.0, 0.0};
    const Vec3d h = { (aabbMax[0] - aabbMin[0]) / nx,
                      (aabbMax[1] - aabbMin[1]) / ny,
                      (aabbMax[2] - aabbMin[2]) / nz };

    // TODO: can be parallelized by make NUM_THREADS copies of validTet, or use atomic op on validTet
    auto labelEnclosingTet = [&surfMesh, &uniformMesh, &aabbMin, &h, nx, ny, nz, &validTet](const size_t i)
                             {
                                 const auto& xyz = surfMesh->getVertexPosition(i);
                                 // find the hex that encloses the point;
                                 size_t idX   = (xyz[0] - aabbMin[0]) / h[0];
                                 size_t idY   = (xyz[1] - aabbMin[1]) / h[1];
                                 size_t idZ   = (xyz[2] - aabbMin[2]) / h[2];
                                 size_t hexId = idX + idY * nx + idZ * nx * ny;

                                 // the index range of tets inside the enclosing hex
                                 const int numDiv = 6;
                                 size_t    tetId0 = numDiv * hexId;
                                 size_t    tetId1 = tetId0 + numDiv;

                                 // loop over the tets to find the enclosing tets
                                 bool                          found   = false;
                                 TetrahedralMesh::WeightsArray weights = { 0.0, 0.0, 0.0, 0.0 };
                                 for (size_t id = tetId0; id < tetId1; ++id)
                                 {
                                     uniformMesh->computeBarycentricWeights(id, xyz, weights);

                                     if ((weights[0] >= 0) && (weights[1] >= 0) && (weights[2] >= 0) && (weights[3] >= 0))
                                     {
                                         validTet[id] = true;
                                         found = true;
                                         break;
                                     }
                                 }

                                 // TODO: not sure how to do thread-safe logging
                                 CHECK(found) << "Failed to find the enclosing tetrahedron";
                             };

    // a customized approach to find the enclosing tet for each surface points
    for (size_t i = 0; i < surfMesh->getNumVertices(); ++i)
    {
        labelEnclosingTet(i);
    }

    for (size_t i = 0; i < validTet.size(); ++i)
    {
        const auto& verts = uniformMesh->getTetrahedronVertices(i);
        if (insideSurfMesh[verts[0]]
            || insideSurfMesh[verts[1]]
            || insideSurfMesh[verts[2]]
            || insideSurfMesh[verts[3]])
        {
            validTet[i] = true;
        }
    }

    size_t numElems = 0;
    for (size_t i = 0; i < validTet.size(); ++i)
    {
        const auto& verts = uniformMesh->getTetrahedronVertices(i);
        if (validTet[i])
        {
            validVtx[verts[0]] = true;
            validVtx[verts[1]] = true;
            validVtx[verts[2]] = true;
            validVtx[verts[3]] = true;
            ++numElems;
        }
    }

    // discard useless vertices, and build old-to-new index map
    size_t numVerts = 0;
    for (auto b : validVtx)
    {
        if (b)
        {
            ++numVerts;
        }
    }

    StdVectorOfVec3d    newCoords(numVerts);
    std::vector<size_t> oldToNew(coords.size(), std::numeric_limits<size_t>::max());
    size_t              cnt = 0;

    for (size_t i = 0; i < validVtx.size(); ++i)
    {
        if (validVtx[i])
        {
            newCoords[cnt] = coords[i];
            oldToNew[i]    = cnt;
            ++cnt;
        }
    }

    // update tet-to-vert connectivity
    std::vector<TetraArray> newIndices(numElems);
    cnt = 0;
    for (size_t i = 0; i < uniformMesh->getNumTetrahedra(); ++i)
    {
        if (validTet[i])
        {
            const auto oldIndices = uniformMesh->getTetrahedronVertices(i);

            newIndices[cnt][0] = oldToNew[oldIndices[0]];
            newIndices[cnt][1] = oldToNew[oldIndices[1]];
            newIndices[cnt][2] = oldToNew[oldIndices[2]];
            newIndices[cnt][3] = oldToNew[oldIndices[3]];

            ++cnt;
        }
    }

    // ready to create the final mesh
    auto mesh = std::make_shared<TetrahedralMesh>();
    mesh->initialize(newCoords, newIndices);

    return mesh;
}
}  // namespace imstk
