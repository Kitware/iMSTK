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

#ifndef imstkMSHMeshReader_h
#define imstkMSHMeshReader_h

#include <memory>
#include <vector>
#include <array>

#include "imstkMeshReader.h"
#include "imstkVolumetricMesh.h"

namespace imstk
{

///
/// \class MSHMeshReader
///
/// \brief Contains utility to generate imstk::VolumetricMesh from mesh given in msh
/// file format. Only works for tet meshes.
///
class MSHMeshReader
{
public:
    ///
    /// \brief Defualt Constructor
    ///
    MSHMeshReader() = default;

    ///
    /// \brief Default Destructor
    ///
    ~MSHMeshReader() = default;

    ///
    /// \brief Read and generate a volumetric mesh given a external msh file
    ///
    static std::shared_ptr<imstk::VolumetricMesh> read(const std::string &filePath,const MeshFileType meshType);

};

}

#endif// imstkMSHMeshReader_h