/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/
#pragma once

#include "imstkSceneObject.h"
#include "imstkMacros.h"
#include "imstkMath.h"

#include <unordered_set>
#include <map>
#include <unordered_map>

namespace imstk
{
class PointSet;
class AbstractCellMesh;
class PbdObject;
class SurfaceMesh;
class PointwiseMap;
///
/// \class PbdObjectCellRemoval
///
/// \brief This class defines a method of removing cells and their associated constraints from a
/// Pbd object.
/// Can update the visual and collision representation when using a tetrahedral mesh as physics object
/// Does not support updating visual and collision representation when the physics mesh is a LineMesh or
/// a SurfaceMesh AND they differ.
/// Note: The two modes for visual meshes exist to support generating new texture coordinates, when reusing
///       a vertex from a visual mesh the vertex should already have uv coordinates assigned to it. Assigning
///       new ones _may_ break the current look of the object as the vertex _may_ still be in use. With the
///       `VisualSeparateVertices` mode, new vertices will be generated for newly exposed surface. This allows
///       the user to assign new uv coordinates to those new points
///
class PbdObjectCellRemoval : public SceneObject
{
public:

    enum class OtherMeshUpdateType
    {
        None = 0,
        Collision = 1,
        VisualReuseVertices = 2,    // If possible reuse a vertex from the visual mesh when removing tetrahedron
        VisualSeparateVertices = 4, // Create a new vertex even if it existed in the visual mesh
        CollisionAndVisualSeparate = Collision | VisualSeparateVertices,
        CollisionAndVisualReused = Collision | VisualReuseVertices
    };

    PbdObjectCellRemoval(std::shared_ptr<PbdObject> pbdObj, OtherMeshUpdateType alsoUpdate = OtherMeshUpdateType::None);
    ~PbdObjectCellRemoval() override = default;

    IMSTK_TYPE_NAME(PbdObjectCellRemoval)

    ///
    /// \brief Adds cell to list of cells to be removed
    ///
    void removeCellOnApply(int cellId);

    ///
    /// \brief removed cells and associated constraints
    ///
    void apply();

    ///
    /// \brief Get ids of cells that have been removed
    ///
    std::vector<int> getRemovedCells() { return m_removedCells; }

protected:

    void removeConstraints();

    void addDummyVertexPointSet(std::shared_ptr<PointSet> pointSet);
    void addDummyVertex(std::shared_ptr<AbstractCellMesh> mesh);
    void fixup();

    std::shared_ptr<PbdObject> m_obj;         ///< Object that cells are removed from
    std::shared_ptr<AbstractCellMesh> m_mesh; ///< Mesh from object cells are removed from
    std::vector<int> m_cellsToRemove;         ///< List of cells to remove, cleared after removal
    std::vector<int> m_removedCells;          ///< Cells that have been removed

private:
    struct LinkedMeshData
    {
        bool newVertexOnSplit = false;
        std::shared_ptr<SurfaceMesh> surfaceMesh;
        std::shared_ptr<PointwiseMap> map;

        /// Maps tet indices to surface mesh triangles
        std::multimap<int, int> tetToTriMap;

        /// Adjacent tets that share a _face_ with the key tet
        /// pair.first is other tet, pair.second is face# on other tet
        std::multimap<int, std::pair<int, Vec3i>> tetAdjancencyMap;

        // The reverse of the PointwiseMap mapping
        std::unordered_map<int, int> tetVertToTriVertMap;
    };

    OtherMeshUpdateType m_updateMode;

    std::vector<LinkedMeshData> m_linkedMeshData;

    void updateMesh(LinkedMeshData& data);
    void setupForExtraMeshUpdates(std::shared_ptr<SurfaceMesh> surfaceMesh, std::shared_ptr<PointwiseMap> map);
};
} // namespace imstk