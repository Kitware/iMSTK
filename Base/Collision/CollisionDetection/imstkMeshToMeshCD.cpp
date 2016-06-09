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
VFCallback1(unsigned int fid1, unsigned int vid2,
            float t, void *userdata)
{
    auto CD = reinterpret_cast<MeshToMeshCD*>(userdata);
    CD->getType();
    LOG(INFO) <<"VF: v2("<<vid2<<"), f1("<<fid1<<") \t\t@ t="<<t;
}

void
VFCallback2(unsigned int fid2, unsigned int vid1,
            float t, void *userdata)
{
    auto CD = reinterpret_cast<MeshToMeshCD*>(userdata);
    CD->getType();
    LOG(INFO) <<"VF: v1("<<vid1<<"), f2("<<fid2<<") \t\t@ t="<<t;
}

MeshToMeshCD::MeshToMeshCD(std::shared_ptr<SurfaceMesh> meshA,
                           std::shared_ptr<SurfaceMesh> meshB,
                           CollisionData& CDA,
                           CollisionData& CDB) :
    CollisionDetection(CollisionDetection::Type::PlaneToSphere, CDA, CDB),
    m_meshA(meshA),
    m_meshB(meshB)
{
    m_modelA = std::make_shared<DeformModel>(meshA->getVerticesPositions(), meshA->getTrianglesVertices());
    m_modelB = std::make_shared<DeformModel>(meshB->getVerticesPositions(), meshB->getTrianglesVertices());

    // Setup Callbacks
    m_modelA->SetEECallBack(EECallback, this);
    m_modelA->SetVFCallBack(VFCallback1, this);
    m_modelB->SetVFCallBack(VFCallback2, this);

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

}
