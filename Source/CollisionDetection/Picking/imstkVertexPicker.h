/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkPickingAlgorithm.h"

namespace imstk
{
class ImplicitGeometry;

///
/// \class VertexPicker
///
/// \brief Picks vertices of geomToPick via those that that are
/// intersecting pickingGeom.
///
class VertexPicker : public PickingAlgorithm
{
public:
    VertexPicker();
    ~VertexPicker() override = default;

    void requestUpdate() override;

    void setPickingGeometry(std::shared_ptr<ImplicitGeometry> pickGeometry) { m_pickGeometry = pickGeometry; }
    std::shared_ptr<ImplicitGeometry> getPickGeometry() const { return m_pickGeometry; }

protected:
    std::shared_ptr<ImplicitGeometry> m_pickGeometry = nullptr;
};
} // namespace imstk