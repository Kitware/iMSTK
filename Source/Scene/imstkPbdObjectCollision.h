/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionInteraction.h"
#include "imstkMacros.h"

#include <array>

namespace imstk
{
class Collider;
class PbdMethod;
class PbdSystem;
class TaskGraph;

///
/// \class PbdObjectCollision
///
/// \brief This class defines a collision interaction between two PbdMethods
/// or PbdMethod & Collider
///
class PbdObjectCollision : public CollisionInteraction
{
public:
    IMSTK_TYPE_NAME(PbdObjectCollision)
    ///
    /// \brief Constructor for PbdMethod-PbdMethod or PbdMethod-Collider collisions
    ///
    PbdObjectCollision(std::shared_ptr<Entity> obj1, std::shared_ptr<Entity> obj2,
                       std::string cdType = "");

    ~PbdObjectCollision() override = default;

    ///
    /// \brief Initialize the interaction based on set input before the simulation starts.
    ///
    bool initialize() override;

    ///
    /// \brief Setup connectivity of task graph
    ///
    void initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink) override;

    ///
    /// \brief Get/Set the restitution, which gives how much velocity is
    /// removed along the contact normals during contact
    /// @{
    imstkSetGetMacro(Restitution, m_restitution, double)
    /// @}

    ///
    /// \brief Get/Set the friction, which gives how much velocity is
    /// removed along the tangents during contact
    /// @{
    imstkSetGetMacro(Friction, m_friction, double)
    /// @}

    ///
    /// \brief Get/Set whether velocity is corrected (in some cases this could
    /// cause instabilities)
    /// @{
    imstkSetGetMacro(UseCorrectVelocity, m_useCorrectVelocity, bool)
    /// @}

    ///
    /// \brief Get enableBoundaryCollision
    ///@{
    imstkSetGetMacro(EnableBoundaryCollisions, m_enableBoundaryCollisions, bool)
    ///@}

    ///
    /// \brief Get/Set compliance of rigid body contacts. Defaults to 0
    /// compliance/infinitely stiff. This is what is needed most of the time
    /// but sometimes making a contact a bit softer can be helpful.
    /// @{
    imstkSetGetMacro(RigidBodyCompliance, m_rigidBodyCompliance, double)
    /// @}

    ///
    /// \brief Get/Set stiffness of deformable contacts. Defaults to 1.0.
    /// This is what is needed most of the time but sometimes making a
    /// contact a bit softer can be helpful.
    /// @{
    imstkSetGetMacro(DeformableStiffnessA, m_deformableStiffness[0], double)
    imstkSetGetMacro(DeformableStiffnessB, m_deformableStiffness[1], double)
/// @}

protected:
    std::shared_ptr<TaskNode> m_updatePrevGeometryCCDNode = nullptr;

private:
    /// Called from the constructor
    void setupConnections();

    // Colliding components.
    struct
    {
        std::shared_ptr<Collider> collider;
        std::shared_ptr<PbdMethod> method;
        std::shared_ptr<TaskGraph> taskGraph;
        std::shared_ptr<PbdSystem> system;
    } m_objectA, m_objectB;

    // Collision Handler configuration (default values copied from PbdCollisionHandling).
    // In the future, refactor these parameters as a config struct to maintain consistent set of parameters
    // and default values across classes.
    double m_friction    = 0.0,
           m_restitution = 0.0,
           m_rigidBodyCompliance = 0.000001;
    std::array<double, 2> m_deformableStiffness = { 0.3, 0.3 };
    bool m_useCorrectVelocity = true, m_enableBoundaryCollisions = false;
};
} // namespace imstk