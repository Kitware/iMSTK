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
#include <memory>

namespace imstk
{
class Geometry;
class PointSet;
class SurfaceMesh;
struct CollisionData;

///
/// \class PointSetToTriMeshCD
///
/// \brief PointSet to sphere collision detection
///
class PointSetToVolumeMeshCD : public CollisionDetection
{
friend class SurfaceMeshToSurfaceMeshCD;
friend class SurfaceMeshToVolumeMeshCD;
friend class VolumeMeshToVolumeMeshCD;
public:

    ///
    /// \brief Constructor
    ///
    PointSetToVolumeMeshCD(std::shared_ptr<PointSet>      pointset,
                           std::shared_ptr<SurfaceMesh>   triMesh,
                           std::shared_ptr<CollisionData> colData);

    ///
    /// \brief Detect collision and compute collision data
    ///
    void computeCollisionData() override;

private:
    std::shared_ptr<PointSet>    m_pointset;
    std::shared_ptr<SurfaceMesh> m_triMesh;
};
}
