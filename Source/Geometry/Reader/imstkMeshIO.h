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

// std library
#include <memory>
#include <vector>
#include <array>

// imstk
#include "imstkPointSet.h"

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
    DAE,
    FBX,
    _3DS,
    VEG,
    MSH,
    DCM
};

///
/// \class MeshIO
///
/// \brief Mesh data IO
///
class MeshIO
{
public:

    ///
    /// \brief Constructor
    ///
    MeshIO() = default;

    ///
    /// \brief Destructor
    ///
    ~MeshIO() = default;

    ///
    /// \brief Read external file
    ///
    static std::shared_ptr<PointSet> read(const std::string& filePath);

    ///
    /// \brief Write external file
    ///
    static bool write(const std::shared_ptr<imstk::PointSet> imstkMesh, const std::string& filePath);

    ///
    /// \brief Returns true if the file exists, else false.
    /// Also sets isDirectory to true if the path is a directory, else false.
    ///
    static bool fileExists(const std::string& file, bool& isDirectory);

    ///
    /// \brief Returns the type of the file
    ///
    static const MeshFileType getFileType(const std::string& filePath);
};
} // imstk
