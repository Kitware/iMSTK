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

#ifndef imstkVegaMeshReader_h
#define imstkVegaMeshReader_h

#include <memory>

#include "imstkMeshReader.h"
#include "imstkVolumetricMesh.h"

// Vega
#include "volumetricMeshLoader.h"
#include "volumetricMesh.h"

namespace imstk
{

///
/// \class VegaMeshReader
///
/// \brief Contains utility classes that convert vega volume mesh to volume mesh and
/// vice-versa
///
class VegaMeshReader
{
public:

    VegaMeshReader() = default;
    ~VegaMeshReader() = default;

    ///
    /// \brief Read and generate volumetric mesh given a external vega mesh file
    ///
    static std::shared_ptr<VolumetricMesh> read(const std::string& filePath, MeshFileType meshType);

    ///
    /// \brief Read vega volume mesh from a file
    ///
    static std::shared_ptr<vega::VolumetricMesh> readVegaMesh(const std::string& filePath);

protected:
    ///
    /// \brief Generate volumetric mesh given a vega volume mesh
    ///
    static std::shared_ptr<VolumetricMesh> convertVegaMeshToVolumetricMesh(std::shared_ptr<vega::VolumetricMesh> vegaVolumeMesh);

    ///
    /// \brief Generate a vega volume mesh given volumetric mesh
    ///
    static std::shared_ptr<vega::VolumetricMesh> convertVolumetricMeshToVegaMesh(std::shared_ptr<VolumetricMesh> volumeMesh);

    ///
    /// \brief
    ///
    static void copyVertices(std::shared_ptr<vega::VolumetricMesh> vegaMesh, std::vector<Vec3d>& vertices);

    ///
    /// \brief
    ///
    template<size_t dim>
    static void copyCells(std::shared_ptr<vega::VolumetricMesh> vegaMesh, std::vector<std::array<size_t,dim>>& cells);
};

} // imstk

#endif // ifndef imstkVegaMeshReader_h
