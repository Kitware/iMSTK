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

#include "imstkSurfaceInsertionConstraint.h"
#include "imstkThreadInsertionConstraint.h"

// using namespace imstk;
namespace imstk
{
class PbdObject;
class PbdBaryPointToPointConstraint;
class LineMesh;
class TetrahedralMesh;

/// \class NeedlePbdCH
///
/// \brief Handles penetration constraints for the needle and the thread by creating a set of puncture points
/// that are used to find the nearest segment on either the needle or the thread and constraining the tissue to the
/// needle, or the thread to the tissue. This class assumes the mesh is not cut or otherwise modified during runtime.
/// WARNING: This class currently assumes a tetrahedral mesh for the physics mesh of the punctureable pbdObject and
/// a triangle mesh for the collision geometry of that object.
class NeedlePbdCH : public PbdCollisionHandling
{
public:
    NeedlePbdCH() = default;
    ~NeedlePbdCH() override = default;

    IMSTK_TYPE_NAME(NeedlePbdCH)

    ///
    /// \brief Initialize interaction data
    ///
    void init(std::shared_ptr<PbdObject> threadObj);

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

    void generateNewPunctureData();
    void addPunctureConstraints();

    void setNeedleToSurfaceStiffness(double stiffness) { m_needleToSurfaceStiffness = stiffness; }
    double getNeedleToSurfaceStiffness() { return m_needleToSurfaceStiffness; }

    void setSurfaceToNeedleStiffness(double stiffness) { m_surfaceToNeedleStiffness = stiffness; }
    double getSurfaceToNeedleStiffness() { return m_surfaceToNeedleStiffness; }

    void setThreadToSurfaceStiffness(double stiffness) { m_needleToSurfaceStiffness = stiffness; }
    double getThreadToSurfaceStiffness() { return m_needleToSurfaceStiffness; }

    void setSurfaceToThreadStiffness(double stiffness) { m_surfaceToThreadStiffness = stiffness; }
    double getSurfaceToThreadStiffness() { return m_surfaceToThreadStiffness; }

    void setPunctureDotThreshold(double threshold) { m_threshold = threshold; }
    double getPunctureDotThreshold() { return m_threshold; }

protected:
    // Stores data for penetration points, both for the needle and the thread
    struct PenetrationData
    {
        // Triangle ID
        int triId = -1;

        // Triangle vertices
        Vec3i triVertIds = { -1, -1, -1 };

        // Puncture barycentric coordinate on triangle
        Vec3d triBaryPuncturePoint = { 0.0, 0.0, 0.0 };

        // Line segment ID of needle or thread
        int segId = -1;
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

    std::vector<PenetrationData> m_threadPData;
    std::vector<PenetrationData> m_needlePData;
    std::vector<PenetrationData> m_punctureData;

    // Bool to activate stitching constraint
    bool m_stitch = false;

    // Thread Data
    std::shared_ptr<PbdObject> m_threadObj;
    std::shared_ptr<LineMesh>  m_threadMesh;

    // PBD Tissue Mesh Data
    std::shared_ptr<PbdObject>   m_pbdTissueObj;
    std::shared_ptr<SurfaceMesh> m_tissueSurfMesh;

    std::shared_ptr<PbdObject> m_needleObj;
    std::shared_ptr<LineMesh>  m_needleMesh;

    // Stiffnesses
    // NOTE: for now, this stiffness times the iteration count needs to be one
    // TODO: Reformat constraints with reprojection step
    double m_needleToSurfaceStiffness = 0.2;
    double m_surfaceToNeedleStiffness = 0.2;
    double m_threadToSurfaceStiffness = 0.2;
    double m_surfaceToThreadStiffness = 0.2;

    bool m_needlePunctured = false;
    bool m_threadPunctured = false;

    // Puncture angle dot product threshold
    double m_threshold = 0.8;

private:

    std::vector<PbdParticleId> m_particles;                          ///< Particles to attach the thread to the needle
    std::vector<std::shared_ptr<PbdConstraint>> m_threadConstraints; ///< Constraints to attach the thread to the needle

    bool didPuncture(const std::vector<CollisionElement>& elementsA, const std::vector<CollisionElement>& elementsB);
};
} // end iMSTK namespace