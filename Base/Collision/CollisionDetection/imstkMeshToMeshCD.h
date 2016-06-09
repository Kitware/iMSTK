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

class SurfaceMesh;
class CollisionData;

class MeshToMeshCD : public CollisionDetection
{
public:

    ///
    /// \brief Constructor
    ///
    MeshToMeshCD(std::shared_ptr<SurfaceMesh> planeA,
                 std::shared_ptr<SurfaceMesh> sphereB,
                 CollisionData& CDA,
                 CollisionData& CDB);

    ///
    /// \brief Destructor
    ///
    ~MeshToMeshCD() = default;

    ///
    /// \brief Detect collision and compute collision data
    ///
    void computeCollisionData() override;

private:

    bool m_initialized = false;
    std::shared_ptr<SurfaceMesh> m_meshA;
    std::shared_ptr<SurfaceMesh> m_meshB;
    std::shared_ptr<DeformModel> m_modelA;
    std::shared_ptr<DeformModel> m_modelB;
};
}

#endif // ifndef imstkMeshToMeshCD_h
