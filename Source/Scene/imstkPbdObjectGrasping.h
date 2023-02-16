/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionInteraction.h"
#include "imstkMacros.h"
#include "imstkMath.h"
#include "imstkPbdConstraint.h"

#include <unordered_map>

namespace imstk
{
class AnalyticalGeometry;
class PbdMethod;
class PickingAlgorithm;
class PointwiseMap;

///
/// \class PbdObjectGrasping
///
/// \brief This class defines grasping of a PbdMethod via different
/// picking methods. Where grasping is define as grabbing & attaching
/// of a PbdMethod's mesh to points.
///
/// Given an input PickData the appropriate grasping will be produced.
///
class PbdObjectGrasping : public SceneObject
{
public:
    enum class GraspMode
    {
        Vertex,     // Grab a vertex (most performant)
        Cell,       // Grab an entire cell/s
        RayPoint,   // Grab a point on the nearest cell along the ray
        RayCell     // Grab a cell along the ray
    };

    ///
    /// \brief Construct PbdObjectGrasping with a object to grasp and
    /// and optionally an object that does the grasping. Whilst grasping
    /// can work with any grasping criteria, if you want a two-way response
    /// you must supply an object to grasp with.
    ///
    PbdObjectGrasping(std::shared_ptr<PbdMethod> graspedObject,
                      std::shared_ptr<PbdMethod> grasperObject = nullptr);
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
    /// \brief Set/Get the compliance
    ///@{
    void setCompliance(const double compliance) { m_compliance = compliance; }
    double getCompliance() const { return m_compliance; }
    ///@}

    ///
    /// \brief Begin a vertex grasp (picking will begin on the next update)
    /// \param Geometry attached/grasped too
    ///
    void beginVertexGrasp(std::shared_ptr<AnalyticalGeometry> geometry);

    ///
    /// \brief Begin a cell grasp (picking will begin on the next update)
    /// Also works for rigid on rigid
    /// \param Geometry attached/grasped too
    /// \param The intersection type/class name
    ///
    void beginCellGrasp(std::shared_ptr<AnalyticalGeometry> geometry, std::string cdType = "");

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
    virtual void addPointToPointConstraint(
        const std::vector<PbdParticleId>& ptsA,
        const std::vector<double>& weightsA,
        const std::vector<PbdParticleId>& ptsB,
        const std::vector<double>& weightsB,
        const double stiffnessA, const double stiffnessB);

    ///
    /// \brief Add 0 distance constraint between two points defined on
    /// two separate bodies.
    ///
    virtual void addBodyToBodyConstraint(
        const PbdParticleId& graspedBodyId,
        const PbdParticleId& grasperBodyId,
        const Vec3d&         pointOnBodies,
        const double         compliance);

    ///
    /// \brief Add a 0 distance constraint between a deformable point and
    /// a point on a body
    ///
    virtual void addPointToBodyConstraint(
        const PbdParticleId& graspedParticleId,
        const PbdParticleId& grasperBodyId,
        const Vec3d&         pointOnBody,
        const double         compliance);

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

    ///
    /// \brief Returns if any grasp constraints are present
    ///
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

    std::shared_ptr<PbdMethod> m_objectToGrasp      = nullptr;
    std::shared_ptr<PbdMethod> m_grasperObject      = nullptr; // Optional
    std::shared_ptr<AnalyticalGeometry> m_graspGeom = nullptr;

    std::shared_ptr<PickingAlgorithm> m_pickMethod = nullptr;
    GraspMode m_graspMode = GraspMode::Cell;

    bool m_isGrasping     = false;
    bool m_isPrevGrasping = false;

    /// Stiffness of grasp, when 1 the position is completely moved too the grasp point
    /// when stiffness < 1 it will slowly converge on the grasp point
    double m_stiffness  = 0.4;    // For deformables
    double m_compliance = 0.0001; // For rigid bodies (inverse of stiffness)

    /// Vec of virtual particle grasp point ids, and local positions when grasped
    std::vector<std::tuple<PbdParticleId, Vec3d>> m_constraintPts;
    std::vector<std::shared_ptr<PbdConstraint>>   m_constraints; ///< List of PBD constraints

private:
    std::vector<PbdConstraint*> m_collisionConstraints;
};
} // namespace imstk