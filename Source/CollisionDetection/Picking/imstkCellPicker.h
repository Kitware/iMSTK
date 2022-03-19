/*=========================================================================

Library: iMSTK

Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
& Imaging in Medicine, Rensselaer Polytechnic Institute.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0.txt

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

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