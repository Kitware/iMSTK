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

#include "imstkMeshIO.h"

namespace imstk
{
class VolumetricMesh;
class SurfaceMesh;

///
/// \class MSHMeshIO
///
/// \brief Contains utility to generate imstk::VolumetricMesh from mesh given in msh
/// file format. Only works for tet meshes.
///
class MSHMeshIO
{
public:
    ///
    /// \brief Defualt Constructor
    ///
    MSHMeshIO() = default;

    ///
    /// \brief Default Destructor
    ///
    ~MSHMeshIO() = default;

    ///
    /// \brief Read and generate a volumetric mesh given a external msh file
    ///
    static std::shared_ptr<VolumetricMesh> read(const std::string& filePath, const MeshFileType meshType);

protected:
    ///
    /// \brief Possible element types in a MSH file format
    /// as mentioned at http://www.manpagez.com/info/gmsh/gmsh-2.2.6/gmsh_63.php
    ///
    enum ElemType
    {
        line=1,
        triangle,
        quadrangle,
        tetrahedron,
        hexahedron,
        prism,
        pyramid,
        lineSecondOrder,
        triangleSecondOrder,
        quadrangleSecondOrderType1,
        tetrahedronSecondOrder,
        hexahedronSecondOrderType1,
        prismSecondOrderType1,
        pyramidSecondOrderType1,
        point,
        quadrangleSecondOrderType2,
        hexahedronSecondOrderType2,
        prismSecondOrderType2,
        pyramidSecondOrderType2,
        triangleThirdOrderIncomplete,
        triangleThirdOrder,
        triangleFourthOrderIncomplete,
        triangleFourthOrder,
        triangleFifthOrderIncomplete,
        triangleFifthOrder,
        edgeThirdOrder,
        edgeFourthOrder,
        edgeFifthOrder,
        tetrahedronThirdOrder,
        tetrahedronFourthOrder,
        tetrahedronFifthOrder,
    };

    //
    // \brief Returns the number of nodes in an element of a given type
    //
    static size_t numElemNodes(const ElemType& elType);
};
}
