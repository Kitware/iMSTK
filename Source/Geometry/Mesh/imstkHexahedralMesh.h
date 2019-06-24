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

#include <set>

#include "imstkVolumetricMesh.h"

namespace imstk
{
///
/// \class HexahedralMesh
///
/// \brief Hexahedral mesh class
///
class HexahedralMesh : public VolumetricMesh
{
public:

    using HexaArray = std::array<size_t, 8>;

    ///
    /// \brief Constructor
    ///
    HexahedralMesh() : VolumetricMesh(Geometry::Type::HexahedralMesh) {}

    ///
    /// \brief Destructor
    ///
    ~HexahedralMesh() = default;

    ///
    /// \brief Initializes the rest of the data structures given vertex positions and
    ///  hexahedra connectivity
    ///
    void initialize(const StdVectorOfVec3d& vertices,
                    const std::vector<HexaArray>& hexahedra,
                    bool computeAttachedSurfaceMesh = false);

    ///
    /// \brief Clear all the mesh data
    ///
    void clear() override;

    ///
    /// \brief Print the hexahedral mesh
    ///
    void print() const override;

    ///
    /// \brief Compute and return the volume of the hexahedral mesh
    ///
    double getVolume() const override;

    ///
    /// \brief Compute and set the attached surface mesh
    ///
    void computeAttachedSurfaceMesh() override;

    ///
    /// \brief Extract surface Mesh
    bool extractSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh);

    // Accessors

    ///
    /// \brief Sets/Returns the hexahedral connectivity
    ///
    void setHexahedraVertices(const std::vector<HexaArray>& hexahedra);
    const std::vector<HexaArray>& getHexahedraVertices() const;

    ///
    /// \brief Returns the connectivity of a hexahedron given its index
    ///
    const HexaArray& getHexahedronVertices(const int& hexaNum) const;

    ///
    /// \brief Returns the number of hexahedra
    ///
    size_t getNumHexahedra() const;

protected:

    friend class VTKHexahedralMeshRenderDelegate;

    std::vector<HexaArray> m_hexahedraVertices; ///< vertices of the hexahedra
};
} // imstk

