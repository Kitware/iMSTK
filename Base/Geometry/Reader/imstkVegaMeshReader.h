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
    /// \brief Generate volumetric mesh given a external vega mesh file
    ///
    static std::shared_ptr<VolumetricMesh> getVolumeMeshFromVegaVolumeMesh(const std::string& filePath, MeshFileType meshType);

    ///
    /// \brief Generate volumetric mesh given a vega mesh object
    ///
    static std::shared_ptr<VolumetricMesh> getVolumeMeshFromVegaVolumeMesh(std::shared_ptr<vega::VolumetricMesh> vegaVolumeMesh);

    ///
    /// \brief Generate a vega volume mesh given volumetric mesh
    ///
    static std::shared_ptr<vega::VolumetricMesh> getVegaVolumeMeshFromVolumeMesh(std::shared_ptr<VolumetricMesh> volumeMesh);

    ///
    /// \brief Read and generate volumetric mesh given a external vega mesh file
    ///
    static std::shared_ptr<Mesh> read(const std::string& filePath, MeshFileType meshType) { return getVolumeMeshFromVegaVolumeMesh(filePath, meshType); };
};

} // imstk

#endif // ifndef imstkVegaMeshReader_h
