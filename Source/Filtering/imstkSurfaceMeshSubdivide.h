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
/// \class SubdivideSurfaceMesh
///
/// \brief This filter subidivdes the triangles of a SurfaceMesh into more
/// smaller triangles
/// \todo: test with LineMesh
///
class SurfaceMeshSubdivide : public GeometryAlgorithm
{
public:
    enum class Type
    {
        LINEAR,
        LOOP,
        BUTTERFLY
    };

public:
    SurfaceMeshSubdivide();
    ~SurfaceMeshSubdivide() override = default;

public:
    std::shared_ptr<SurfaceMesh> getOutputMesh();

    void setInputMesh(std::shared_ptr<SurfaceMesh> inputMesh);

    imstkGetMacro(SubdivisionType, Type);
    imstkGetMacro(NumberOfSubdivisions, int);
    imstkSetMacro(SubdivisionType, Type);
    imstkSetMacro(NumberOfSubdivisions, int);

protected:
    void requestUpdate() override;

private:
    Type m_SubdivisionType      = Type::LINEAR;
    int  m_NumberOfSubdivisions = 1;
};
}