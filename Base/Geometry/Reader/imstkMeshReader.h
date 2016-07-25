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

#ifndef imstkMeshReader_h
#define imstkMeshReader_h

// std library
#include <memory>
#include <vector>
#include <array>

// imstk
#include "imstkMesh.h"

namespace imstk
{

///
/// \brief Enumeration the mesh file type
///
enum MeshFileType
{
    UNKNOWN,
    VTK,
    VTU,
    VTP,
    STL,
    PLY,
    OBJ,
    VEG
};

///
/// \class MeshReader
///
/// \brief Mesh data reader
///
class MeshReader
{
public:

    ///
    /// \brief Constructor
    ///
    MeshReader() = default;

    ///
    /// \brief Destructor
    ///
    ~MeshReader() = default;

    ///
    /// \brief Read external file
    ///
    static std::shared_ptr<Mesh> read(const std::string& filePath);

    ///
    /// \brief Returns true if the file exists, else false
    ///
    static bool fileExists(const std::string& file);

protected:
    ///
    /// \brief Returns the type of the file
    ///
    static const MeshFileType getFileType(const std::string& filePath);

};

} // imstk

#endif // ifndef imstkMeshReader_h
