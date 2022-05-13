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
/// \class MshMeshIO
///
/// \brief Can read/return LineMesh, SurfaceMesh, TetrahedralMesh, or
/// HexahedralMesh from given .msh file. Can only read homogenous elements.
///
/// If given a file with mixed elements it will always choose the elements
/// with the most vertices. Ex: Given both tetrahedral and triangle data,
/// only the tetrahedral will be read.
///
/// Only supports vertex data that is doubles (8 byte sized floating point).
///
class MshMeshIO
{
public:
    MshMeshIO() = default;
    virtual ~MshMeshIO() = default;

    ///
    /// \brief Read and generate a volumetric mesh given a external msh file
    ///
    static std::shared_ptr<PointSet> read(const std::string& filePath);
};
} // namespace imstk
