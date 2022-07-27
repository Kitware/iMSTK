/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkMacros.h"

namespace imstk
{
class SurfaceMesh;

///
/// \class SurfaceMeshToSurfaceMeshCD
///
/// \brief Collision detection for surface meshes
///
class SurfaceMeshToSurfaceMeshCD : public CollisionDetectionAlgorithm
{
public:
    SurfaceMeshToSurfaceMeshCD();
    ~SurfaceMeshToSurfaceMeshCD() override = default;

    IMSTK_TYPE_NAME(SurfaceMeshToSurfaceMeshCD)

public:
    void setMaxNumContacts(const int maxNumContacts) { m_maxNumContacts = maxNumContacts; }
    const int getMaxNumContacts() const { return m_maxNumContacts; }

protected:
    ///
    /// \brief Compute collision data for AB simultaneously
    ///
    void computeCollisionDataAB(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA,
        std::vector<CollisionElement>& elementsB) override;

protected:
    std::vector<std::pair<int, int>> m_intersectingPairs;
    int m_maxNumContacts = 1000;
};
} // namespace imstk