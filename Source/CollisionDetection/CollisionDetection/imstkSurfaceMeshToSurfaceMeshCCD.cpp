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
#include "DeformModel.h"
#include "imstkCollisionData.h"
#include "imstkSurfaceMesh.h"

namespace imstk
{
SurfaceMeshToSurfaceMeshCCD::SurfaceMeshToSurfaceMeshCCD(std::shared_ptr<SurfaceMesh>   meshA,
                                                         std::shared_ptr<SurfaceMesh>   meshB,
                                                         std::shared_ptr<CollisionData> colData) :
    CollisionDetection(CollisionDetection::Type::SurfaceMeshToSurfaceMeshCCD, colData),
    m_meshA(meshA),
    m_meshB(meshB)
{
    m_triangleIndicesA = std::vector<std::array<size_t, 3>>(meshA->getNumTriangles());
    m_triangleIndicesB = std::vector<std::array<size_t, 3>>(meshB->getNumTriangles());

    // Copy data A
    const VecDataArray<int, 3>& indicesA = *meshA->getTriangleIndices();
    for (int i = 0; i < meshA->getNumTriangles(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            m_triangleIndicesA[i][j] = indicesA[i][j];
        }
    }
    const VecDataArray<double, 3>& positionsA = *meshA->getVertexPositions();
    for (int i = 0; i < meshA->getNumVertices(); i++)
    {
        m_positionsA[i] = positionsA[i];
    }

    // Copy data B
    const VecDataArray<int, 3>& indicesB = *meshB->getTriangleIndices();
    for (int i = 0; i < meshB->getNumTriangles(); i++)
    {
        for (int j = 0; j < 3; j++)
        {
            m_triangleIndicesB[i][j] = indicesB[i][j];
        }
    }
    const VecDataArray<double, 3>& positionsB = *meshB->getVertexPositions();
    for (int i = 0; i < meshB->getNumVertices(); i++)
    {
        m_positionsB[i] = positionsB[i];
    }

    m_modelA = std::make_shared<DeformModel>(m_positionsA, m_triangleIndicesA);
    m_modelB = std::make_shared<DeformModel>(m_positionsB, m_triangleIndicesB);

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

    const VecDataArray<double, 3>& positionsA = *m_meshA->getVertexPositions();
    for (int i = 0; i < m_meshA->getNumVertices(); i++)
    {
        m_positionsA[i] = positionsA[i];
    }
    const VecDataArray<double, 3>& positionsB = *m_meshB->getVertexPositions();
    for (int i = 0; i < m_meshB->getNumVertices(); i++)
    {
        m_positionsB[i] = positionsB[i];
    }

    // Update model
    m_modelA->UpdateVert(m_positionsA);
    m_modelB->UpdateVert(m_positionsB);
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
