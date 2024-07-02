/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkPbdObjectCellRemoval.h"
#include "imstkPbdConstraintContainer.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkCellMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkPointwiseMap.h"
#include "imstkTetrahedralMesh.h"

#include <iostream>

namespace
{
// This is from `ExtractSurfaceMesh` in TetrahedralMesh, did not find whether
// the vertices in a tetrahedron are suppose to be in a specific order
static const std::array<imstk::Vec3i, 4> facePattern = {
    imstk::Vec3i(0, 1, 2), imstk::Vec3i(0, 1, 3), imstk::Vec3i(0, 2, 3), imstk::Vec3i(1, 2, 3)
};

bool
isOn(imstk::Vec3i tri, imstk::Vec4i tet)
{
    const auto& triArray = tet.array();
    for (int i = 0; i < 3; ++i)
    {
        if (!(triArray == tri[i]).any())
        {
            return false;
        }
    }
    return true;
}

imstk::Vec3i
getFace(const std::array<imstk::Vec3i, 4>& pattern, const imstk::Vec4i& tet, int index)
{
    return imstk::Vec3i(
                        tet[pattern[index][0]],
                        tet[pattern[index][1]],
                        tet[pattern[index][2]]);
}

bool
tryGetSharedFace(imstk::Vec4i left, imstk::Vec4i right, std::pair<imstk::Vec3i, imstk::Vec3i>& faces)
{
    for (int leftIndex = 0; leftIndex < 4; ++leftIndex)
    {
        auto leftFace = getFace(facePattern, left, leftIndex);
        for (int rightIndex = 0; rightIndex < 4; ++rightIndex)
        {
            auto rightFace = getFace(facePattern, right, rightIndex);
            bool isSame    = true;
            for (int i = 0; i < 3; ++i)
            {
                if (!(leftFace.array() == rightFace[i]).any())
                {
                    isSame = false;
                    break;
                }
            }
            if (isSame)
            {
                faces.first  = leftFace;
                faces.second = rightFace;
                return true;
            }
        }
    }
    return false;
}
} // namespace

namespace imstk
{
PbdObjectCellRemoval::PbdObjectCellRemoval(std::shared_ptr<PbdObject> pbdObj, OtherMeshUpdateType alsoUpdate) :
    m_obj(pbdObj),
    m_updateMode(alsoUpdate)
{
    // Add checks here as needed

    // Get mesh and add dummy vertex for storing removed cell
    m_mesh = std::dynamic_pointer_cast<AbstractCellMesh>(pbdObj->getPhysicsGeometry());
    addDummyVertex(m_mesh);

    // Update fixed node ids to account for dummy vertex at index zero
    for (int fixedId = 0; fixedId < m_obj->getPbdBody()->fixedNodeIds.size(); fixedId++)
    {
        m_obj->getPbdBody()->fixedNodeIds[fixedId]++;
    }

    // Reinitialize to account for new dummy vertex
    pbdObj->initialize();

    // Note: maps on the pbdObject are no longer valid after this point
    int alsoUpdateInt = static_cast<int>(alsoUpdate);

    if (alsoUpdateInt != 0)
    {
        auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_mesh);

        if (tetMesh != nullptr)
        {
            if (pbdObj->getCollidingGeometry() == pbdObj->getVisualGeometry() && alsoUpdate != OtherMeshUpdateType::None)
            {
                alsoUpdate = OtherMeshUpdateType::Collision;
            }

            tetMesh->computeVertexToCellMap();

            if ((alsoUpdateInt & static_cast<int>(OtherMeshUpdateType::Collision)) != 0)
            {
                auto mesh = std::dynamic_pointer_cast<SurfaceMesh>(pbdObj->getCollidingGeometry());
                auto map  = std::dynamic_pointer_cast<PointwiseMap>(pbdObj->getPhysicsToCollidingMap());
                if (mesh == nullptr)
                {
                    LOG(WARNING) << "Collision mesh not a surface mesh, can't maintain for cell removal";
                }
                else if (map == nullptr)
                {
                    LOG(WARNING) << "PhysicsToCollidingMap not a Pointwise map, can't maintain for cell removal";
                }
                else
                {
                    setupForExtraMeshUpdates(mesh, map);
                }
            }

            if ((alsoUpdateInt & (static_cast<int>(OtherMeshUpdateType::VisualSeparateVertices) |
                                  static_cast<int>(OtherMeshUpdateType::VisualReuseVertices))) != 0)
            {
                auto mesh = std::dynamic_pointer_cast<SurfaceMesh>(pbdObj->getVisualGeometry());
                auto map  = std::dynamic_pointer_cast<PointwiseMap>(pbdObj->getPhysicsToVisualMap());
                if (mesh == nullptr)
                {
                    LOG(WARNING) << "Visual mesh not a surface mesh, can't maintain for cell removal";
                }
                else if (map == nullptr)
                {
                    LOG(WARNING) << "PhysicsToVisualMap not a Pointwise map, can't maintain for cell removal";
                }
                else
                {
                    setupForExtraMeshUpdates(mesh, map);
                    m_linkedMeshData.back().newVertexOnSplit = alsoUpdate == OtherMeshUpdateType::VisualSeparateVertices;
                }
            }
        }
        else
        {
            LOG(WARNING) << "Underlying mesh not a tet mesh, cannot maintain other meshes";
            m_updateMode = OtherMeshUpdateType::None;
        }
    }
}

void
PbdObjectCellRemoval::removeCellOnApply(int cellId)
{
    m_cellsToRemove.push_back(cellId);
}

void
PbdObjectCellRemoval::apply()
{
    if (m_cellsToRemove.empty())
    {
        return;
    }

    // Only for tetmeshes...
    for (auto& data : m_linkedMeshData)
    {
        updateMesh(data);
    }

    removeConstraints();

    m_removedCells.insert(m_removedCells.end(), m_cellsToRemove.begin(), m_cellsToRemove.end());
    std::sort(m_removedCells.begin(), m_removedCells.end());
    m_cellsToRemove.clear();

    fixup();
}

void
PbdObjectCellRemoval::updateMesh(LinkedMeshData& data)
{                                                                                           // m_mesh->getAbstractCells()->getNumberOfComponents();
    auto cellVerts = std::dynamic_pointer_cast<DataArray<int>>(m_mesh->getAbstractCells()); // underlying 1D array

    constexpr int vertsPerTri = 3;

    auto& triangles = *(data.surfaceMesh->getCells());
    auto& vertices  = *(data.surfaceMesh->getVertexPositions());

    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_mesh);

    // "Remove" all triangles that are adjacent to tets that are being removed
    for (int cellId : m_cellsToRemove)
    {
        auto range = data.tetToTriMap.equal_range(cellId);
        for (auto item = range.first; item != range.second; ++item)
        {
            triangles[item->second] = { 0, 0, 0 };
        }
        data.tetToTriMap.erase(cellId);
    }

    const auto& tetrahedra  = *(tetMesh->getCells());
    const auto& tetVertices = *(tetMesh->getVertexPositions());

    // Add all triangles that are on neighboring faces but NOT on other removed tets
    for (int cellId : m_cellsToRemove)
    {
        // All neighbors of the tetrahedron
        auto range = data.tetAdjancencyMap.equal_range(cellId);
        for (auto item = range.first; item != range.second; ++item)
        {
            auto otherTetIndex = item->second.first;

            // Don't add if the other tet is in the progress of being removed or has already been removed
            if (std::find(m_cellsToRemove.cbegin(), m_cellsToRemove.cend(), otherTetIndex) != m_cellsToRemove.cend()
                || std::find(m_removedCells.cbegin(), m_removedCells.cend(), otherTetIndex) != m_removedCells.cend())
            {
                continue;
            }

            auto faceOnTetMesh = item->second.second;
            // Add Vertices
            Vec3i triangle = { 0, 0, 0 };
            for (int i = 0; i < vertsPerTri; ++i)
            {
                int tetVertexIndex = faceOnTetMesh[i];

                // Check if the tet vertex is already on the surface mesh
                auto found = data.tetVertToTriVertMap.find(tetVertexIndex);
                // Reuse vertex _if_ its found
                // For visual meshes we want a new vertex so a different uv coordinate
                // can be calculated
                if (data.newVertexOnSplit || found == data.tetVertToTriVertMap.cend())
                {
                    triangle[i] = vertices.size();
                    vertices.push_back(tetVertices[tetVertexIndex]);
                    data.tetVertToTriVertMap[tetVertexIndex] = triangle[i];
                    data.map->addNewUniquePoint(triangle[i], tetVertexIndex);
                }
                else
                {
                    triangle[i] = found->second;
                }
            }

            const Vec3d& v0       = tetVertices[faceOnTetMesh[0]];
            const Vec3d& v1       = tetVertices[faceOnTetMesh[1]];
            const Vec3d& v2       = tetVertices[faceOnTetMesh[2]];
            const Vec3d  normal   = ((v1 - v0).cross(v2 - v0));
            const Vec3d  centroid = (v0 + v1 + v2) / 3.0;

            const Vec4i& tet = tetrahedra[otherTetIndex];
            const Vec3d  tetCentroid =
                (tetVertices[tet[0]] +
                 tetVertices[tet[1]] +
                 tetVertices[tet[2]] +
                 tetVertices[tet[3]]) / 4.0;

            int triangleIndex = triangles.size();

            // If the normal is correct, it should be pointing in the same direction as the (face centroid - tetCentroid)
            if (normal.dot(centroid - tetCentroid) < 0)
            {
                triangles.push_back({ triangle[0], triangle[2], triangle[1] });
            }
            else
            {
                triangles.push_back(triangle);
            }
            data.tetToTriMap.insert({ otherTetIndex, triangleIndex });
        }
        data.tetAdjancencyMap.erase(cellId);
    }

    for (int cellId : m_cellsToRemove)
    {
        tetMesh->setTetrahedraAsRemoved(cellId);
    }

    if (!m_cellsToRemove.empty())
    {
        data.surfaceMesh->postModified();
        data.surfaceMesh->getVertexPositions()->postModified();
        data.surfaceMesh->getCells()->postModified();
    }
}

void
PbdObjectCellRemoval::removeConstraints()
{
    // Mesh Data
    int       bodyId       = m_obj->getPbdBody()->bodyHandle;
    const int vertsPerCell = m_mesh->getAbstractCells()->getNumberOfComponents();
    auto      cellVerts    = std::dynamic_pointer_cast<DataArray<int>>(m_mesh->getAbstractCells());  // underlying 1D array

    // Constraint Data
    std::shared_ptr<PbdConstraintContainer>            constraintsPtr = m_obj->getPbdModel()->getConstraints();
    const std::vector<std::shared_ptr<PbdConstraint>>& constraints    = constraintsPtr->getConstraints();

    for (int i = 0; i < m_cellsToRemove.size(); i++)
    {
        int cellId = m_cellsToRemove[i];

        // Find and remove the associated constraints
        for (auto j = constraints.begin(); j != constraints.end();)
        {
            const std::vector<PbdParticleId>& vertexIds = (*j)->getParticles();

            // Dont remove any constraints that do not involve
            // every node of the cell
            if (vertexIds.size() < vertsPerCell)
            {
                j++;
                continue;
            }

            // Check that constraint involves this body and get associated vertices
            bool isBody = false;
            for (int cVertId = 0; cVertId < vertexIds.size(); cVertId++)
            {
                if (vertexIds[cVertId].first == bodyId)
                {
                    isBody = true;
                    break;
                }
            }

            // Skip if body is not involved
            if (isBody == false)
            {
                j++;
                continue;
            }

            // Check if the constraint involves ONLY the body of interest
            // This is used for removing constraints that connect two or more bodies
            bool isOnlyBody = true;
            for (int cVertId = 0; cVertId < vertexIds.size(); cVertId++)
            {
                if (vertexIds[cVertId].first != bodyId)
                {
                    isOnlyBody = false;
                    break;
                }
            }

            // Sets for comparing constraint vertices to cell vertices
            std::unordered_set<int> constraintVertIds;
            std::unordered_set<int> cellVertIds;
            // Fill in sets
            for (int vertId = 0; vertId < vertsPerCell; vertId++)
            {
                cellVertIds.insert((*cellVerts)[cellId * vertsPerCell + vertId]);
            }

            for (int cVertId = 0; cVertId < vertexIds.size(); cVertId++)
            {
                constraintVertIds.insert(vertexIds[cVertId].second);
            }

            // Check if cell nodes are a subset of the nodes used for the constraint
            bool isSubset = true;
            for (const auto& elem : constraintVertIds)
            {
                if (cellVertIds.find(elem) == cellVertIds.end())
                {
                    isSubset = false;
                    break;
                }
            }

            // Handle constraints connecting two bodies when an associated cell is deleted.
            bool isMultiBodyConstraint = false;
            if (isOnlyBody == false)
            {
                for (int cVertId = 0; cVertId < vertexIds.size(); cVertId++)
                {
                    if (vertexIds[cVertId].first == bodyId && cellVertIds.find(vertexIds[cVertId].second) != cellVertIds.end())
                    {
                        isMultiBodyConstraint = true;
                    }
                }
            }

            if (isSubset == true || (isMultiBodyConstraint && isSubset == false))
            {
                j = constraintsPtr->eraseConstraint(j);
            }
            else
            {
                j++;
            }
        }

        // Set removed cell to dummy vertex
        for (int k = 0; k < vertsPerCell; k++)
        {
            (*cellVerts)[cellId * vertsPerCell + k] = 0;
        }
    }

    if (m_cellsToRemove.size() > 0)
    {
        // Note: if the collision geometry is different from the physics geometry the collision geometry
        // will need to be updated. This is not yet implemented.
        m_mesh->getAbstractCells()->postModified();
        std::cout << "Removing " << m_cellsToRemove.size() << " Cells";
    }
}

void
PbdObjectCellRemoval::fixup()
{
    auto volumeMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_mesh);
    if (volumeMesh == nullptr)
    {
        return;
    }
    // Gather all the actual points in the tetrahedron
    std::unordered_set<int> validTetVertices;
    for (const auto& tet : *volumeMesh->getTetrahedraIndices())
    {
        validTetVertices.insert(tet[0]);
        validTetVertices.insert(tet[1]);
        validTetVertices.insert(tet[2]);
        validTetVertices.insert(tet[3]);
    }

    for (auto& meshData : m_linkedMeshData)
    {
        auto  map       = meshData.map->getMap();
        auto& triangles = *(meshData.surfaceMesh->getTriangleIndices());
        for (auto& tri : triangles)
        {
            for (int j = 0; j < 3; j++)
            {
                if (tri[j] != 0 && validTetVertices.find(map[tri[j]]) == validTetVertices.end())
                {
                    tri = { 0, 0, 0 };
                    break;
                }
            }
        }
    }
}

void
PbdObjectCellRemoval::setupForExtraMeshUpdates(std::shared_ptr<SurfaceMesh> surfaceMesh, std::shared_ptr<PointwiseMap> map)
{
    auto tetMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_mesh);

    CHECK(tetMesh != nullptr);
    CHECK(surfaceMesh != nullptr);
    CHECK(map != nullptr);

    LinkedMeshData data;
    data.surfaceMesh = surfaceMesh;
    data.map = map;

    std::multimap<int, int>                   tetToTriMap;
    std::multimap<int, std::pair<int, Vec3i>> tetAdjancencyMap;

    const auto& tetrahedra = *(tetMesh->getCells());
    const auto& triangles  = *(surfaceMesh->getCells());

    // TODO need to check if that vertex isn't already there
    addDummyVertex(surfaceMesh);
    map->compute();

    // Create reverse lookup map for finding existing vertices
    for (auto entry : map->getMap())
    {
        data.tetVertToTriVertMap[entry.second] = entry.first;
    }

    // Create a map that collects connects tetrahdra to
    // triangles on the mesh. This way the triangles can be
    // removed when the tetrahedron is removed
    const auto& triVertToTetVertMap = map->getMap();

    for (int tetIndex = 0; tetIndex < tetrahedra.size(); ++tetIndex)
    {
        const auto& tet = tetrahedra[tetIndex];

        // Find faces on surfacemesh that are on the current tet
        for (int triIndex = 0; triIndex < triangles.size(); ++triIndex)
        {
            Vec3i triangle = triangles[triIndex];
            bool  allInTet = true;
            for (int i = 0; i < 3; ++i)
            {
                auto found = triVertToTetVertMap.find(triangle[i]);
                if (found == triVertToTetVertMap.cend())
                {
                    allInTet = false;
                    break;
                }
                triangle[i] = found->second;
            }

            if (allInTet && isOn(triangle, tet))
            {
                tetToTriMap.insert({ tetIndex, triIndex });
            }
        }

        // Build a structure where adjacent tetrahedra and their faces can be looked up
        // This way a new face can be created on the adjacent tetrahedron when it's neighbor
        // is removed
        for (int otherTetIndex = 0; otherTetIndex < tetrahedra.size(); ++otherTetIndex)
        {
            if (tetIndex == otherTetIndex)
            {
                continue;
            }

            // If we find a face in the adjacency map, we've already been there
            auto range = tetAdjancencyMap.equal_range(otherTetIndex);
            auto found = std::find_if(range.first, range.second, [tetIndex](const auto& item) { return item.second.first == tetIndex; });
            if (found != range.second)
            {
                continue;
            }

            auto                    other = tetrahedra[otherTetIndex];
            std::pair<Vec3i, Vec3i> faces;
            if (tryGetSharedFace(tet, other, faces))
            {
                tetAdjancencyMap.insert({ tetIndex, { otherTetIndex, faces.second } });
                tetAdjancencyMap.insert({ otherTetIndex, { tetIndex, faces.first } });
            }
        }
    }

    data.tetToTriMap      = tetToTriMap;
    data.tetAdjancencyMap = tetAdjancencyMap;
    m_linkedMeshData.push_back(data);
}

void
PbdObjectCellRemoval::addDummyVertexPointSet(std::shared_ptr<PointSet> pointSet)
{
    // Add a dummy vertex to the vertices
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pointSet->getVertexPositions();
    VecDataArray<double, 3>&                 vertices    = *verticesPtr;
    vertices.resize(vertices.size() + 1);

    for (int i = vertices.size() - 1; i >= 1; i--)
    {
        vertices[i] = vertices[i - 1];
    }

    // Note: may cause collision issues
    vertices[0] = Vec3d(0.0, 0.0, 0.0);
    pointSet->setInitialVertexPositions(std::make_shared<VecDataArray<double, 3>>(*verticesPtr));
}

void
PbdObjectCellRemoval::addDummyVertex(std::shared_ptr<AbstractCellMesh> mesh)
{
    addDummyVertexPointSet(mesh);

    // Mesh data
    const int vertsPerCell = mesh->getAbstractCells()->getNumberOfComponents();
    auto      cellVerts    = std::dynamic_pointer_cast<DataArray<int>>(mesh->getAbstractCells());

    // Then shift all indices by 1
    for (int cellId = 0; cellId < mesh->getNumCells(); cellId++)
    {
        for (int vertId = 0; vertId < vertsPerCell; vertId++)
        {
            (*cellVerts)[cellId * vertsPerCell + vertId]++;
        }
    }
}
} // namespace imstk
