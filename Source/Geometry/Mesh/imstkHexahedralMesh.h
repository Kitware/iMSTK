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

#include "imstkMacros.h"
#include "imstkVolumetricMesh.h"

namespace imstk
{
///
/// \class HexahedralMesh
///
/// \brief Hexahedral mesh class
///
class HexahedralMesh : public VolumetricMesh<8>
{
public:
    HexahedralMesh() = default;
    ~HexahedralMesh() override = default;

    IMSTK_TYPE_NAME(HexahedralMesh)

    ///
    /// \brief Clear all the mesh data
    ///
    void clear() override;

    ///
    /// \brief Print the hexahedral mesh
    ///
    void print() const override;

    ///
    /// \brief Extract surface Mesh
    std::shared_ptr<SurfaceMesh> extractSurfaceMesh() override;

// Accessors
    ///
    /// \brief Returns the number of hexahedra
    ///
    int getNumHexahedra() const { return getNumCells(); }

    ///
    /// \brief Compute and return the volume of the hexahedral mesh
    ///
    double getVolume() override;
};
} // namespace imstk