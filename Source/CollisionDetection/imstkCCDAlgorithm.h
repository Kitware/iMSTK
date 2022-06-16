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

#include "imstkCollisionDetectionAlgorithm.h"

namespace imstk
{
class Geometry;

///
/// \class CCDAlgorithm
///
/// \brief Base class for all Continuous Collision Detection (CCD) classes.
/// A continuous collision detection algorithm typically requires geometries in two
/// timesteps (previous, current) for continuous analysis between the two.
/// CCDAlgorithm classes follow the same logic as CollisionDetectionAlgorithm.
/// This base class for CCD algorithms enforces the implementation of a function
/// (UpdatePreviousTimestepGeometry) to cache/store previous state of the colliding
/// geometries.
///
class CCDAlgorithm : public CollisionDetectionAlgorithm
{
public:
    ///
    /// \brief Call with finalized geometries to be cached for reference
    /// when computing continuous collision in a future iteration.
    ///
    virtual void updatePreviousTimestepGeometry(
        std::shared_ptr<const Geometry> geomA, std::shared_ptr<const Geometry> geomB) = 0;

protected:
    CCDAlgorithm() : CollisionDetectionAlgorithm() {}
};
} // namespace imstk
