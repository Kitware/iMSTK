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

// std library
#include <memory>

// imstk
#include "imstkCollisionDetection.h"
#include "DeformModel.h"

namespace imstk
{
class SurfaceMesh;
class CollisionData;

///
/// \class MeshToMeshCD
///
/// \brief Base class for mesh-to-mesh collision detection
///
class MeshToMeshCD : public CollisionDetection
{
public:

    ///
    /// \brief Constructor
    ///
    MeshToMeshCD(std::shared_ptr<SurfaceMesh> meshA,
                 std::shared_ptr<SurfaceMesh> meshB,
                 std::shared_ptr<CollisionData> colData);

    ///
    /// \brief Destructor
    ///
    ~MeshToMeshCD() = default;

    ///
    /// \brief Detect collision and compute collision data
    ///
    void computeCollisionData() override;

private:

    ///
    /// \brief Callback defining edge to edge collision (A - B)
    ///
    static void EECallback(unsigned int eA_v1, unsigned int eA_v2,
                           unsigned int eB_v1, unsigned int eB_v2,
                           float t, void *userdata);
    ///
    /// \brief Callback defining vertex to triangle collision (A - B)
    ///
    static void VFCallbackA(unsigned int fidA, unsigned int vidB,
                            float t, void *userdata);
    ///
    /// \brief Callback defining vertex to triangle collision (B - A)
    ///
    static void VFCallbackB(unsigned int fidB, unsigned int vidA,
                            float t, void *userdata);

    std::shared_ptr<SurfaceMesh> m_meshA;   ///> Mesh A
    std::shared_ptr<SurfaceMesh> m_meshB;   ///> Mesh B
    std::shared_ptr<DeformModel> m_modelA;  ///>
    std::shared_ptr<DeformModel> m_modelB;  ///>
};
}

