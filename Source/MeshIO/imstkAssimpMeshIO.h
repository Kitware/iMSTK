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

#include "imstkSurfaceMesh.h"
#include "imstkMeshIO.h"

struct aiMesh;

namespace imstk
{
///
/// \class AssimpMeshIO
///
/// \brief Assimp reader for surface meshes
///
class AssimpMeshIO
{
public:
    ///
    /// \brief Ensures file can be read and reads it if possible
    /// \param filePath path to file
    /// \param type mesh file type
    /// \returns iMSTK surface mesh
    ///
    static std::shared_ptr<SurfaceMesh> read(
        const std::string& filePath,
        MeshFileType       type);

    ///
    /// \brief Reads mesh data and returns mesh
    /// \param filePath
    /// \returns iMSTK surface mesh
    ///
    static std::shared_ptr<SurfaceMesh> readMeshData(const std::string& filePath);

    ///
    /// \brief Convert from Assimp mesh to iMSTK surface mesh
    /// \param importedMesh Assimp mesh to covert
    /// \returns iMSTK surface mesh
    ///
    static std::shared_ptr<SurfaceMesh> convertAssimpMesh(aiMesh* importedMesh);

    ///
    /// \brief Helper function for getting default post processing flags
    /// \returns Bit mask of default flags
    /// This function should be used to ensure consistency among different loaders
    ///
    static unsigned int getDefaultPostProcessSteps();
};
}
