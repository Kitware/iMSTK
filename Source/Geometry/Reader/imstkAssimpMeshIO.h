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

#ifndef imstkAssimpMeshIO_h
#define imstkAssimpMeshIO_h

#include "imstkSurfaceMesh.h"
#include "imstkMeshIO.h"

#include "assimp/Importer.hpp"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

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
    ///
    static std::shared_ptr<SurfaceMesh> read(const std::string& filePath, MeshFileType type);

protected:
    ///
    /// \brief Reads mesh data and returns mesh
    /// \param filePath
    ///
    static std::shared_ptr<SurfaceMesh> readMeshData(const std::string& filePath);
};
}

#endif
