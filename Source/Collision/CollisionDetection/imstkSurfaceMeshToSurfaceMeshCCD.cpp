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

#include "imstkSurfaceMeshToSurfaceMeshCCD.h"

#include "imstkCollisionData.h"
#include "imstkSurfaceMesh.h"
#include "DeformModel.h"

#include <g3log/g3log.hpp>

namespace imstk
{
SurfaceMeshToSurfaceMeshCCD::SurfaceMeshToSurfaceMeshCCD(std::shared_ptr<SurfaceMesh>   meshA,
                                                         std::shared_ptr<SurfaceMesh>   meshB,
                                                         std::shared_ptr<CollisionData> colData) :
    CollisionDetection(CollisionDetection::Type::SurfaceMeshToSurfaceMeshCCD, colData),
    m_meshA(meshA),
    m_meshB(meshB)
{
    m_modelA = std::make_shared<DeformModel>(meshA->getVertexPositions(), meshA->getTrianglesVertices());
    m_modelB = std::make_shared<DeformModel>(meshB->getVertexPositions(), meshB->getTrianglesVertices());

    // Setup Callbacks
    m_modelA->SetEECallBack(SurfaceMeshToSurfaceMeshCCD::EECallback, this);
    m_modelA->SetVFCallBack(SurfaceMeshToSurfaceMeshCCD::VFCallbackA, this);
    m_modelB->SetVFCallBack(SurfaceMeshToSurfaceMeshCCD::VFCallbackB, this);

    // Build BVH
    m_modelA->BuildBVH(false);
    m_modelB->BuildBVH(false);
}

void
SurfaceMeshToSurfaceMeshCCD::computeCollisionData()
{
    // Clear collisionData
    m_colData->clearAll();

    // Update model
    m_modelA->UpdateVert(m_meshA->getVertexPositions());
    m_modelB->UpdateVert(m_meshB->getVertexPositions());
    m_modelA->UpdateBoxes();
    m_modelB->UpdateBoxes();

    // Update BVH
    m_modelA->RefitBVH();
    m_modelB->RefitBVH();

    // Reset Results
    m_modelA->ResetCounter();
    m_modelB->ResetCounter();

    // Collide
    m_modelA->Collide(m_modelB.get());
}

void
SurfaceMeshToSurfaceMeshCCD::EECallback(unsigned int eA_v1, unsigned int eA_v2,
                                        unsigned int eB_v1, unsigned int eB_v2,
                                        float t, void* userdata)
{
    auto CD = reinterpret_cast<SurfaceMeshToSurfaceMeshCCD*>(userdata);
    if (CD == nullptr)
    {
        return;
    }

    auto colData = CD->getCollisionData();
    colData->EEColData.safeAppend(EdgeEdgeCollisionDataElement { { eA_v1, eA_v2 }, { eB_v1, eB_v2 }, t });
    //LOG(INFO) <<"EE: eA("<<eA_v1<<", "<<eA_v2<<"), eB("<<eB_v1<<", "<<eB_v2<<") \t@ t="<<t;
}

void
SurfaceMeshToSurfaceMeshCCD::VFCallbackA(unsigned int fidA, unsigned int vidB,
                                         float t, void* userdata)
{
    auto CD = reinterpret_cast<SurfaceMeshToSurfaceMeshCCD*>(userdata);
    if (CD == nullptr)
    {
        return;
    }

    auto colData = CD->getCollisionData();
    colData->TVColData.safeAppend(TriangleVertexCollisionDataElement { fidA, vidB, (double)t });
    //LOG(INFO) <<"VF: fA("<<fidA<<"), vB("<<vidB<<") \t\t@ t="<<t;
}

void
SurfaceMeshToSurfaceMeshCCD::VFCallbackB(unsigned int fidB, unsigned int vidA,
                                         float t, void* userdata)
{
    auto CD = reinterpret_cast<SurfaceMeshToSurfaceMeshCCD*>(userdata);
    if (CD == nullptr)
    {
        return;
    }
    auto colData = CD->getCollisionData();
    colData->VTColData.safeAppend(VertexTriangleCollisionDataElement { vidA, fidB, (double)t });
    //LOG(INFO) <<"VF: vA("<<vidA<<"), fB("<<fidB<<") \t\t@ t="<<t;
}
}
