/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version B.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-B.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   =========================================================================*/

#include "imstkMeshToMeshCD.h"

#include "imstkCollidingObject.h"
#include "imstkCollisionData.h"
#include "imstkSurfaceMesh.h"

#include <g3log/g3log.hpp>

namespace imstk {


void
EECallback(unsigned int e1_v1, unsigned int e1_v2,
           unsigned int e2_v1, unsigned int e2_v2,
           float t, void *userdata)
{
    auto CD = reinterpret_cast<MeshToMeshCD*>(userdata);
    CD->getType();
    LOG(INFO) <<"EE: e1("<<e1_v1<<", "<<e1_v2<<"), e2("<<e2_v1<<", "<<e2_v2<<") \t@ t="<<t;
}

void
VFCallback(unsigned int vid, unsigned int fid,
           float t, void *userdata)
{
    auto CD = reinterpret_cast<MeshToMeshCD*>(userdata);
    CD->getType();
    LOG(INFO) <<"VF: v("<<vid<<"), f("<<fid<<") \t\t@ t="<<t;
}

void
MeshToMeshCD::computeCollisionData(std::shared_ptr<CollidingObject> objA,
                                   std::shared_ptr<CollidingObject> objB,
                                   CollisionData& colDataA,
                                   CollisionData& colDataB)
{
    auto geomA = std::dynamic_pointer_cast<SurfaceMesh>(objA->getCollidingGeometry());
    auto geomB = std::dynamic_pointer_cast<SurfaceMesh>(objB->getCollidingGeometry());

    // Geometries check
    if (geomA == nullptr || geomB == nullptr)
    {
        LOG(WARNING) << "MeshToMeshCD::computeCollisionData error: invalid geometries.";
        return;
    }

    // Init models for continuous CD
    if(!m_initialized)
    {
        // Init
        modelA = std::make_shared<DeformModel>(geomA->getVerticesPositions(), geomA->getTrianglesVertices());
        modelB = std::make_shared<DeformModel>(geomB->getVerticesPositions(), geomB->getTrianglesVertices());

        // Setup Callbacks
        modelA->SetEECallBack(EECallback, this);
        modelA->SetVFCallBack(VFCallback, this);

        // Build BVH
        modelA->BuildBVH(false);
        modelB->BuildBVH(false);

        m_initialized = true;
    }
    else
    {
        // Update model
        modelA->UpdateVert(geomA->getVerticesPositions());
        modelB->UpdateVert(geomB->getVerticesPositions());
        modelB->UpdateBoxes();
        modelB->UpdateBoxes();

        // Update BVH
        modelA->RefitBVH();
        modelB->RefitBVH();

        // Reset Results
        modelA->ResetCounter();
        modelB->ResetCounter();

        // Collide
        modelA->Collide(modelB.get());
    }
}

}
