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

const std::string&
VTKTextureDelegate::getTextureName() const
{
    return m_textureName;
}

void
VTKTextureDelegate::loadTexture(std::shared_ptr<Texture> texture)
{
    std::string tFileName = texture->getPath();

    auto readerFactory = vtkSmartPointer<vtkImageReader2Factory>::New();

    m_sourceTexture = vtkSmartPointer<vtkTexture>::New();

    // Mangle an unique texture name from the texture type and texture path
    std::string manglingSymbol = "::";
    m_textureName = texture->getTypeAsString() + manglingSymbol + texture->getPath();

    if (texture->getType() == Texture::Type::Cubemap)
    {
        std::string sideNames[6] = { "posx", "negx", "posy", "negy", "posz", "negz" };
        m_sourceTexture->SetCubeMap(true);

        for (int i = 0; i < 6; i++)
        {
            vtkImageReader2* imgReader;

            auto index     = tFileName.find(".");
            auto tempName  = tFileName.substr(0, index);
            auto extension = tFileName.substr(index);
            auto sideName  = tempName + sideNames[i] + extension;

            imgReader = readerFactory->CreateImageReader2(sideName.c_str());

            CHECK(imgReader != nullptr) << "VTKTextureDelegate::loadTexture error: could not find reader for "
                                        << sideName;

            auto imageFlip = vtkSmartPointer<vtkImageFlip>::New();
            imageFlip->SetFilteredAxis(1);
            imgReader->SetFileName(sideName.c_str());
            imgReader->Update();
            imageFlip->SetInputConnection(imgReader->GetOutputPort());
            m_sourceTexture->SetInputConnection(i, imageFlip->GetOutputPort());
        }
    }
    else
    {
        vtkImageReader2* imgReader;
        imgReader = readerFactory->CreateImageReader2(tFileName.c_str());

        CHECK(imgReader != nullptr) << "VTKTextureDelegate::loadTexture error: could not find reader for "
                                    << tFileName;

        imgReader->SetFileName(tFileName.c_str());
        imgReader->Update();
        m_sourceTexture->SetBlendingMode(vtkTexture::VTK_TEXTURE_BLENDING_MODE_ADD);
        m_sourceTexture->RepeatOff();
        m_sourceTexture->SetInputConnection(imgReader->GetOutputPort());
    }
}
}
