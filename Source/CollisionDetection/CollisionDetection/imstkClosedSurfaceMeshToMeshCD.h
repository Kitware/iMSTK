/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkDataArray.h"
#include "imstkMacros.h"

namespace imstk
{
class CollisionData;
class PointSet;
class SurfaceMesh;

///
/// \class ClosedSurfaceMeshToMeshCD
///
/// \brief Closed mesh to mesh collision with brute force strategy.
/// It can handle closed SurfaceMesh vs PointSet, LineMesh, & SurfaceMesh.
/// Note: This CD method cannot yet automatically determine the closed
/// SurfaceMesh given two unordered inputs. Ensure the second input/B is
/// the closed SurfaceMesh.
///
/// It produces vertex-triangle, edge-edge, vertex-edge, & vertex-vertex
/// collision data.
///
/// It's exact implementation roughly follows along with Pierre Terdiman's
/// "Contact Generation for Meshes" but further described with GJK instead
/// of brute force closest point determination in "Game Physics Pearls"
///
/// It resolves vertices by computing signed distances using the psuedonormal
/// method. This allows it to resolve very deep penetrations.
///
/// If enabled, it may resolve edge-edge contact by brute force as well. This
/// is an extremely costly operation in brute force and is off by default.
/// Additionally it cannot find the globally best edge to resolve too.
///
/// Extrapolation is used past an opening based on the nearest elements normal.
/// So some openings are ok depending on the intention. For instance, a
/// triangle mesh plane is valid, assuming "beneath" the plane is inside and
/// above is outside.
///
class ClosedSurfaceMeshToMeshCD : public CollisionDetectionAlgorithm
{
public:
    ClosedSurfaceMeshToMeshCD();
    ~ClosedSurfaceMeshToMeshCD() override = default;

    IMSTK_TYPE_NAME(ClosedSurfaceMeshToMeshCD)

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
    /// \brief Set padding to the broad phase (AABB tests)
    ///@{
    void setPadding(const Vec3d& padding) { m_padding = padding; }
    const Vec3d& getPadding() const { return m_padding; }
    ///@}

    ///
    /// \brief Any edges with vertices not within this proximity will not have
    /// edges checked. This can be used to greatly reduce the # of edge-edge checks
    ///@{
    void setProximity(const double proximity) { m_proximity = proximity; }
    double getProximity() const { return m_proximity; }
///@}

    ///
    /// \brief Get/Set whether to do AABB broad phase. The volume bounded by the object
    /// could be larger than the volume of the pointset. For instance triangle on
    /// a single plane vs a point
    ///
    void setDoBroadPhase(const bool doBroadPhase) { m_doBroadPhase = doBroadPhase; }
    bool getDoBroadPhase() const { return m_doBroadPhase; }
///

protected:
    ///
    /// \brief Compute collision data for AB simultaneously
    ///
    void computeCollisionDataAB(
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
    bool m_doBroadPhase = true;

    std::vector<bool> m_vertexInside;
    DataArray<double> m_signedDistances;
    Vec3d  m_padding   = Vec3d(0.001, 0.001, 0.001);
    double m_proximity = -1.0; // Default off -1
};
} // namespace imstk