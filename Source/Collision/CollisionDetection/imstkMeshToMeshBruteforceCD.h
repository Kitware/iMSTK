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

#ifndef imstkMeshToMeshBruteforceCD_h
#define imstkMeshToMeshBruteforceCD_h

#include <memory>

#include "imstkCollisionDetection.h"

namespace imstk
{
class Geometry;
class SurfaceMesh;
class CollisionData;

///
/// \class MeshToMeshBruteForceCD
///
/// \brief Mesh to mesh collision with brute force strategy
///
class MeshToMeshBruteForceCD : public CollisionDetection
{
public:

    ///
    /// \brief Constructor
    ///
    MeshToMeshBruteForceCD(std::shared_ptr<Geometry> obj1,
                           std::shared_ptr<SurfaceMesh> obj2,
                           CollisionData& colData) :
        CollisionDetection(CollisionDetection::Type::MeshToMeshBruteForce,
                           colData),
        m_object1(obj1),
        m_object2(obj2){}

    ///
    /// \brief Destructor
    ///
    ~MeshToMeshBruteForceCD() = default;

    ///
    /// \brief Detect collision and compute collision data
    ///
    void computeCollisionData() override;

private:
    ///
    /// \brief Do a broad phase collision check using AABB
    ///
    bool doBroadPhaseCollisionCheck() const;

    double m_proximityTolerance = 0.1;        ///> proximity tolerance used for collision
    std::shared_ptr<Geometry> m_object1;      ///> object 1
    std::shared_ptr<SurfaceMesh> m_object2;   ///> object 2
};
}

#endif // ifndef imstkMeshToMeshBruteForceCD_h
