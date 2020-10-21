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

#include "imstkCollisionHandling.h"

namespace imstk
{
class ImplicitGeometryToPointSetCD;
struct CollisionData;
class SPHObject;

///
/// \class SPHCollisionHandling
///
/// \brief Supports SPH vs analytic and SPH vs Implicit Geometry
///
class SPHCollisionHandling : public CollisionHandling
{
public:
    SPHCollisionHandling(const Side&                    side,
                         std::shared_ptr<CollisionData> colData,
                         std::shared_ptr<SPHObject>     obj);

    SPHCollisionHandling() = delete;

    virtual ~SPHCollisionHandling() override = default;

public:
    void setNumberOfIterations(int iterations) { this->m_iterations = iterations; }

    void setDetection(std::shared_ptr<ImplicitGeometryToPointSetCD> colDetect) { this->m_colDetect = colDetect; }

    ///
    /// \brief Compute forces based on collision data
    ///
    virtual void processCollisionData() override;

private:
    std::shared_ptr<SPHObject> m_SPHObject = nullptr;
    std::shared_ptr<ImplicitGeometryToPointSetCD> m_colDetect = nullptr;
    int m_iterations = 1;
};
} // end namespace imstk
