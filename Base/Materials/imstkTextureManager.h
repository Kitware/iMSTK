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

#ifndef imstkTextureManager_h
#define imstkTextureManager_h

#include "imstkTexture.h"
#include "imstkTextureDelegate.h"

#include <map>
#include <string>
#include <memory>

namespace imstk
{

template<class T>
class TextureManager
{
static_assert(std::is_base_of<TextureDelegate, T>::value, "T isn't a subclass of TextureDelegate");

public:
    ///
    /// \brief Add texture
    ///
    std::shared_ptr<T> getTextureDelegate(std::shared_ptr<Texture> texture);

protected:
    friend class VTKRenderer;

    ///
    /// \brief Constructor
    ///
    TextureManager() {}

    std::map<std::shared_ptr<Texture>, std::shared_ptr<T>> m_textureMap;
};

template<class T> std::shared_ptr<T>
TextureManager<T>::getTextureDelegate(std::shared_ptr<Texture> texture)
{
    if (m_textureMap.count(texture) == 0)
    {
        auto textureDelegate = std::make_shared<T>();
        textureDelegate->loadTexture(texture);
        m_textureMap[texture] = textureDelegate;
    }
    return m_textureMap[texture];
}

}

#endif
