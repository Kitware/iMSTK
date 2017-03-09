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

#ifndef imstkMeshToPlaneCD_h
#define imstkMeshToPlaneCD_h

#include <memory>

#include "imstkCollisionDetection.h"

namespace imstk
{

class Mesh;
class Plane;
class CollisionData;

///
/// \class MeshToPlaneCD
///
/// \brief Mesh to plane collision detection
///
class MeshToPlaneCD : public CollisionDetection
{
public:

    ///
    /// \brief Constructor
    ///
    MeshToPlaneCD(std::shared_ptr<Mesh> mesh,
                  std::shared_ptr<Plane> plane,
                  CollisionData& colData) :
				  CollisionDetection(CollisionDetection::Type::MeshToSphere,
                  colData),
				  m_mesh(mesh),
				  m_plane(plane){}

    ///
    /// \brief Destructor
    ///
    ~MeshToPlaneCD() = default;

    ///
    /// \brief Detect collision and compute collision data
    ///
    void computeCollisionData() override;

private:

    std::shared_ptr<Mesh> m_mesh;      ///> Mesh
    std::shared_ptr<Plane> m_plane;    ///> Plane
};

}

#endif // ifndef imstkMeshToPlaneCD_h
