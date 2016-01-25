// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CORE_TEXTURE_H
#define CORE_TEXTURE_H

// VTK glew library
#include <vtk_glew.h>

#ifndef __APPLE__
#  include <GL/gl.h> // for GLfloat, etc.
#else
#  include <OpenGL/gl.h> // for GLfloat, etc.
#endif

// STL includes
#include <unordered_map>
#include <cstring>

// iMSTK includes
#include "Core/Config.h"
#include "Core/ErrorLog.h"

namespace imstk {

enum TextureReturnType
{
    IMSTK_TEXTURE_NOTFOUND,
    IMSTK_TEXTURE_IMAGELOADINGERROR,
    IMSTK_TEXTURE_DRIVERNOTINITIALIZED,
    IMSTK_TEXTURE_OK
};

enum ImageColorType
{
    IMSTK_IMAGECOLOR_RGB,
    IMSTK_IMAGECOLOR_RGBA,
    IMSTK_IMAGECOLOR_OFFSCREENRGBA,
    IMSTK_IMAGECOLOR_DEPTH
};
/// \brief texture structure
struct Texture
{
    /// \brief texture file name
    std::string textureFileName;
    /// \brief GL id
    GLuint textureGLId;
    /// \brief stores the width of the texture
    int width;
    /// \brief stores the height of the texture
    int height;
    /// \brief image color type
    ImageColorType imageColorType;
    /// \brief SFML image
    sf::Image image;
    /// \brief GL id returned by opengl library
    GLuint GLtype;
    /// \brief true when initilized
    bool isInitialized;
    /// \brief to make a copy of the texture.
    inline Texture *copy()
    {
        Texture *myCopy = new Texture();
        myCopy->textureFileName = this->textureFileName;
        myCopy->textureGLId = this->textureGLId;
        myCopy->width = this->width;
        myCopy->height = this->height;
        myCopy->imageColorType = this->imageColorType;
        myCopy->image.create(this->width, this->height, this->image.getPixelsPtr());
        myCopy->isInitialized = this->isInitialized;

        return myCopy;
    }
};

}

#endif // CORE_TEXTURE_H
