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

#include "imstkImplicitGeometry.h"

namespace imstk
{
class ImplicitGeometry;

///
/// \class LevelSetState
///
/// \brief Generic definition of the state of the levelset via ImplicitGeometry
/// The state gives a set of implicit geometries, each of which represent the state
/// of a quantity on the levelset. Each implicit geometry then gives the signed function value
/// and gradient of the quantity. Usually this would be distances, but one may also evolve densities
/// or even heat on any domain
///
class LevelSetState : public ImplicitGeometry
{
public:
    LevelSetState() = default;
    ~LevelSetState() override = default;

    ///
    /// \brief Initialize the pbd state
    ///
    void initialize(std::shared_ptr<ImplicitGeometry> geometry) {}

    ///
    /// \brief Set the state to a given one
    ///
    void setState(std::shared_ptr<LevelSetState> rhs) {}
};
}