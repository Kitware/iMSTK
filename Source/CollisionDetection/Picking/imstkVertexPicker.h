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