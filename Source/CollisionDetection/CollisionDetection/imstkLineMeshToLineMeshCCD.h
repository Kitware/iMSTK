/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <array>
#include "imstkCCDAlgorithm.h"
#include "imstkVecDataArray.h"
#include "imstkMacros.h"

namespace imstk
{
template<typename T, int N> class VecDataArray;

class LineMesh;

///
/// \class LineMeshToLineMeshCCD
///
/// \brief LineMesh to LineMesh continous collision detection.
/// This CCD method can process self collision as well.
/// Self collision mode is indicated to the algorithm by providing
/// geometryA (input 0) == geometryB (input 1).
///
class LineMeshToLineMeshCCD : public CCDAlgorithm
{
public:
    LineMeshToLineMeshCCD();
    ~LineMeshToLineMeshCCD() override = default;

    IMSTK_TYPE_NAME(LineMeshToLineMeshCCD)

    ///
    /// \brief Copy LineMesh geometry information (points only),
    /// as previous timestep information. These are used with current geometries
    /// that are received in computeCollisionDataXX functions for computing
    /// the continuous collision detection.
    ///
    virtual void updatePreviousTimestepGeometry(
        std::shared_ptr<const Geometry> geomA,
        std::shared_ptr<const Geometry> geomB) override;

protected:
    ///
    /// \brief Compute collision data for AB simultaneously
    ///
    virtual void computeCollisionDataAB(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA,
        std::vector<CollisionElement>& elementsB) override;

    ///
    /// \brief Compute collision data for side A
    ///
    virtual void computeCollisionDataA(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsA) override;

    ///
    /// \brief Compute collision data for side B
    ///
    virtual void computeCollisionDataB(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>& elementsB) override;

private:
    void internalComputeCollision(
        std::shared_ptr<Geometry>      geomA,
        std::shared_ptr<Geometry>      geomB,
        std::vector<CollisionElement>* elementsA,
        std::vector<CollisionElement>* elementsB);

    std::shared_ptr<imstk::LineMesh> m_prevA;
    std::shared_ptr<imstk::LineMesh> m_prevB;
};
} // namespace imstk