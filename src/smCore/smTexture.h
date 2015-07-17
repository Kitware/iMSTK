// This file is part of the SimMedTK project.
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

#ifndef SMTEXTURE_H
#define SMTEXTURE_H

// STL includes
#include <unordered_map>
#include <cstring>

// SimMedTK includes
#include "smConfig.h"
#include "smErrorLog.h"

enum smTextureReturnType
{
    SIMMEDTK_TEXTURE_NOTFOUND,
    SIMMEDTK_TEXTURE_IMAGELOADINGERROR,
    SIMMEDTK_TEXTURE_DRIVERNOTINITIALIZED,
    SIMMEDTK_TEXTURE_OK
};

enum ImageColorType
{
    SIMMEDTK_IMAGECOLOR_RGB,
    SIMMEDTK_IMAGECOLOR_RGBA,
    SIMMEDTK_IMAGECOLOR_OFFSCREENRGBA,
    SIMMEDTK_IMAGECOLOR_DEPTH
};
/// \brief texture structure
struct smTexture
{
    /// \brief texture file name
    smString textureFileName;
    /// \brief GL id
    GLuint textureGLId;
    /// \brief stores the width of the texture
    smInt width;
    /// \brief stores the height of the texture
    smInt height;
    /// \brief image color type
    ImageColorType imageColorType;
    /// \brief SFML image
    sf::Image image;
    /// \brief GL id returned by opengl library
    smGLUInt GLtype;
    /// \brief true when initilized
    smBool isInitialized;
    /// \brief to make a copy of the texture.
    inline smTexture *copy()
    {
        smTexture *myCopy = new smTexture();
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

/// \brief image data
struct smImageData
{
    void *data;
    smInt width;
    smInt height;
    smInt bytePerPixel;
    ImageColorType imageColorType;
    smString fileName;
};

#endif // SMTEXTURE_H
