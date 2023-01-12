/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionInteraction.h"
#include "imstkMacros.h"

namespace imstk
{
class PbdObject;

///
/// \class PbdObjectCollision
///
/// \brief This class defines a collision interaction between two PbdObjects
/// or PbdObject & CollidingObject
///
class PbdObjectCollision : public CollisionInteraction
{
public:
    ///
    /// \brief Constructor for PbdObject-PbdObject or PbdObject-CollidingObject collisions
    ///
    PbdObjectCollision(std::shared_ptr<Entity> obj1, std::shared_ptr<Entity> obj2,
                       std::string cdType = "");

    ~PbdObjectCollision() override = default;

    IMSTK_TYPE_NAME(PbdObjectCollision)

    ///
    /// \brief Get/Set the restitution, which gives how much velocity is
    /// removed along the contact normals during contact
    /// @{
    void setRestitution(const double restitution);
    double getRestitution() const;
    /// @}

    ///
    /// \brief Get/Set the friction, which gives how much velocity is
    /// removed along the tangents during contact
    /// @{
    void setFriction(const double friction);
    double getFriction() const;
    /// @}

    ///
    /// \brief Get/Set whether velocity is corrected (in some cases this could
    /// cause instabilities)
    /// @{
    bool getUseCorrectVelocity() const;
    void setUseCorrectVelocity(const bool useCorrectVelocity);
    /// @}

    ///
    /// \brief Get/Set compliance of rigid body contacts. Defaults to 0
    /// compliance/infinitely stiff. This is what is needed most of the time
    /// but sometimes making a contact a bit softer can be helpful.
    /// @{
    void setRigidBodyCompliance(const double compliance);
    double getRigidBodyCompliance() const;
    /// @}

    ///
    /// \brief Get/Set stiffness of deformable contacts. Defaults to 1.0.
    /// This is what is needed most of the time but sometimes making a
    /// contact a bit softer can be helpful.
    /// @{
    void setDeformableStiffnessA(const double stiffness);
    double getDeformableStiffnessA() const;
    void setDeformableStiffnessB(const double stiffness);
    double getDeformableStiffnessB() const;
    /// @}

    ///
    /// \brief Setup connectivity of task graph
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

protected:
    std::shared_ptr<TaskNode> m_updatePrevGeometryCCDNode = nullptr;

private:
    /// Called from the constructor
    void setupConnections(std::shared_ptr<Entity> obj1, std::shared_ptr<Entity> obj2,
                          std::string cdType = "");
};
} // namespace imstk