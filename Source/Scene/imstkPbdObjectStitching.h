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

#include "imstkSceneObject.h"
#include "imstkMath.h"
#include "imstkPbdCollisionConstraint.h"

#include <unordered_map>

namespace imstk
{
class OneToOneMap;
class PbdCollisionConstraint;
class PbdObject;
class PointSet;
class PickingAlgorithm;

///
/// \class PbdObjectStitching
///
/// \brief This class defines stitching of a PbdObject via different
/// picking methods. Ray based stitching is default.
///
class PbdObjectStitching : public SceneObject
{
protected:
    enum class StitchMode
    {
        Vertex,     // Stitch two vertices together
        Cell,       // Stitch two cells together
        RayPoint,   // Stitch two points on two cells together found via rays
        RayCell     // Stitch two cells found via rays
    };

public:
    PbdObjectStitching(std::shared_ptr<PbdObject> obj1);
    ~PbdObjectStitching() override = default;

    const std::string getTypeName() const override { return "PbdObjectStitching"; }

    ///
    /// \brief Set/Get the stiffness, 0-1 value that alters the step size in
    /// the solver
    ///@{
    void setStiffness(const double stiffness) { m_stiffness = stiffness; }
    double getStiffness() const { return m_stiffness; }
    ///@}

   /* ///
    /// \brief Begin a vertex grasp (picking will begin on the next update)
    /// \param Geometry attached/grasped too
    ///
    void beginVertexGrasp(std::shared_ptr<AnalyticalGeometry> geometry);

    ///
    /// \brief Begin a cell grasp (picking will begin on the next update)
    /// \param Geometry attached/grasped too
    /// \param The intersection type/class name
    ///
    void beginCellGrasp(std::shared_ptr<AnalyticalGeometry> geometry, std::string cdType);*/

    ///
    /// \brief Begin a ray point grasp (picking will begin on the next update)
    /// \param Global space ray start
    /// \param Global space ray direction
    ///
    void beginRayPointStitch(const Vec3d& rayStart, const Vec3d& rayDir, const double maxDist = -1.0);

    /* ///
     /// \brief Begin a ray point grasp (picking will begin on the next update)
     /// \param Geometry attached/grasped too
     /// \param Global space ray start
     /// \param Global space ray direction
     ///
     void beginRayCellStitch(std::shared_ptr<AnalyticalGeometry> geometry,
         const Vec3d& rayStart, const Vec3d& rayDir, const double maxDist = -1.0);*/

    ///
    /// \brief Compute/generate the constraints for stitching
    ///
    void addStitchConstraints();

    ///
    /// \brief Clears all the stitches
    ///
    void removeStitchConstraints();

    ///
    /// \brief Add constraint between a point on each element given via
    /// barycentric coordinates
    /// pt position = weightA_0 * ptsA_0 + weightA_1 * ptsA_1 + ...
    ///
    virtual void addConstraint(
        const std::vector<VertexMassPair>& ptsA,
        const std::vector<double>& weightsA,
        const std::vector<VertexMassPair>& ptsB,
        const std::vector<double>& weightsB,
        const double stiffnessA, const double stiffnessB);

    ///
    /// \brief Get/Set the method use for picking, default is CellPicker
    ///@{
    void setPickingAlgorithm(std::shared_ptr<PickingAlgorithm> pickMethod) { m_pickMethod = pickMethod; }
    std::shared_ptr<PickingAlgorithm> getPickingAlgorithm() const { return m_pickMethod; }
    ///@}

    ///
    /// \brief Set a different geometry to pick with and a mapping back to the physics geometry
    /// to select the correct vertices
    ///
    void setGeometryToStitch(std::shared_ptr<Geometry> geomToStitch, std::shared_ptr<OneToOneMap> map)
    {
        m_geomToStitch = geomToStitch;
        m_geometryToStitchMap = map;
    }

    std::shared_ptr<TaskNode> getPickingNode() const { return m_stitchingNode; }

    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    ///
    /// \brief Update picking state, this should move grasp points
    ///
    virtual void updateStitching();

    ///
    /// \brief Update the constraints used for picking
    ///
    void updateConstraints();

protected:
    std::shared_ptr<TaskNode> m_stitchingNode = nullptr;

    std::shared_ptr<Geometry>    m_geomToStitch = nullptr;
    std::shared_ptr<OneToOneMap> m_geometryToStitchMap = nullptr;

    std::shared_ptr<PbdObject> m_objectToStitch = nullptr;

    std::shared_ptr<PickingAlgorithm> m_pickMethod = nullptr;
    StitchMode m_mode = StitchMode::Cell;

    bool m_isStitching = false;
    bool m_isPrevStitching = false;

    /// Stiffness of stitches, when 1 the position is completely moved too the grasp point
    /// when stiffness < 1 it will slowly converge on the grasp point
    double m_stiffness = 0.1;

    bool m_debugGeometry = false; ///< Display debug points & lines for stitches
    bool m_retractingStitch = false; ///< Place a stitch that slowly retracts to 0 after placement

    std::vector<std::shared_ptr<PbdCollisionConstraint>> m_constraints; ///< List of PBD constraints
};
} // namespace imstk