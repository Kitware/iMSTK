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

#ifndef imstkMeshToMeshCD_h
#define imstkMeshToMeshCD_h

#include "imstkCollisionDetection.h"

#include "DeformModel.h"

#include <memory>

namespace imstk {

class CollidingObject;
class CollisionData;

class MeshToMeshCD : public CollisionDetection
{
public:

    ///
    /// \brief Constructor
    ///
    MeshToMeshCD() : CollisionDetection(CollisionDetection::Type::MeshToMesh) {}

    ///
    /// \brief Destructor
    ///
    ~MeshToMeshCD() = default;

    ///
    /// \brief Detect collision and compute collision data
    ///
    void computeCollisionData(std::shared_ptr<CollidingObject> objA,
                              std::shared_ptr<CollidingObject> objB,
                              CollisionData& colDataA,
                              CollisionData& colDataB) override;

private:

    void EECallback(unsigned int e1_v1, unsigned int e1_v2, unsigned int e2_v1, unsigned int e2_v2, float t);
    void VFCallback(unsigned int vid, unsigned int fid, float t);

    bool m_initialized = false;
    std::shared_ptr<DeformModel> modelA;
    std::shared_ptr<DeformModel> modelB;
};
}

#endif // ifndef imstkMeshToMeshCD_h
