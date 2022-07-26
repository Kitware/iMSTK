/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVTKTextureDelegate.h"
#include "imstkColor.h"
#include "imstkGeometryUtilities.h"
#include "imstkLogger.h"
#include "imstkTexture.h"

#include <vtkImageData.h>
#include <vtkImageFlip.h>
#include <vtkImageReader2Factory.h>
#include <vtkPNGReader.h>
#include <vtkTexture.h>
#include <vtksys/SystemTools.hxx>

namespace imstk
{
VTKTextureDelegate::VTKTextureDelegate(std::shared_ptr<Texture> texture) : m_vtkTexture(vtkSmartPointer<vtkTexture>::New())
{
    m_texture = texture;
    const std::string tFileName = texture->getPath();

    // If the texture is provided by ImageData use that, otherwise load via path
    std::shared_ptr<ImageData> imstkImgData = texture->getImageData();
    if (imstkImgData == nullptr)
    {
        CHECK(vtksys::SystemTools::FileExists(tFileName.c_str()))
            << "Error: texture file \"" << tFileName << "\" does not exist";

        // Load by file name
        vtkNew<vtkImageReader2Factory> readerFactory;
        if (texture->getType() == Texture::Type::Cubemap)
        {
            std::string sideNames[6] = { "posx", "negx", "posy", "negy", "posz", "negz" };
            m_vtkTexture->SetCubeMap(true);

            for (int i = 0; i < 6; i++)
            {
                auto index     = tFileName.find(".");
                auto tempName  = tFileName.substr(0, index);
                auto extension = tFileName.substr(index);
                auto sideName  = tempName + sideNames[i] + extension;

                vtkImageReader2* imgReader = readerFactory->CreateImageReader2(sideName.c_str());

                CHECK(imgReader != nullptr) << "Error: could not find reader for "
                                            << sideName;

                auto imageFlip = vtkSmartPointer<vtkImageFlip>::New();
                imageFlip->SetFilteredAxis(1);
                imgReader->SetFileName(sideName.c_str());
                imgReader->Update();
                imageFlip->SetInputConnection(imgReader->GetOutputPort());
                m_vtkTexture->SetInputConnection(i, imageFlip->GetOutputPort());
            }
        }
        else
        {
            vtkImageReader2* imgReader = readerFactory->CreateImageReader2(tFileName.c_str());

            CHECK(imgReader != nullptr) << "Error: could not find reader for "
                                        << tFileName;

            imgReader->SetFileName(tFileName.c_str());
            imgReader->Update();
            m_vtkTexture->SetBlendingMode(vtkTexture::VTK_TEXTURE_BLENDING_MODE_ADD);
            const Texture::WrapType wrapType = m_texture->getWrapType();
            if (wrapType == Texture::WrapType::REPEAT)
            {
                m_vtkTexture->SetWrap(vtkTexture::Repeat);
            }
            else if (wrapType == Texture::WrapType::CLAMP_TO_BORDER)
            {
                m_vtkTexture->SetWrap(vtkTexture::ClampToBorder);
            }
            else if (wrapType == Texture::WrapType::CLAMP_TO_EDGE)
            {
                m_vtkTexture->SetWrap(vtkTexture::ClampToEdge);
            }
            const Color& borderColor = m_texture->getBorderColor();
            m_vtkTexture->SetBorderColor(
                static_cast<float>(borderColor.r),
                static_cast<float>(borderColor.g),
                static_cast<float>(borderColor.b),
                static_cast<float>(borderColor.a));
            m_vtkTexture->SetInterpolate(m_texture->getInterpolation());
            m_vtkTexture->SetInputConnection(0, imgReader->GetOutputPort());

            if (texture->getType() == Texture::Type::Diffuse)
            {
                m_vtkTexture->SetUseSRGBColorSpace(true);
            }
        }
    }
    else
    {
        // Load by ImageData
        vtkSmartPointer<vtkImageData> vtkImgData = GeometryUtils::coupleVtkImageData(imstkImgData);
        m_vtkTexture->SetBlendingMode(vtkTexture::VTK_TEXTURE_BLENDING_MODE_ADD);
        m_vtkTexture->SetInterpolate(m_texture->getInterpolation());

        const Texture::WrapType wrapType = m_texture->getWrapType();
        if (wrapType == Texture::WrapType::REPEAT)
        {
            m_vtkTexture->SetWrap(vtkTexture::Repeat);
        }
        else if (wrapType == Texture::WrapType::CLAMP_TO_BORDER)
        {
            m_vtkTexture->SetWrap(vtkTexture::ClampToBorder);
        }
        else if (wrapType == Texture::WrapType::CLAMP_TO_EDGE)
        {
            m_vtkTexture->SetWrap(vtkTexture::ClampToEdge);
        }
        const Color& borderColor = m_texture->getBorderColor();
        m_vtkTexture->SetBorderColor(
            static_cast<float>(borderColor.r),
            static_cast<float>(borderColor.g),
            static_cast<float>(borderColor.b),
            static_cast<float>(borderColor.a));
        m_vtkTexture->SetInputData(vtkImgData);

        if (texture->getType() == Texture::Type::Diffuse)
        {
            m_vtkTexture->SetUseSRGBColorSpace(true);
        }
    }

    // Observe changes to the texture
    connect<Event>(m_texture, &Texture::modified, this, &VTKTextureDelegate::textureModified);
}

void
VTKTextureDelegate::textureModified(Event* imstkNotUsed(e))
{
    //m_vtkTexture->GetInputDataObject();
    m_vtkTexture->Modified();
}
} // namespace imstk
