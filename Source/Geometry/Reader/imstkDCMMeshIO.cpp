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

#include <fstream>

#include "imstkDCMMeshIO.h"

#include <vtkSmartPointer.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageData.h>

namespace imstk
{
std::shared_ptr<ImageData>
DCMMeshIO::read(const std::string& filePath)
{
    bool isDirectory;
    if (!MeshIO::fileExists(filePath, isDirectory))
    {
        LOG(FATAL) << "DCMMeshIO::read error: file not found: " << filePath;
        return nullptr;
    }

    vtkNew<vtkDICOMImageReader> reader;
    if (isDirectory)
    {
        reader->SetDirectoryName(filePath.c_str());
    }
    else
    {
        reader->SetFileName(filePath.c_str());
    }
    reader->Update();

    auto imageData = std::make_shared<ImageData>();
    imageData->initialize(reader->GetOutput());
    return imageData;
}
} // namespace imstk
