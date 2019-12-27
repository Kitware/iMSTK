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

#include "imstkMeshIO.h"
#include "imstkImageData.h"

namespace imstk
{
///
/// \class DCMMeshIO
///
/// \brief Reader for DICOM format image data.
///
class DCMMeshIO
{
public:
    ///
    /// \brief Default constructor
    ///
    DCMMeshIO() = default;

    ///
    /// \brief Default destructor
    ///
    ~DCMMeshIO() = default;

    ///
    /// \brief Read and generate an image data from a give file/directory
    ///
    static std::shared_ptr<imstk::ImageData> read(const std::string& filePath);
};
} // namespace imstk
