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

#ifndef imstkTextureDelegate_h
#define imstkTextureDelegate_h

#include <string>
#include <memory>

#include "imstkTexture.h"

namespace imstk
{

///
/// \class TextureDelegate
///
/// \brief iMSTK texture delegate abstract class
///
class TextureDelegate
{
protected:
    ///
    /// \brief Constructor
    /// \param texture The texture
    ///
    TextureDelegate() {}

    ///
    /// \brief Abstract function to load textures
    /// \param texture Texture to load
    ///
    virtual void loadTexture(std::shared_ptr<Texture> texture) = 0;
};

}

#endif
