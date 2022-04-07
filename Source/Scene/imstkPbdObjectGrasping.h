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

#include "imstkCollisionInteraction.h"
#include "imstkMacros.h"
#include "imstkMath.h"
#include "imstkPbdCollisionConstraint.h"

#include <unordered_map>

namespace imstk
{
class AnalyticalGeometry;
class PbdCollisionConstraint;
class PbdObject;
class PickingAlgorithm;
class PointSet;
class PointwiseMap;

///
/// \class PbdObjectGrasping
///
/// \brief This class defines grasping of a PbdObject via different
/// picking methods. Where grasping is define as grabbing & attaching
/// of a PbdObject's mesh to points.
///
/// Given an input PickData the appropriate grasping will be produced.
///
class PbdObjectGrasping : public SceneObject
{
protected:
    enum class GraspMode
    {
        Vertex,     // Grab a vertex (most performant)
        Cell,       // Grab an entire cell/s
        RayPoint,   // Grab a point on the nearest cell along the ray
        RayCell     // Grab a cell along the ray
    };

public:
    PbdObjectGrasping(std::shared_ptr<PbdObject> obj1);
    ~PbdObjectGrasping() override = default;

    IMSTK_TYPE_NAME(PbdObjectGrasping)

    ///
    /// \brief Set/Get the stiffness, 0-1 value that alters the step size in
    /// the solver
    ///@{
    void setStiffness(const double stiffness) { m_stiffness = stiffness; }
    double getStiffness() const { return m_stiffness; }
    ///@}

    ///
    /// \brief Begin a vertex grasp (picking will begin on the next update)
    /// \param Geometry attached/grasped too
    ///
    void beginVertexGrasp(std::shared_ptr<AnalyticalGeometry> geometry);

    ///
    /// \brief Begin a cell grasp (picking will begin on the next update)
    /// \param Geometry attached/grasped too
    /// \param The intersection type/class name
    ///
    void beginCellGrasp(std::shared_ptr<AnalyticalGeometry> geometry, std::string cdType);

    ///
    /// \brief Begin a ray point grasp (picking will begin on the next update)
    /// \param Geometry attached/grasped too
    /// \param Global space ray start
    /// \param Global space ray direction
    ///
    void beginRayPointGrasp(std::shared_ptr<AnalyticalGeometry> geometry,
                            const Vec3d& rayStart, const Vec3d& rayDir, const double maxDist = -1.0);

    ///
    /// \brief Begin a ray point grasp (picking will begin on the next update)
    /// \param Geometry attached/grasped too
    /// \param Global space ray start
    /// \param Global space ray direction
    ///
    void beginRayCellGrasp(std::shared_ptr<AnalyticalGeometry> geometry,
                           const Vec3d& rayStart, const Vec3d& rayDir, const double maxDist = -1.0);

    ///
    /// \brief End a grasp (picking will end on next update)
    ///
    void endGrasp();

    ///
    /// \brief Compute/generate the constraints for picking
    ///
    void addPickConstraints();

    ///
    /// \brief Remove the constraints for picking
    ///
    void removePickConstraints();

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
    void setGeometryToPick(std::shared_ptr<Geometry> geomToPick, std::shared_ptr<PointwiseMap> map)
    {
        m_geomToPick = geomToPick;
        m_geometryToPickMap = map;
    }

    std::shared_ptr<TaskNode> getPickingNode() const { return m_pickingNode; }

    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

    bool hasConstraints() const;

protected:
    ///
    /// \brief Update picking state, this should move grasp points
    ///
    virtual void updatePicking();

    ///
    /// \brief Update the constraints used for picking
    ///
    void updateConstraints();

protected:
    std::shared_ptr<TaskNode> m_pickingNode = nullptr;

    std::shared_ptr<Geometry>     m_geomToPick = nullptr;
    std::shared_ptr<PointwiseMap> m_geometryToPickMap = nullptr;

    std::shared_ptr<PbdObject> m_objectToGrasp      = nullptr;
    std::shared_ptr<AnalyticalGeometry> m_graspGeom = nullptr;

    std::shared_ptr<PickingAlgorithm> m_pickMethod = nullptr;
    GraspMode m_graspMode = GraspMode::Cell;

    bool m_isGrasping     = false;
    bool m_isPrevGrasping = false;

    /// Stiffness of grasp, when 1 the position is completely moved too the grasp point
    /// when stiffness < 1 it will slowly converge on the grasp point
    double m_stiffness = 0.4;

    std::list<std::tuple<Vec3d, Vec3d, Vec3d>> m_constraintPts;         ///< Position, Relative position, Velocity
    std::vector<std::shared_ptr<PbdCollisionConstraint>> m_constraints; ///< List of PBD constraints
};
} // namespace imstk