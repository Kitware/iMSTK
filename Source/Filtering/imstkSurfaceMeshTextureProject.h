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

#include "imstkGeometryAlgorithm.h"

namespace imstk
{
class SurfaceMesh;

///
/// \class SurfaceMeshTextureProject
///
/// \brief This filter projects vertex texture attributes from one SurfaceMesh to
/// another by finding the closest point on the source mesh and interpolating the
/// results. This could later be expanded to arbitrary vertex attributes
///
class SurfaceMeshTextureProject : public GeometryAlgorithm
{
public:
    SurfaceMeshTextureProject();
    virtual ~SurfaceMeshTextureProject() override = default;

public:
    ///
    /// \brief The mesh with attribute to put on the other
    ///
    void setSourceMesh(std::shared_ptr<SurfaceMesh> surfMesh);

    ///
    /// \brief The mesh to recieve the attribute
    ///
    void setDestMesh(std::shared_ptr<SurfaceMesh> destMesh);

    ///
    /// \brief destMesh copy with the attribute
    ///
    std::shared_ptr<SurfaceMesh> getOutputMesh();

protected:
    void requestUpdate() override;
};
}