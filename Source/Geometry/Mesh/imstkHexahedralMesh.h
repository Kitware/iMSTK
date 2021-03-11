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
    ///
    /// \brief Constructor
    ///
    HexahedralMesh(const std::string& name = std::string(""));

    ///
    /// \brief Deconstructor
    ///
    virtual ~HexahedralMesh() override = default;

    ///
    /// \brief Returns the string representing the type name of the geometry
    ///
    virtual const std::string getTypeName() const override { return "HexahedralMesh"; }

public:
    ///
    /// \brief Initializes the rest of the data structures given vertex positions and
    ///  hexahedra connectivity
    ///
    void initialize(std::shared_ptr<VecDataArray<double, 3>> vertices,
                    std::shared_ptr<VecDataArray<int, 8>> hexahedra,
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
    /// \brief Compute and set the attached surface mesh
    ///
    void computeAttachedSurfaceMesh() override;

    ///
    /// \brief Extract surface Mesh
    bool extractSurfaceMesh(std::shared_ptr<SurfaceMesh> surfaceMesh);

    ///
    /// \brief Returns true if the geometry is a mesh, else returns false
    ///
    bool isMesh() const override { return true; }

// Accessors
public:
    ///
    /// \brief Sets/Returns the hexahedral connectivity
    ///
    void setHexahedraIndices(std::shared_ptr<VecDataArray<int, 8>> hexahedra) { m_hexahedraIndices = hexahedra; }
    std::shared_ptr<VecDataArray<int, 8>> getHexahedraIndices() const { return m_hexahedraIndices; }

    ///
    /// \brief Returns the connectivity of a hexahedron given its index
    ///
    const Vec8i& getHexahedronIndices(const int hexaNum) const;

    ///
    /// \brief Returns the number of hexahedra
    ///
    size_t getNumHexahedra() const;

    ///
    /// \brief Compute and return the volume of the hexahedral mesh
    ///
    double getVolume() override;

protected:
    std::shared_ptr<VecDataArray<int, 8>> m_hexahedraIndices;   ///< indices of the hexahedra
};
} // imstk
