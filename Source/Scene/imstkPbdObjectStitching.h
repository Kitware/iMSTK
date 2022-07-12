/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkMacros.h"
#include "imstkMath.h"
#include "imstkPbdCollisionConstraint.h"
#include "imstkSceneObject.h"

#include <unordered_map>

namespace imstk
{
class PbdBaryPointToPointConstraint;
class PbdObject;
class PickingAlgorithm;
class PointwiseMap;

///
/// \class PbdObjectStitching
///
/// \brief This class defines stitching of a PbdObject via different
/// picking methods. Ray based stitching is default.
///
class PbdObjectStitching : public SceneObject
{
public:
    PbdObjectStitching(std::shared_ptr<PbdObject> obj1);
    ~PbdObjectStitching() override = default;

    IMSTK_TYPE_NAME(PbdObjectStitching)

    ///
    /// \brief Set/Get the stiffness, 0-1 value that alters the step size in
    /// the solver
    ///@{
    void setStiffness(const double stiffness) { m_stiffness = stiffness; }
    double getStiffness() const { return m_stiffness; }
    ///@}

    ///
    /// \brief Set/Get the maximum distance for which a stitch may be placed
    ///@{
    void setStitchDistance(const double distance) { m_maxStitchDist = distance; }
    double getStitchDistance() const { return m_maxStitchDist; }
    ///@}

    ///
    /// \brief Begin a ray point stitch. Stitches two points for separate elements.
    /// \param Global space ray start
    /// \param Global space ray direction
    ///
    void beginStitch(const Vec3d& rayStart, const Vec3d& rayDir, const double maxDist = -1.0);

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
        const std::vector<PbdParticleId>& ptsA,
        const std::vector<double>& weightsA,
        const std::vector<PbdParticleId>& ptsB,
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
    void setGeometryToStitch(std::shared_ptr<Geometry> geomToStitch, std::shared_ptr<PointwiseMap> map)
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

protected:
    std::shared_ptr<TaskNode> m_stitchingNode = nullptr;

    std::shared_ptr<Geometry>     m_geomToStitch = nullptr;
    std::shared_ptr<PointwiseMap> m_geometryToStitchMap = nullptr;

    std::shared_ptr<PbdObject> m_objectToStitch = nullptr;

    std::shared_ptr<PickingAlgorithm> m_pickMethod = nullptr;

    bool m_performStitch = false;

    /// Stiffness of stitches, when 1 the position is completely moved too the grasp point
    /// when stiffness < 1 it will slowly converge on the grasp point
    double m_stiffness     = 0.1;
    double m_maxStitchDist = -1.0;                                             // Set a maximum distance for which a stitch can be placed

    std::vector<std::shared_ptr<PbdBaryPointToPointConstraint>> m_constraints; ///< Array of PBD constraints

private:
    std::vector<PbdConstraint*> m_collisionConstraints;
};
} // namespace imstk