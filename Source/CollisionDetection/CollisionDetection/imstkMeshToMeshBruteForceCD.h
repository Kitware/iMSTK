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

#pragma once

#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkMacros.h"

namespace imstk
{
class CollisionData;
class PointSet;
class SurfaceMesh;

///
/// \class MeshToMeshBruteForceCD
///
/// \brief Mesh to mesh collision with brute force strategy.
/// It can handle SurfaceMesh vs PointSet, LineMesh, & SurfaceMesh.
///
/// It does not work with self-intersections. It performs static CD
/// to exactly find the neareset elements to resolve. It can handle
/// deep penetrations as well. Designed for closed and manifold meshes
/// but will work for open meshes so long as there is an "inside"/"outside"
/// such as a triangle quad or plane
///
/// It produces edge-edge, vertex-triangle, vertex-edge, vertex-vertex data.
/// Edge-edge is off by default due to cost and effectiveness
///
/// It's exact implementation follows roughly along with Pierre Terdiman's
/// "Contact Generation for Meshes" but further described in with GJK instead
/// of brute force closest point determination in "Game Physics Pearls"
///
/// \todo: Test computing normal of each triangle first when computing signed
/// distances and backface culling
/// \todo: To greatly speed up edge-edge and reduce potential for bad contacts
/// we can use maximum distance parameter which is dealt with during the first
/// pass
///
class MeshToMeshBruteForceCD : public CollisionDetectionAlgorithm
{
public:
    MeshToMeshBruteForceCD();
    virtual ~MeshToMeshBruteForceCD() override = default;

    IMSTK_TYPE_NAME(MeshToMeshBruteForceCD)

public:
    ///
    /// \brief If true, edge to edge contacts will be generated
    /// default true
    ///
    void setGenerateEdgeEdgeContacts(bool genEdgeEdgeContacts) { m_generateEdgeEdgeContacts = genEdgeEdgeContacts; }

    ///
    /// \brief If true, vertex to triangle contacts will be generated
    /// default true
    ///
    void setGenerateVertexTriangleContacts(bool genVertexTriangleContacts) { m_generateVertexTriangleContacts = genVertexTriangleContacts; }

    ///
    /// \brief Set padding to the broad phase
    ///
    void setPadding(const Vec3d& padding) { m_padding = padding; }
    const Vec3d& getPadding() const { return m_padding; }

protected:
    ///
    /// \brief Compute collision data for AB simultaneously
    ///
    virtual void computeCollisionDataAB(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA,
        std::vector<CollisionElement>& elementsB) override;

    void vertexToTriangleTest(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA,
        std::vector<CollisionElement>& elementsB);

    void lineMeshEdgeToTriangleTest(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA,
        std::vector<CollisionElement>& elementsB);

    void surfMeshEdgeToTriangleTest(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA,
        std::vector<CollisionElement>& elementsB);

private:
    ///
    /// \brief Do a broad phase collision check using AABB
    /// \todo: Abstract and make changeable
    ///
    bool doBroadPhaseCollisionCheck(
        std::shared_ptr<Geometry> geomA,
        std::shared_ptr<Geometry> geomB) const;

    bool m_generateEdgeEdgeContacts       = false;
    bool m_generateVertexTriangleContacts = true;

    std::vector<bool> m_vertexInside;
    Vec3d m_padding = Vec3d(0.001, 0.001, 0.001);
};
} // namespace imstk