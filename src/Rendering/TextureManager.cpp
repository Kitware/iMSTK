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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "TextureManager.h"
#include "GLUtils.h"

#include <cassert>

std::shared_ptr<ErrorLog> TextureManager:: errorLog;
std::vector<Texture*>  TextureManager:: textures;
std::unordered_map<std::string, int>  TextureManager::textureIndexId;
int TextureManager:: activeTextures;
bool TextureManager::isInitialized = false;
bool TextureManager::isInitializedGL = false;
bool TextureManager::isDeleteImagesEnabled = false;

/// \brief
smTextureReturnType TextureManager::initGLTextures()
{
    std::string texManagerError;
    Texture *texture;

    for (size_t i = 0; i < textures.size(); i++)
    {
        texture = textures[i];

        glEnable(GL_TEXTURE_2D);

        if (texture->imageColorType == SIMMEDTK_IMAGECOLOR_DEPTH)
        {
            initDepthTexture(texture);

            continue;
        }

        if (texture->imageColorType == SIMMEDTK_IMAGECOLOR_OFFSCREENRGBA)
        {
            initColorTexture(texture);

            continue;
        }

        glGenTextures(1, &texture->textureGLId);

        glBindTexture(GL_TEXTURE_2D,  texture->textureGLId);
        glEnable(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, texture->image.getPixelsPtr());

        glGenerateMipmap(GL_TEXTURE_2D);

        if (GLUtils::queryGLError(texManagerError))
        {
            if(nullptr != errorLog)
            {
                errorLog->addError(texManagerError);
            }
        }

    }

    isInitializedGL = true;

    return  SIMMEDTK_TEXTURE_OK;
}

/// \brief load the texture and associated it with reference name.
/// Also you could use texture Id for activation of the texture
smTextureReturnType TextureManager::loadTexture(const std::string& p_fileName,
        const std::string& p_textureReferenceName, int &p_textureId)
{
    smTextureReturnType ret = loadTexture(p_fileName, p_textureReferenceName, true);
    if (ret == SIMMEDTK_TEXTURE_OK)
    {
        p_textureId = textureIndexId[p_textureReferenceName];
    }
    return ret;
}

/// \brief
smTextureReturnType TextureManager::loadTexture(const std::string& p_fileName, const std::string& p_textureReferenceName, bool p_flipImage)
{
    Texture *texture = nullptr;
    sf::Vector2u imageSize;

    assert(p_fileName != "");
    assert(p_textureReferenceName != "");

    if (!isInitialized)
    {
        return SIMMEDTK_TEXTURE_DRIVERNOTINITIALIZED;
    }

    texture = new Texture();
    assert(texture);
    if (false == texture->image.loadFromFile(p_fileName))
    {
        std::cout << "[TextureManager::loadTexture] Texture not found: \"" << p_fileName << "\"\n";
        return SIMMEDTK_TEXTURE_NOTFOUND;
    }

    if (p_flipImage)
    {
        texture->image.flipVertically();
    }

    texture->textureFileName = p_fileName;
    imageSize = texture->image.getSize();
    texture->width = imageSize.x;
    texture->height = imageSize.y;

    textures.push_back(texture);
    textureIndexId[p_textureReferenceName] = activeTextures;
    activeTextures++;
    return SIMMEDTK_TEXTURE_OK;
}


/// \brief if the texture is not loaded previously, create and the texture return the id
smTextureReturnType TextureManager::findTextureId(const std::string& p_textureReferenceName,
        int &p_textureId)
{
    if (textureIndexId.count(p_textureReferenceName) > 0)
    {
        p_textureId = textureIndexId[p_textureReferenceName];
        return   SIMMEDTK_TEXTURE_OK;
    }
    else
    {
        p_textureId = 0;
        return SIMMEDTK_TEXTURE_NOTFOUND;
    }

}

/// \brief  activate the texture with given texture reference name
GLuint TextureManager::activateTexture(const std::string& p_textureReferenceName)
{

    int textureId;
    Texture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    texture = textures[textureId];

    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glEnable(GL_TEXTURE_2D);
    return texture->textureGLId;
}

/// \brief activate the texture given the Texture argument
GLuint TextureManager::activateTexture(Texture *p_texture)
{

    glBindTexture(GL_TEXTURE_2D, p_texture->textureGLId);
    glEnable(GL_TEXTURE_2D);
    return p_texture->textureGLId;
}

/// \brief This function binds the texture to the appropriate texture.
///For instance if the argument is 0, the it will bind to GL_TEXTURE0
GLuint TextureManager::activateTexture(const std::string& p_textureReferenceName, int p_textureGLOrder)
{

    int textureId;
    Texture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    texture = textures[textureId];
    glActiveTexture(GL_TEXTURE0 + p_textureGLOrder);
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glEnable(GL_TEXTURE_2D);
    return texture->textureGLId;
}

/// \brief This function binds the texture to the appropriate shader  texture.
/// first parameter is texture reference
/// For instance if the argument is 0, the it will bind to GL_TEXTURE0
/// Also for the shader the binded name will be p_shaderBindName
GLuint TextureManager::activateTexture(const std::string& p_textureReferenceName,
        int p_textureGLOrder, int p_shaderBindGLId)
{

    int textureId;
    Texture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    texture = textures[textureId];
    glActiveTexture(GL_TEXTURE0 + p_textureGLOrder);
    glUniform1iARB(p_shaderBindGLId, p_textureGLOrder);
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glEnable(GL_TEXTURE_2D);
    return texture->textureGLId;
}

/// \brief This function binds the Texture to the appropriate shader texture.
/// For instance if the argument is 0, the it will bind to GL_TEXTURE0
/// Also for the shader the binded name will be p_shaderBindName
GLuint TextureManager::activateTexture(Texture *p_texture,
        int p_textureGLOrder, int p_shaderBindGLId)
{

    glActiveTexture(GL_TEXTURE0 + p_textureGLOrder);
    glUniform1iARB(p_shaderBindGLId, p_textureGLOrder);
    glBindTexture(GL_TEXTURE_2D, p_texture->textureGLId);
    glEnable(GL_TEXTURE_2D);
    return p_texture->textureGLId;
}

/// \brief
GLuint TextureManager::activateTexture(int p_textureId)
{

    Texture *texture;
    texture = textures[p_textureId];
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glEnable(GL_TEXTURE_2D);
    return texture->textureGLId;

}

/// \brief
GLuint TextureManager::activateTexture(int p_textureId, int p_textureGLOrder)
{

    Texture *texture;
    texture = textures[p_textureId];
    glActiveTexture(GL_TEXTURE0 + p_textureGLOrder);
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glEnable(GL_TEXTURE_2D);
    return texture->textureGLId;
}

/// \brief
void TextureManager::activateTextureGL(GLuint  p_textureId, int p_textureGLOrder)
{

    glActiveTexture(GL_TEXTURE0 + p_textureGLOrder);
    glBindTexture(GL_TEXTURE_2D, p_textureId);
    glEnable(GL_TEXTURE_2D);
}

/// \brief
GLuint TextureManager::disableTexture(const std::string& p_textureReferenceName)
{

    GLuint textureId;
    Texture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    texture = textures[textureId];
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glDisable(GL_TEXTURE_2D);
    return texture->textureGLId;
}

/// \brief
GLuint TextureManager::disableTexture(const std::string& p_textureReferenceName, int p_textureGLOrder)
{

    GLuint textureId;
    Texture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    texture = textures[textureId];

    glActiveTexture(GL_TEXTURE0 + p_textureGLOrder);
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glDisable(GL_TEXTURE_2D);
    return texture->textureGLId;
}

/// \brief
GLuint TextureManager::disableTexture(int p_textureId)
{

    Texture *texture;
    texture = textures[p_textureId];
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glDisable(GL_TEXTURE_2D);
    return texture->textureGLId;
}

/// \brief
GLuint TextureManager::getOpenglTextureId(const std::string& p_textureReferenceName)
{

    int textureId;
    Texture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    texture = textures[textureId];
    return texture->textureGLId;

}

/// \brief
GLuint TextureManager::getOpenglTextureId(int p_textureId)
{

    Texture *texture;
    texture = textures[p_textureId];
    return texture->textureGLId;

}

/// \brief
void TextureManager::createDepthTexture(const std::string& p_textureReferenceName, int p_width, int p_height)
{

    Texture *tex;
    tex = new Texture();
    tex->height = p_height;
    tex->width = p_width;
    tex->GLtype = GL_TEXTURE_2D;
    tex->textureFileName = p_textureReferenceName;
    tex->imageColorType = SIMMEDTK_IMAGECOLOR_DEPTH;
    textures.push_back(tex);
    textureIndexId[p_textureReferenceName] = activeTextures;
    activeTextures++;
}

/// \brief
void TextureManager::duplicateTexture(const std::string& p_textureReferenceName, Texture *p_texture, ImageColorType p_type)
{

    Texture *tex;
    tex = new Texture();
    tex->height = p_texture->height;
    tex->width = p_texture->width;
    tex->GLtype = GL_TEXTURE_2D;
    tex->textureFileName = p_textureReferenceName;
    tex->imageColorType = p_type;
    textures.push_back(tex);
    textureIndexId[p_textureReferenceName] = activeTextures;
    activeTextures++;
}

/// \brief
void TextureManager::copyTexture(const std::string& /*p_textureDestinationName*/, const std::string& /*p_textureSourceName*/)
{
// WARNING: This function does nothing
//     int textureDstId;
//     Texture *dstTex;
//     int textureSrcId;
//     Texture *srcTex;
//
//     textureDstId = textureIndexId[p_textureDestinationName];
//     dstTex = textures[textureDstId];
//
//     textureSrcId = textureIndexId[p_textureSourceName];
//     srcTex = textures[textureSrcId];
}

/// \brief
void TextureManager::createColorTexture(const std::string& p_textureReferenceName, int p_width, int p_height)
{

    Texture *tex;
    tex = new Texture();
    tex->height = p_height;
    tex->width = p_width;
    tex->GLtype = GL_TEXTURE_2D;
    tex->textureFileName = p_textureReferenceName;
    tex->imageColorType = SIMMEDTK_IMAGECOLOR_OFFSCREENRGBA;
    textures.push_back(tex);
    textureIndexId[p_textureReferenceName] = activeTextures;
    activeTextures++;
}

/// \brief
void TextureManager::initDepthTexture(Texture *p_texture)
{

    glGenTextures(1, &p_texture->textureGLId);
    glBindTexture(GL_TEXTURE_2D, p_texture->textureGLId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, p_texture->width, p_texture->height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    p_texture->isInitialized = true;
}

/// \brief
void TextureManager::initColorTexture(Texture *p_texture)
{

    glGenTextures(1, &p_texture->textureGLId);
    glBindTexture(GL_TEXTURE_2D, p_texture->textureGLId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE); // automatic mipmap
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, p_texture->width, p_texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    p_texture->isInitialized = true;
}

/// \brief
Texture *TextureManager::getTexture(const std::string& p_textureReferenceName)
{

    int textureId;
    Texture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    return texture = textures[textureId];
}

/// \brief
void TextureManager::generateMipMaps(int p_textureId)
{

    Texture *texture;
    texture = textures[p_textureId];
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glGenerateMipmap(GL_TEXTURE_2D);
}

/// \brief
void TextureManager::generateMipMaps(const std::string& p_textureReferenceName)
{

    int textureId;
    Texture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    texture = textures[textureId];
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glGenerateMipmap(GL_TEXTURE_2D);
}
