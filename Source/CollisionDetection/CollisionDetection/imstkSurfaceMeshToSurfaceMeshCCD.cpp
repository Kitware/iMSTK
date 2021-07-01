///*=========================================================================
//
//   Library: iMSTK
//
//   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
//   & Imaging in Medicine, Rensselaer Polytechnic Institute.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0.txt
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//=========================================================================*/
//
//#include "imstkSurfaceMeshToSurfaceMeshCCD.h"
//#include "imstkCollisionData.h"
//#include "imstkSurfaceMesh.h"
//
//#include <fcl/narrowphase/collision_object.h>
//#include <fcl/narrowphase/collision_request.h>
//#include <fcl/narrowphase/collision_result.h>
//
//namespace imstk
//{
//SurfaceMeshToSurfaceMeshCCD::SurfaceMeshToSurfaceMeshCCD() :
//    m_modelA(std::make_shared<fcl::BVHModel<fcl::OBBRSSd>>()),
//    m_modelB(std::make_shared<fcl::BVHModel<fcl::OBBRSSd>>())
//{
//    setInputPortReq<SurfaceMesh>(0);
//    setInputPortReq<SurfaceMesh>(1);
//
//    // By default generate contact data for both sides
//    setGenerateCD(true, true);
//}
//
//void
//SurfaceMeshToSurfaceMeshCCD::setup(std::shared_ptr<SurfaceMesh> surfMeshA, std::shared_ptr<SurfaceMesh> surfMeshB)
//{
//    //if (m_meshA == surfMeshA && m_meshB == surfMeshB)
//    //{
//    //    return;
//    //}
//
//    //m_meshA = surfMeshA;
//    //m_meshB = surfMeshB;
//
//    //m_triangleIndicesA = std::vector<std::array<size_t, 3>>(surfMeshA->getNumTriangles());
//    //m_triangleIndicesB = std::vector<std::array<size_t, 3>>(surfMeshB->getNumTriangles());
//
//    //// Copy data A
//    //const VecDataArray<int, 3>& indicesA = *surfMeshA->getTriangleIndices();
//    //for (int i = 0; i < surfMeshA->getNumTriangles(); i++)
//    //{
//    //    for (int j = 0; j < 3; j++)
//    //    {
//    //        m_triangleIndicesA[i][j] = indicesA[i][j];
//    //    }
//    //}
//    //const VecDataArray<double, 3>& positionsA = *surfMeshA->getVertexPositions();
//    //m_positionsA.resize(positionsA.size());
//    //for (int i = 0; i < surfMeshA->getNumVertices(); i++)
//    //{
//    //    m_positionsA[i] = positionsA[i];
//    //}
//
//    //// Copy data B
//    //const VecDataArray<int, 3>& indicesB = *surfMeshB->getTriangleIndices();
//    //for (int i = 0; i < surfMeshB->getNumTriangles(); i++)
//    //{
//    //    for (int j = 0; j < 3; j++)
//    //    {
//    //        m_triangleIndicesB[i][j] = indicesB[i][j];
//    //    }
//    //}
//    //const VecDataArray<double, 3>& positionsB = *surfMeshB->getVertexPositions();
//    //m_positionsB.resize(positionsB.size());
//    //for (int i = 0; i < surfMeshB->getNumVertices(); i++)
//    //{
//    //    m_positionsB[i] = positionsB[i];
//    //}
//
//    //m_modelA = std::make_shared<DeformModel>(m_positionsA, m_triangleIndicesA);
//    //m_modelB = std::make_shared<DeformModel>(m_positionsB, m_triangleIndicesB);
//
//    //// Setup Callbacks
//    //m_modelA->SetEECallBack(SurfaceMeshToSurfaceMeshCCD::EECallback, this);
//    //m_modelA->SetVFCallBack(SurfaceMeshToSurfaceMeshCCD::VFCallbackA, this);
//    //m_modelB->SetVFCallBack(SurfaceMeshToSurfaceMeshCCD::VFCallbackB, this);
//
//    //// Build BVH
//    //m_modelA->BuildBVH(false);
//    //m_modelB->BuildBVH(false);
//}
//
//void
//SurfaceMeshToSurfaceMeshCCD::computeCollisionDataAB(
//    std::shared_ptr<Geometry>          geomA,
//    std::shared_ptr<Geometry>          geomB,
//    CDElementVector<CollisionElement>& elementsA,
//    CDElementVector<CollisionElement>& elementsB)
//{
//    std::shared_ptr<SurfaceMesh> surfMeshA = std::dynamic_pointer_cast<SurfaceMesh>(geomA);
//    std::shared_ptr<SurfaceMesh> surfMeshB = std::dynamic_pointer_cast<SurfaceMesh>(geomB);
//
//    fcl::Transform3d poseA = fcl::Transform3d::Identity();
//
//    VecDataArray<double, 3>& verticesA = *surfMeshA->getVertexPositions();
//    if (verticesA.size() != m_positionsA.size())
//    {
//        m_positionsA.resize(verticesA.size());
//    }
//    std::copy_n(verticesA.getPointer(), verticesA.size(), m_positionsA.data());
//    const VecDataArray<int, 3>& indicesA = *surfMeshA->getTriangleIndices();
//    if (indicesA.size() != m_fclIndicesA.size())
//    {
//        m_fclIndicesA.resize(indicesA.size());
//        for (int i = 0; i < indicesA.size(); i++)
//        {
//            for (int j = 0; j < 3; j++)
//            {
//                m_fclIndicesA[i][j] = indicesA[i][j];
//            }
//        }
//    }
//    fcl::BVHModel<fcl::OBBRSSd> modelA;
//    modelA.beginModel();
//    modelA.addSubModel(m_positionsA, m_fclIndicesA);
//    modelA.endModel();
//
//
//    fcl::Transform3d poseB = fcl::Transform3d::Identity();
//
//    VecDataArray<double, 3>& verticesB = *surfMeshB->getVertexPositions();
//    if (verticesB.size() != m_positionsB.size())
//    {
//        m_positionsB.resize(verticesB.size());
//    }
//    std::copy_n(verticesB.getPointer(), verticesB.size(), m_positionsB.data());
//    const VecDataArray<int, 3>& indicesB = *surfMeshB->getTriangleIndices();
//    if (indicesB.size() != m_fclIndicesB.size())
//    {
//        m_fclIndicesB.resize(indicesB.size());
//        for (int i = 0; i < indicesB.size(); i++)
//        {
//            for (int j = 0; j < 3; j++)
//            {
//                m_fclIndicesB[i][j] = indicesB[i][j];
//            }
//        }
//    }
//    fcl::BVHModel<fcl::OBBRSSd> modelB;
//    modelB.beginModel();
//    modelB.addSubModel(m_positionsB, m_fclIndicesB);
//    modelB.endModel();
//
//    // set the distance request structure, here we just use the default setting
//    fcl::CollisionRequestd request;
//    request.enable_contact = true;
//    // result will be returned via the collision result structure
//    fcl::CollisionResultd result;
//
//    const size_t numContacts = fcl::collide<double>(&modelA, poseA, &modelB, poseB, request, result);
//
//    for (size_t i = 0; i < result.numContacts(); i++)
//    {
//        fcl::Contactd contact = result.getContact(i);
//        if (contact.b1 != -1)
//        {
//            // How do I know if b1 refers to a point or triangle?
//        }
//    }
//}
//
//VecDataArray<int, 3>&
//SurfaceMeshToSurfaceMeshCCD::getIndicesA()
//{
//    return *m_meshA->getTriangleIndices();
//}
//
//VecDataArray<int, 3>&
//SurfaceMeshToSurfaceMeshCCD::getIndicesB()
//{
//    return *m_meshB->getTriangleIndices();
//}
//
//void
//SurfaceMeshToSurfaceMeshCCD::EECallback(unsigned int eA_v1, unsigned int eA_v2,
//                                        unsigned int eB_v1, unsigned int eB_v2,
//                                        float t, void* userdata)
//{
//    auto CD = reinterpret_cast<SurfaceMeshToSurfaceMeshCCD*>(userdata);
//    if (CD == nullptr)
//    {
//        return;
//    }
//
//    CDElementVector<CollisionElement>& elementsA = CD->getElementsA();
//    CDElementVector<CollisionElement>& elementsB = CD->getElementsB();
//
//    EdgeIndexElement elemA;
//    elemA.p1Index = eA_v1;
//    elemA.p2Index = eA_v2;
//
//    EdgeIndexElement elemB;
//    elemB.p1Index = eB_v1;
//    elemB.p2Index = eB_v2;
//
//    // Does this need to be safe append?? can this callback occur in parallel?
//    elementsA.safeAppend(elemA);
//    elementsB.safeAppend(elemB);
//
//    //LOG(INFO) <<"EE: eA("<<eA_v1<<", "<<eA_v2<<"), eB("<<eB_v1<<", "<<eB_v2<<") \t@ t="<<t;
//}
//
//void
//SurfaceMeshToSurfaceMeshCCD::VFCallbackA(unsigned int fidA, unsigned int vidB,
//                                         float t, void* userdata)
//{
//    auto CD = reinterpret_cast<SurfaceMeshToSurfaceMeshCCD*>(userdata);
//    if (CD == nullptr)
//    {
//        return;
//    }
//
//    CDElementVector<CollisionElement>& elementsA = CD->getElementsA();
//    CDElementVector<CollisionElement>& elementsB = CD->getElementsB();
//    VecDataArray<int, 3>&              indicesA  = CD->getIndicesA();
//
//    TriangleIndexElement elemA;
//    elemA.p1Index = indicesA[fidA][0];
//    elemA.p2Index = indicesA[fidA][1];
//    elemA.p3Index = indicesA[fidA][2];
//
//    PointIndexElement elemB;
//    elemB.ptIndex = vidB;
//
//    elementsA.safeAppend(elemA);
//    elementsB.safeAppend(elemB);
//
//    //LOG(INFO) <<"VF: fA("<<fidA<<"), vB("<<vidB<<") \t\t@ t="<<t;
//}
//
//void
//SurfaceMeshToSurfaceMeshCCD::VFCallbackB(unsigned int fidB, unsigned int vidA,
//                                         float t, void* userdata)
//{
//    auto CD = reinterpret_cast<SurfaceMeshToSurfaceMeshCCD*>(userdata);
//    if (CD == nullptr)
//    {
//        return;
//    }
//
//    CDElementVector<CollisionElement>& elementsA = CD->getElementsA();
//    CDElementVector<CollisionElement>& elementsB = CD->getElementsB();
//    VecDataArray<int, 3>&              indicesB  = CD->getIndicesB();
//
//    PointIndexElement elemA;
//    elemA.ptIndex = vidA;
//
//    TriangleIndexElement elemB;
//    elemB.p1Index = indicesB[fidB][0];
//    elemB.p2Index = indicesB[fidB][1];
//    elemB.p3Index = indicesB[fidB][2];
//
//    elementsA.safeAppend(elemA);
//    elementsB.safeAppend(elemB);
//
//    //LOG(INFO) <<"VF: vA("<<vidA<<"), fB("<<fidB<<") \t\t@ t="<<t;
//}
//}