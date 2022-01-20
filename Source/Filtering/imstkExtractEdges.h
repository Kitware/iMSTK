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
class LineMesh;
class SurfaceMesh;

///
/// \class ExtractEdges
///
/// \brief This filter extracts the edges of a SurfaceMesh producing a LineMesh
///
class ExtractEdges : public GeometryAlgorithm
{
public:
    ExtractEdges();
    ~ExtractEdges() override = default;

    std::shared_ptr<LineMesh> getOutputMesh() const;

    void setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh);

protected:
    void requestUpdate() override;
};
}