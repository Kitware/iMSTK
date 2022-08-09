/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionHandling.h"
#include "imstkPbdConstraint.h"

#include <unordered_map>

namespace imstk
{
enum PbdContactCase
{
    Vertex,    // Mesh vertex either from Pbd or CollidingObject
    Edge,      // Mesh edge either from Pbd or CollidingObject
    Triangle,  // Mesh triangle either from Pbd or CollidingObject
    Body,      // Body
    Primitive, // Not a mesh at all still could be any CD data type
    None
};
static std::string
getContactCaseStr(PbdContactCase contactCase)
{
    switch (contactCase)
    {
    case PbdContactCase::Body:
        return "Body";
    case PbdContactCase::Edge:
        return "Edge";
    case PbdContactCase::Primitive:
        return "Primitive";
    case PbdContactCase::Triangle:
        return "Triangle";
    case PbdContactCase::Vertex:
        return "Vertex";
    default:
        return "None";
    }
    ;
}

///
/// \struct PbdCHTableKey
///
/// \brief Used as a key in a function table to decide how to handle
/// resulting collision.
///
struct PbdCHTableKey
{
    PbdContactCase elemAType;
    PbdContactCase elemBType;
    bool ccd;

    friend std::ostream& operator<<(std::ostream& os, const PbdCHTableKey& dt);

    bool operator==(const PbdCHTableKey& other) const
    {
        return
            (elemAType == other.elemAType)
            && (elemBType == other.elemBType)
            && (ccd == other.ccd);
    }
};
} // namespace imstk

namespace std
{
///
/// \struct hash<imstk::PbdCHTableKey>
///
/// \brief Gives hashing function for PbdCHFuncTableKey
/// complete unique/garunteed no collisions
///
template<>
struct hash<imstk::PbdCHTableKey>
{
    std::size_t operator()(const imstk::PbdCHTableKey& k) const
    {
        using std::size_t;
        using std::hash;

        // Base on the bit width of each value
        std::size_t v0 = static_cast<std::size_t>(k.elemAType); // first 2 bits
        std::size_t v1 = static_cast<std::size_t>(k.elemBType); // Next 2 bits
        std::size_t v2 = static_cast<std::size_t>(k.ccd);       // Next bit
        return v0 ^ (v1 << 3) ^ (v2 << 5);
    }
};
} // namespace std

namespace imstk
{
class PbdObject;
class PbdModel;
class PointSet;
class PointwiseMap;

///
/// \class PbdCollisionHandling
///
/// \brief Implements PBD based collision handling. Given an input PbdObject
/// and CollisionData it creates & adds constraints in the PbdModel to be solved
/// in order to resolve the collision.
///
/// This solve happens later together with all other collision constraints in the
/// PbdModels collision solve step.
///
/// This supports PD (PointDirection) collision data as well as contacting feature
/// collision data (ie: EE (Edge, Edge), VT (Vertex-Triangle), VV (Vertex-Vertex), VE (Vertex-Edge)).
/// The VV and VE are often redundant but handled anyways for robustness to different inputs.
/// The PD is often reported for point contacts, most commonly on primitive vs mesh collisions.
///
class PbdCollisionHandling : public CollisionHandling
{
public:
    enum class ObjType
    {
        PbdDeformable,
        PbdRigid,
        Colliding
    };
    struct CollisionSideData
    {
        CollisionSideData() = default;

        // Objects
        PbdObject* pbdObj       = nullptr;
        CollidingObject* colObj = nullptr;
        ObjType objType = ObjType::Colliding;

        PbdModel* model    = nullptr;
        double compliance  = 0.0;
        double stiffness   = 0.0;
        Geometry* geometry = nullptr;
        PointSet* pointSet = nullptr;
        VecDataArray<double, 3>* vertices = nullptr;
        PointwiseMap* mapPtr = nullptr;
        AbstractDataArray* indicesPtr = nullptr;
        int bodyId = 0;

        Geometry* prevGeometry = nullptr;
    };
    ///
    /// \brief Packs the collision element together with the
    /// data it will need to process it (for swapping)
    ///
    struct ColElemSide
    {
        const CollisionElement* elem  = nullptr;
        const CollisionSideData* data = nullptr;
    };

    PbdCollisionHandling();
    ~PbdCollisionHandling() override;

    IMSTK_TYPE_NAME(PbdCollisionHandling)

    ///
    /// \brief Get/Set the restitution, which gives how much velocity is
    /// removed along the contact normals during contact
    /// @{
    double getRestitution() const { return m_restitution; }
    void setRestitution(const double restitution) { m_restitution = restitution; }
    /// @}

    ///
    /// \brief Get/Set the friction, which gives how much velocity is
    /// removed along the tangents during contact
    /// @{
    double getFriction() const { return m_friction; }
    void setFriction(const double friction) { m_friction = friction; }
    /// @}

    ///
    /// \brief Get enableBoundaryCollision
    ///@{
    void setEnableBoundaryCollisions(const double enableBoundaryCollisions) { m_enableBoundaryCollisions = enableBoundaryCollisions; }
    const double getEnableBoundaryCollisions() const { return m_enableBoundaryCollisions; }
    ///@}

    ///
    /// \brief Get/Set compliance of rigid body contacts. Defaults to 0
    /// compliance/infinitely stiff. This is what is needed most of the time
    /// but sometimes making a contact a bit softer can be helpful.
    /// @{
    void setRigidBodyCompliance(const double compliance) { m_compliance = compliance; }
    double getRigidBodyCompliance() const { return m_compliance; }
    /// @}

    ///
    /// \brief Get/Set stiffness of deformable contacts. Defaults to 1.0.
    /// This is what is needed most of the time but sometimes making a
    /// contact a bit softer can be helpful.
    /// @{
    void setDeformableStiffnessA(const double stiffness) { m_stiffness[0] = stiffness; }
    double getDeformableStiffnessA() const { return m_stiffness[0]; }
    void setDeformableStiffnessB(const double stiffness) { m_stiffness[1] = stiffness; }
    double getDeformableStiffnessB() const { return m_stiffness[1]; }
    /// @}

    ///
    /// \brief Get the body particle id from the collision side
    /// as well as the contact point on the body (in global space)
    ///
    std::pair<PbdParticleId, Vec3d> getBodyAndContactPoint(
        const CollisionElement&  elem,
        const CollisionSideData& data);

protected:
    std::array<PbdParticleId, 2> getEdge(
        const CollisionElement&  elem,
        const CollisionSideData& side);
    std::array<PbdParticleId, 3> getTriangle(
        const CollisionElement&  elem,
        const CollisionSideData& side);
    ///
    /// \brief getVertex takes slightly differing paths than the others, as the
    /// cell vertex directly refers to the vertex buffer, not an index buffer
    ///
    std::array<PbdParticleId, 1> getVertex(
        const CollisionElement&  elem,
        const CollisionSideData& side);

    ///
    /// \brief Creates a CollisionSideData struct from the provided object, this
    /// gives all the info needed to response to collision
    ///
    CollisionSideData getDataFromObject(std::shared_ptr<CollidingObject> obj);

    ///
    /// \brief Get the contact case from the collision element and data as
    /// additional context
    ///
    PbdContactCase getCaseFromElement(const ColElemSide& elem);

    ///
    /// \brief Add collision constraints based off contact data
    ///
    void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override;

    ///
    /// \brief Handle a single element
    ///
    void handleElementPair(ColElemSide sideA, ColElemSide sideB);

    // -----------------One-Way Rigid on X Cases-----------------
    virtual void addConstraint_Body_V(
        const ColElemSide& sideA,
        const ColElemSide& sideB);
    virtual void addConstraint_Body_E(
        const ColElemSide& sideA,
        const ColElemSide& sideB);
    virtual void addConstraint_Body_T(
        const ColElemSide& sideA,
        const ColElemSide& sideB);
    // ---------------Two-Way Rigid on Rigid Cases---------------
    /*virtual void addConstraint_Body_Body(
        const ColElemSide& sideA,
        const ColElemSide& sideB);*/

    // ----------DeformableMesh on DeformableMesh Cases----------
    virtual void addConstraint_V_T(
        const ColElemSide& sideA,
        const ColElemSide& sideB);
    virtual void addConstraint_E_E(
        const ColElemSide& sideA,
        const ColElemSide& sideB);
    virtual void addConstraint_E_E_CCD(
        const ColElemSide& sideA,
        const ColElemSide& sideB);
    virtual void addConstraint_V_E(
        const ColElemSide& sideA,
        const ColElemSide& sideB);
    virtual void addConstraint_V_V(
        const ColElemSide& sideA,
        const ColElemSide& sideB);

private:
    double m_restitution = 0.0;  ///< Coefficient of restitution (1.0 = perfect elastic, 0.0 = inelastic)
    double m_friction    = 0.0;  ///< Coefficient of friction (1.0 = full frictional force, 0.0 = none)

    /// Enables collisions on fixed pbd elements. Collision on these elements can cause instabilities
    /// as the collisions near the fixed vertices.
    bool   m_enableBoundaryCollisions = false;
    double m_compliance = 0.0;
    std::array<double, 2> m_stiffness = { 1.0, 1.0 };

protected:
    std::vector<PbdConstraint*> m_constraints; ///< Constraints users can add too

    std::unordered_map<PbdCHTableKey, std::function<void(
                                                        const ColElemSide& elemA, const ColElemSide& elemB)>> m_funcTable;
};
} // namespace imstk
