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

#include "imstkMacros.h"
#include "imstkPbdCollisionHandling.h"
#include "imstkPbdPointTriangleConstraint.h"
#include "imstkSurfaceMesh.h"

#include "NeedleObject.h"
#include "SurfaceInsertionConstraint.h"
#include "ThreadInsertionConstraint.h"

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
///
class NeedlePbdCH : public PbdCollisionHandling
{
protected:

    // Stores data for penetration points, both for the needle and the thread
    struct SuturePenetrationData
    {
        // Triangle ID
        int triId = -1;

        // Triangle vertices
        Vec3d* triVerts[3] = { nullptr, nullptr, nullptr };
        Vec3i triVertIds   = { -1, -1, -1 };

        // Puncture barycentric coordinate on triangle
        Vec3d triBaryPuncturePoint = { 0.0, 0.0, 0.0 };
    };

    Vec3d m_needleDirection = Vec3d::Zero();

    // Flags for which entity is puncturing a triangle
    std::vector<bool> m_isNeedlePunctured;
    std::vector<bool> m_isThreadPunctured;

    // Vector of needle-triangle constraints (one sided, force triangle to follow needle)
    std::vector<std::shared_ptr<SurfaceInsertionConstraint>> pointTriangleConstraints;

    // Vector of thread-triangle constraints (one sided, force thread to follow triangle)
    std::vector<std::shared_ptr<PbdBaryPointToPointConstraint>> m_stitchConstraints;

    // Center of puncture points for stitching constraint
    Vec3d m_stitchCenter = Vec3d::Zero();

    // Storage for penetration data for both the needle and the thread
    std::vector<SuturePenetrationData> m_needlePData;
    std::vector<SuturePenetrationData> m_threadPData;

    // Bool to activate stitching constraint
    bool m_stitch = false;

    // Thread Data
    std::shared_ptr<PbdObject> m_threadObj;
    std::shared_ptr<LineMesh>  m_threadMesh;
    std::shared_ptr<VecDataArray<double, 3>> m_threadVerticesPtr;

    // PBD Tissue Mesh Data
    std::shared_ptr<PbdObject>   m_pbdTissueObj;
    std::shared_ptr<SurfaceMesh> m_surfMesh;
    std::shared_ptr<VecDataArray<double, 3>> m_meshVerticesPtr;

    // Fake velocity for stitch constraint
    Vec3d m_fakeVelocity = Vec3d::Zero();

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

    ///
    /// \brief Add a vertex-triangle constraint
    ///
    void addVTConstraint(
        VertexMassPair ptA,
        VertexMassPair ptB1, VertexMassPair ptB2, VertexMassPair ptB3,
        double stiffnessA, double stiffnessB) override;
};
} // end iMSTK namespace