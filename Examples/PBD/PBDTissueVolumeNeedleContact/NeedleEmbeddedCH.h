/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionHandling.h"
#include "imstkSurfaceMesh.h"

#include <unordered_map>

class NeedleObject;

namespace imstk
{
class EmbeddingConstraint;
class LineMesh;
class PbdConstraint;
class PbdObject;
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
    std::shared_ptr<VecDataArray<double, 3>> velocitiesPtr;
    VecDataArray<double, 3>& velocities;
    std::shared_ptr<DataArray<double>> invMassesPtr;
    DataArray<double>& invMasses;
};
struct NeedleData
{
    std::shared_ptr<NeedleObject> obj;

    NeedleData(std::shared_ptr<NeedleObject> obj);

    std::shared_ptr<LineMesh> geom;
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr;
    VecDataArray<double, 3>& vertices;
    std::shared_ptr<VecDataArray<int, 2>> indicesPtr;
    VecDataArray<int, 2>& indices;
};

///
/// \class NeedleEmbeddedCH
///
/// \brief Implements PBD-RBD embedded tissue handling for when the
/// needle is embedded in the tissue
///
class NeedleEmbeddedCH : public CollisionHandling
{
public:
    ~NeedleEmbeddedCH() override = default;

    IMSTK_TYPE_NAME(NeedleEmbeddedCH)

public:
    std::shared_ptr<Geometry> getHandlingGeometryA() override;

public:
    void setFriction(const double friction) { m_friction = friction; }
    double getFriction() const { return m_friction; }

    void setCompliance(const double compliance) { m_compliance = compliance; }
    double getCompliance() const { return m_compliance; }

    void setStaticFrictionForceThreshold(const double force) { m_staticFrictionForceThreshold = force; }
    const double getStaticFrictionForceThreshold() const { return m_staticFrictionForceThreshold; }

    void setPunctureForceThreshold(const double forceThreshold) { m_forceThreshold = forceThreshold; }
    const double getPunctureForceThreshold() const { return m_forceThreshold; }

protected:
    ///
    /// \brief Add embedding constraints based off contact data
    /// We need to add the constraint once and then update it later
    ///
    void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override;

    ///
    /// \brief Updates puncture state for the needle. Could be not
    /// touching, touching, or inserted.
    /// Works via projection and thresholding of the force on the needle axes.
    /// Returns whether embedded
    ///
    void updatePunctureState(
        TissueData& tissueData,
        NeedleData& needleData,
        bool        notIntersecting);

    ///
    /// \brief Adds embedding constraint (ie: The constraint maintained after puncture)
    ///
    virtual void addFaceEmbeddingConstraint(
        TissueData& tissueData,
        NeedleData& needleData,
        int v1, int v2, int v3,
        const Vec3d& iPt);

private:
    // TriCell takes care of duplicate faces
    std::unordered_map<TriCell, std::shared_ptr<EmbeddingConstraint>> m_faceConstraints;
    std::unordered_set<std::shared_ptr<EmbeddingConstraint>> m_constraintEnabled;

    std::vector<PbdConstraint*> m_constraints; ///< List of PBD constraints

    double m_friction   = 0.0;                 ///< Coefficient of friction (1.0 = full frictional force, 0.0 = none)
    double m_compliance = 0.000001;
    double m_staticFrictionForceThreshold = 0.0;
    double m_forceThreshold = 10.0;

public:
    std::vector<Vec3d> m_debugEmbeddingPoints; ///< Used for debug visualization
    std::vector<Vec3i> m_debugEmbeddedTriangles;
};