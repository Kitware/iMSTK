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

#include "imstkVTKTextureDelegate.h"
#include "g3log/g3log.hpp"

namespace imstk
{

vtkSmartPointer<vtkTexture>
VTKTextureDelegate::getTexture() const
{
    return m_sourceTexture;
}

void
VTKTextureDelegate::loadTexture(std::shared_ptr<Texture> texture)
{
    std::string tFileName = texture->getPath();

    auto readerFactory = vtkSmartPointer<vtkImageReader2Factory>::New();

    // Read texture image
    auto imgReader = readerFactory->CreateImageReader2(texture->getPath().c_str());
    if (!imgReader)
    {
        LOG(WARNING) << "VTKTextureDelegate::loadTexture error: could not find reader for "
                     << tFileName;
        return;
    }

    imgReader->SetFileName(tFileName.c_str());
    imgReader->Update();
    m_sourceTexture = vtkSmartPointer<vtkTexture>::New();
    m_sourceTexture->SetInputConnection(imgReader->GetOutputPort());
    m_sourceTexture->SetBlendingMode(vtkTexture::VTK_TEXTURE_BLENDING_MODE_ADD);
}

}
