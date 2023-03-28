/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionHandling.h"
#include "imstkMacros.h"

namespace imstk
{
class Collider;
class CollisionData;
class CollisionDetectionAlgorithm;
class SphSystem;
class SphMethod;

///
/// \class SphCollisionHandling
///
/// \brief The SphCollisionHandling consumes PointIndexDirection contact data
/// to resolve positions and correct velocities of SPH particles. It does
/// not correct pressures/densities.
///
class SphCollisionHandling : public CollisionHandling
{
public:
    SphCollisionHandling() = default;
    virtual ~SphCollisionHandling() override = default;

    IMSTK_TYPE_NAME(SphCollisionHandling)

    void setInputSphObject(std::shared_ptr<SphMethod> sphObj, std::shared_ptr<Collider> collider)
    {
        m_sphMethod = sphObj;
        m_collider  = collider;
    }

    ///
    /// \brief How many times to resolve and correct position. This is useful when colliding
    /// with multiple objects or in a corner of another object
    ///
    void setNumberOfIterations(int iterations) { this->m_iterations = iterations; }

    ///
    /// \brief Sets detection object for iteration handling+detection
    ///
    void setDetection(std::shared_ptr<CollisionDetectionAlgorithm> colDetect) { this->m_colDetect = colDetect; }

    bool initialize() override;

    ///
    /// \brief Resolve SPH particle positions
    ///
    void handle(
        const std::vector<CollisionElement>& elementsA,
        const std::vector<CollisionElement>& elementsB) override;

protected:
    ///
    /// \brief Get the geometry used for handling
    /// defaults to the collision geometry
    ///
    std::shared_ptr<Geometry> getCollidingGeometryA() override;
    std::shared_ptr<Geometry> getCollidingGeometryB() override;
    ///
    /// \brief Solves positiona and corrects velocity of individual particle
    ///
    void solve(Vec3d& pos, Vec3d& velocity, const Vec3d& penetrationVector);

private:
    std::shared_ptr<CollisionDetectionAlgorithm> m_colDetect = nullptr;
    int    m_iterations       = 1;
    double m_boundaryFriction = 0.0;

    std::shared_ptr<SphMethod> m_sphMethod;
    std::shared_ptr<Collider>  m_collider;
    std::shared_ptr<SphSystem> m_sphSystem;
};
} // end namespace imstk
