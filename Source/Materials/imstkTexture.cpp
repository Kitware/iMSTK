/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkTexture.h"

namespace imstk
{
Texture::Texture(std::string path, Type type) :
    m_type(type), m_path(path)
{
}

Texture::Texture(std::shared_ptr<ImageData> imageTex, Type type) :
    imageTexture(imageTex), m_type(type), m_path("")
{
}

Texture::Type
Texture::getType() const
{
    return m_type;
}

Texture::FileType
Texture::getFileType()
{
    FileType textureType = FileType::Unknown;

    std::string extString = m_path.substr(m_path.find_last_of(".") + 1);

    if (extString.empty() || m_path.find(".") == std::string::npos)
    {
        return textureType;
    }

    if (extString == "bmp" || extString == "BMP")
    {
        textureType = FileType::Bmp;
    }
    else if (extString == "png" || extString == "PNG")
    {
        textureType = FileType::Png;
    }
    else if (extString == "jpg" || extString == "JPG"
             || extString == "jpeg" || extString == "JPEG")
    {
        textureType = FileType::Jpg;
    }
    else if (extString == "dds" || extString == "DDS")
    {
        textureType = FileType::Dds;
    }

    return textureType;
}
} // namespace imstk