/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkSurfaceMesh.h"

#include <unordered_map>

namespace imstk
{
class CollisionData;
class EmbeddingConstraint;
class LineMesh;
class PbdConstraint;
class PbdObject;
class Puncturable;
class StraightNeedle;
class TaskNode;
class TetrahedralMesh;
} // namespace imstk

using namespace imstk;

///
/// \brief Flattened out with reference members
///
struct TissueData
{
    std::shared_ptr<PbdObject> obj;

    TissueData(std::shared_ptr<PbdObject> obj);

    std::shared_ptr<TetrahedralMesh> geom;
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr;
    VecDataArray<double, 3>& vertices;
    std::shared_ptr<VecDataArray<int, 4>> indicesPtr;
    VecDataArray<int, 4>& indices;
};
struct NeedleData
{
    std::shared_ptr<PbdObject> obj;

    NeedleData(std::shared_ptr<PbdObject> obj);

    std::shared_ptr<StraightNeedle> needle;
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr;
    VecDataArray<double, 3>& vertices;
    std::shared_ptr<VecDataArray<int, 2>> cellsPtr;
    VecDataArray<int, 2>& cells;
};

///
/// \class NeedleEmbedder
///
/// \brief Implements PBD embedded tissue handling for when the needle is
/// embedded in the tissue.
///
class NeedleEmbedder
{
public:
    void setTissueObject(std::shared_ptr<PbdObject> tissueObject) { m_tissueObject = tissueObject; }
    std::shared_ptr<PbdObject> getTissueObject() const { return m_tissueObject; }
    void setNeedleObject(std::shared_ptr<PbdObject> needleObject) { m_needleObject = needleObject; }
    std::shared_ptr<PbdObject> getNeedleObject() const { return m_needleObject; }

    void setCollisionData(std::shared_ptr<CollisionData> cdData) { m_cdData = cdData; }
    void setCollisionHandleNode(std::shared_ptr<TaskNode> pbdCHNode) { m_pbdCHNode = pbdCHNode; }

    void setFriction(const double friction) { m_friction = friction; }
    double getFriction() const { return m_friction; }

    void setCompliance(const double compliance) { m_compliance = compliance; }
    double getCompliance() const { return m_compliance; }

    void setStaticFrictionForceThreshold(const double force) { m_staticFrictionForceThreshold = force; }
    const double getStaticFrictionForceThreshold() const { return m_staticFrictionForceThreshold; }

    void setPunctureForceThreshold(const double forceThreshold) { m_forceThreshold = forceThreshold; }
    const double getPunctureForceThreshold() const { return m_forceThreshold; }

    ///
    /// \brief Add embedding constraints based off contact data
    /// We need to add the constraint once and then update it later
    ///
    void update();

protected:
    ///
    /// \brief Adds embedding constraint (ie: The constraint maintained after puncture)
    ///
    virtual void addFaceEmbeddingConstraint(
        TissueData& tissueData,
        NeedleData& needleData,
        int v1, int v2, int v3,
        const Vec3d& iPt);

protected:
    std::shared_ptr<PbdObject> m_tissueObject = nullptr;
    std::shared_ptr<PbdObject> m_needleObject = nullptr;

    std::shared_ptr<TaskNode>      m_pbdCHNode = nullptr;
    std::shared_ptr<CollisionData> m_cdData    = nullptr;

    // TriCell takes care of duplicate faces
    std::unordered_map<TriCell, std::shared_ptr<EmbeddingConstraint>> m_faceConstraints;
    std::vector<PbdConstraint*> m_constraints; ///< List of PBD constraints

    double m_friction   = 0.0;                 ///< Coefficient of friction (1.0 = full frictional force, 0.0 = none)
    double m_compliance = 0.000001;
    double m_staticFrictionForceThreshold = 0.0;
    double m_forceThreshold = 10.0;

    VecDataArray<double, 3> tissuePrevVertices;
    VecDataArray<double, 3> needlePrevVertices;

public:
    std::vector<Vec3d> m_debugEmbeddingPoints; ///< Used for debug visualization
    std::vector<Vec3i> m_debugEmbeddedTriangles;
};