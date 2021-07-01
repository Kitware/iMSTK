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
//#pragma once
//
//#include "imstkCollisionDetectionAlgorithm.h"
//
//#include <fcl/fcl.h>
//#include <fcl/geometry/bvh/BVH_model.h>
//#include <fcl/math/bv/OBBRSS.h>
//#include <vector>
//#include <array>
//
//namespace imstk
//{
//class SurfaceMesh;
//
/////
///// \class SurfaceMeshToSurfaceMeshCCD
/////
///// \brief Continuous collision detection for surface meshes
/////
//class SurfaceMeshToSurfaceMeshCCD : public CollisionDetectionAlgorithm
//{
//public:
//    SurfaceMeshToSurfaceMeshCCD();
//    virtual ~SurfaceMeshToSurfaceMeshCCD() override = default;
//
//    ///
//    /// \brief Returns collision detection type string name
//    ///
//    virtual const std::string getTypeName() const override { return "SurfaceMeshToSurfaceMeshCCD"; }
//
//public:
//    ///
//    /// \brief Compute collision data for AB simulatenously
//    ///
//    virtual void computeCollisionDataAB(
//        std::shared_ptr<Geometry>          geomA,
//        std::shared_ptr<Geometry>          geomB,
//        CDElementVector<CollisionElement>& elementsA,
//        CDElementVector<CollisionElement>& elementsB) override;
//
//protected:
//    void setup(std::shared_ptr<SurfaceMesh> geomA, std::shared_ptr<SurfaceMesh> geomB);
//
//protected:
//    CDElementVector<CollisionElement>& getElementsA() { return *m_elementsA; }
//    CDElementVector<CollisionElement>& getElementsB() { return *m_elementsB; }
//
//    VecDataArray<int, 3>& getIndicesA();
//    VecDataArray<int, 3>& getIndicesB();
//
//    ///
//    /// \brief Callback defining edge to edge collision (A - B)
//    ///
//    static void EECallback(unsigned int eA_v1, unsigned int eA_v2,
//                           unsigned int eB_v1, unsigned int eB_v2,
//                           float t, void* userdata);
//    ///
//    /// \brief Callback defining vertex to triangle collision (A - B)
//    ///
//    static void VFCallbackA(unsigned int fidA, unsigned int vidB,
//                            float t, void* userdata);
//    ///
//    /// \brief Callback defining vertex to triangle collision (B - A)
//    ///
//    static void VFCallbackB(unsigned int fidB, unsigned int vidA,
//                            float t, void* userdata);
//
//protected:
//    std::shared_ptr<SurfaceMesh> m_meshA;   ///> Mesh A
//    std::shared_ptr<SurfaceMesh> m_meshB;   ///> Mesh B
//
//    std::shared_ptr<fcl::BVHModel<fcl::OBBRSSd>> m_modelA;
//    std::shared_ptr<fcl::BVHModel<fcl::OBBRSSd>> m_modelB;
//
//    std::vector<Vec3d> m_positionsA;
//    std::vector<fcl::Triangle> m_fclIndicesA;
//
//    std::vector<Vec3d> m_positionsB;
//    std::vector<fcl::Triangle> m_fclIndicesB;
//
//    CDElementVector<CollisionElement>* m_elementsA;
//    CDElementVector<CollisionElement>* m_elementsB;
//};
//}