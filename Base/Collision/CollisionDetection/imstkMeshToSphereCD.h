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

#ifndef imstkMeshToSphereCD_h
#define imstkMeshToSphereCD_h

#include <memory>

#include "imstkCollisionDetection.h"

namespace imstk
{
class Mesh;
class Sphere;
class CollisionData;

///
/// \class MeshToSphereCD
///
/// \brief Mesh to sphere collision detection
///
class MeshToSphereCD : public CollisionDetection
{
public:

    ///
    /// \brief Constructor
    ///
    MeshToSphereCD(std::shared_ptr<Mesh> mesh,
                   std::shared_ptr<Sphere> sphere,
                   CollisionData& colData) :
        CollisionDetection(CollisionDetection::Type::MeshToSphere,
                           colData),
        m_mesh(mesh),
        m_sphere(sphere){}

    ///
    /// \brief Destructor
    ///
    ~MeshToSphereCD() = default;

    ///
    /// \brief Detect collision and compute collision data
    ///
    void computeCollisionData() override;

private:

    std::shared_ptr<Mesh> m_mesh;       ///> Mesh
    std::shared_ptr<Sphere> m_sphere;  ///> Sphere
};
}

#endif // ifndef imstkMeshToSphereCD_h
