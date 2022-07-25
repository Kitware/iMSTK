/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPickingAlgorithm.h"

namespace imstk
{
class CollisionDetectionAlgorithm;

///
/// \class CellPicker
///
/// \brief Picks cells of geomToPick via those that that are
/// intersecting pickingGeom.
///
class CellPicker : public PickingAlgorithm
{
public:
    CellPicker();
    ~CellPicker() override = default;

    void requestUpdate() override;

    void setPickingGeometry(std::shared_ptr<Geometry> pickGeometry) { m_pickGeometry = pickGeometry; }
    std::shared_ptr<Geometry> getPickGeometry() const { return m_pickGeometry; }

    void setCollisionDetection(std::shared_ptr<CollisionDetectionAlgorithm> colDetect) { m_colDetect = colDetect; }
    std::shared_ptr<CollisionDetectionAlgorithm> getCollisionDetection() const { return m_colDetect; }

protected:
    std::shared_ptr<Geometry> m_pickGeometry = nullptr;
    std::shared_ptr<CollisionDetectionAlgorithm> m_colDetect = nullptr;
};
} // namespace imstk