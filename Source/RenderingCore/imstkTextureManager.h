/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkTexture.h"
#include "imstkTextureDelegate.h"

#include <unordered_map>

namespace imstk
{
///
/// \class TextureManager
///
/// \brief The TextureManager provides delegates for textures, it will create new ones
/// and cache old ones
///
template<class T>
class TextureManager
{
static_assert(std::is_base_of<TextureDelegate, T>::value, "T isn't a subclass of TextureDelegate");

public:
    TextureManager() = default;

    std::shared_ptr<T> getTextureDelegate(std::shared_ptr<Texture> texture);

protected:
    std::unordered_map<std::shared_ptr<Texture>, std::shared_ptr<T>> m_textureMap;
};

template<class T> std::shared_ptr<T>
TextureManager<T>::getTextureDelegate(std::shared_ptr<Texture> texture)
{
    // If doesn't exist, create new delegate for the texture
    if (m_textureMap.count(texture) == 0)
    {
        m_textureMap[texture] = std::make_shared<T>(texture);
    }
    return m_textureMap[texture];
}
} // namespace imstk
