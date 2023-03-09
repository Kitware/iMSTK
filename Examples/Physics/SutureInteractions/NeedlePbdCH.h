/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdPointTriangleConstraint.h"
#include "imstkSurfaceMesh.h"

#include "SurfaceInsertionConstraint.h"
#include "ThreadInsertionConstraint.h"

// using namespace imstk;
namespace imstk
{
class Entity;
class LineMesh;
class Needle;
class PbdBaryPointToPointConstraint;
class Puncturable;
class TetrahedralMesh;

/// \class NeedlePbdCH
///
/// \brief Handles penetration constraints for the needle and the thread by creating a set of puncture points
/// that are used to find the nearest segment on either the needle or the thread and constraining the tissue to the
/// needle, or the thread to the tissue. This class assumes the mesh is not cut or otherwise modified during runtime.
///
class NeedlePbdCH : public PbdCollisionHandling
{
public:
    NeedlePbdCH() = default;
    ~NeedlePbdCH() override = default;

    IMSTK_TYPE_NAME(NeedlePbdCH)

    void setTissue(std::shared_ptr<Entity> tissueEntity) { m_tissue.entity = tissueEntity.get(); }
    void setNeedle(std::shared_ptr<Entity> needleEntity) { m_needle.entity = needleEntity.get(); }
    void setThread(std::shared_ptr<Entity> threadEntity) { m_thread.entity = threadEntity.get(); }

    ///
    /// \brief Initialize interaction data
    ///
    bool initialize() override;

    ///
    /// \brief Create stitching constraints on button press for four or more puncture points
    ///
    void stitch();

    ///
    /// \brief Handles puncture constraints for both the needle and the thread
    ///
    void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override;

    ///
    /// \brief Add a vertex-triangle constraint
    ///
    void addConstraint_V_T(const ColElemSide& sideA,
                           const ColElemSide& sideB) override;

protected:
    // Stores data for penetration points, both for the needle and the thread
    struct SuturePenetrationData
    {
        // Triangle ID
        int triId = -1;

        // Triangle vertices
        Vec3i triVertIds = { -1, -1, -1 };

        // Puncture barycentric coordinate on triangle
        Vec3d triBaryPuncturePoint = { 0.0, 0.0, 0.0 };
    };

    // Flags for which entity is puncturing a triangle
    std::vector<bool> m_isThreadPunctured;

    // Vector of needle-triangle constraints (one sided, force triangle to follow needle)
    std::vector<std::shared_ptr<SurfaceInsertionConstraint>> pointTriangleConstraints;

    // Vector of thread-triangle constraints (one sided, force thread to follow triangle)
    std::vector<std::shared_ptr<PbdBaryPointToPointConstraint>> m_stitchConstraints;

    // All constraints
    std::vector<std::shared_ptr<PbdConstraint>> m_constraints;
    std::vector<PbdConstraint*> m_solverConstraints;

    // Center of puncture points for stitching constraint
    Vec3d m_stitchCenter = Vec3d::Zero();

    std::vector<SuturePenetrationData> m_threadPData;

    // Bool to activate stitching constraint
    bool m_stitch = false;

    // Thread Data
    struct
    {
        Entity* entity       = nullptr;
        PbdMethod* method    = nullptr;
        LineMesh* threadMesh = nullptr;
    } m_thread;
    // std::shared_ptr<Entity> m_threadObj;
    // std::shared_ptr<LineMesh>  m_threadMesh;

    // PBD Tissue Data
    struct
    {
        Entity* entity        = nullptr;
        Collider* collider    = nullptr;
        PbdMethod* method     = nullptr;
        SurfaceMesh* surfMesh = nullptr;
        std::shared_ptr<Puncturable> puncturable;
    } m_tissue;

    // Needle Data
    struct
    {
        Entity* entity     = nullptr;
        Collider* collider = nullptr;
        std::shared_ptr<Needle> needleComp;
    } m_needle;
    // std::shared_ptr<SurfaceMesh> m_tissueSurfMesh;

    bool m_punctured = false;
};
} // end iMSTK namespace