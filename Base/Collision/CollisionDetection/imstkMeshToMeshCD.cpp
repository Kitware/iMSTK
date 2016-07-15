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

#include "imstkCollisionData.h"
#include "imstkSurfaceMesh.h"

#include <g3log/g3log.hpp>

namespace imstk {

MeshToMeshCD::MeshToMeshCD(std::shared_ptr<SurfaceMesh> meshA,
                           std::shared_ptr<SurfaceMesh> meshB,
                           CollisionData& colData) :
    CollisionDetection(CollisionDetection::Type::PlaneToSphere, colData),
    m_meshA(meshA),
    m_meshB(meshB)
{
    m_modelA = std::make_shared<DeformModel>(meshA->getVerticesPositions(), meshA->getTrianglesVertices());
    m_modelB = std::make_shared<DeformModel>(meshB->getVerticesPositions(), meshB->getTrianglesVertices());

    // Setup Callbacks
    m_modelA->SetEECallBack(MeshToMeshCD::EECallback, this);
    m_modelA->SetVFCallBack(MeshToMeshCD::VFCallbackA, this);
    m_modelB->SetVFCallBack(MeshToMeshCD::VFCallbackB, this);

    // Build BVH
    m_modelA->BuildBVH(false);
    m_modelB->BuildBVH(false);
}

void
MeshToMeshCD::computeCollisionData()
{
    // Update model
    m_modelA->UpdateVert(m_meshA->getVerticesPositions());
    m_modelB->UpdateVert(m_meshB->getVerticesPositions());
    m_modelB->UpdateBoxes();
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
MeshToMeshCD::EECallback(unsigned int eA_v1, unsigned int eA_v2,
                         unsigned int eB_v1, unsigned int eB_v2,
                         float t, void *userdata)
{
    auto CD = reinterpret_cast<MeshToMeshCD*>(userdata);
    if(CD == nullptr)
    {
        return;
    }

    auto colData = CD->getCollisionData();
    colData.EEColData.push_back(EdgeEdgeCollisionData(eA_v1, eA_v2, eB_v1, eB_v2, t));
    //LOG(INFO) <<"EE: eA("<<eA_v1<<", "<<eA_v2<<"), eB("<<eB_v1<<", "<<eB_v2<<") \t@ t="<<t;
}

void
MeshToMeshCD::VFCallbackA(unsigned int fidA, unsigned int vidB,
                          float t, void *userdata)
{
    auto CD = reinterpret_cast<MeshToMeshCD*>(userdata);
    if(CD == nullptr)
    {
        return;
    }

    auto colData = CD->getCollisionData();
    colData.TVColData.push_back(TriangleVertexCollisionData(fidA, vidB, t));
    //LOG(INFO) <<"VF: fA("<<fidA<<"), vB("<<vidB<<") \t\t@ t="<<t;
}

void
MeshToMeshCD::VFCallbackB(unsigned int fidB, unsigned int vidA,
                          float t, void *userdata)
{
    auto CD = reinterpret_cast<MeshToMeshCD*>(userdata);
    if(CD == nullptr)
    {
        return;
    }
    auto colData = CD->getCollisionData();
    colData.VTColData.push_back(VertexTriangleCollisionData(vidA, fidB, t));
    //LOG(INFO) <<"VF: vA("<<vidA<<"), fB("<<fidB<<") \t\t@ t="<<t;
}

}
