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

#include "imstkCollisionDetection.h"

namespace imstk
{
class PointSet;
class SurfaceMesh;
struct CollisionData;

///
/// \class PointSetToSurfaceMeshCD
///
/// \brief PointSet to surface mesh collision detection
///
class PointSetToSurfaceMeshCD : public CollisionDetection
{
public:

    ///
    /// \brief Constructor
    ///
    PointSetToSurfaceMeshCD(const std::shared_ptr<PointSet>&      pointset,
                            const std::shared_ptr<SurfaceMesh>&   surfMesh,
                            const std::shared_ptr<CollisionData>& colData);

    ///
    /// \brief Detect collision and compute collision data
    /// Do nothing here, as the collision detection is performed by a static octree,
    /// which is a static member of CollisionDetection class
    ///
    void computeCollisionData() override {}
};
}
