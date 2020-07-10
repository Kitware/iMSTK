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
//#pragma once
//
//#include "imstkGeometryAlgorithm.h"
//
//namespace imstk
//{
//class SurfaceMesh;
//
/////
///// \class SurfaceMeshTextureProjection
/////
///// \brief This filter does naive registration/correspodence finding to map and interpolate texture
///// coordinates from one SurfaceMesh to another using a simple ray casting approach
/////
//class SurfaceMeshTextureProjection : public GeometryAlgorithm
//{
//public:
//    SurfaceMeshTextureProjection();
//    virtual ~SurfaceMeshTextureProjection() override = default;
//
//public:
//    ///
//    /// \brief Required input, port 0, mesh to be sampled from
//    ///
//    void setSourceMesh(std::shared_ptr<SurfaceMesh> srcMesh);
//
//    ///
//    /// \brief Required input, port 1, mesh to sample too
//    ///
//    void setDestMesh(std::shared_ptr<SurfaceMesh> destMesh);
//
//protected:
//    void requestUpdate() override;
//};
//}