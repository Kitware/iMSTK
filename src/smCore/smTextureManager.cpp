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

#include "smCore/smTextureManager.h"
#include "smUtilities/smGLUtils.h"

#include <cassert>

std::shared_ptr<smErrorLog> smTextureManager:: errorLog;
std::vector<smTexture*>  smTextureManager:: textures;
std::unordered_map<smString, smInt>  smTextureManager::textureIndexId;
smInt smTextureManager:: activeTextures;
smBool smTextureManager::isInitialized = false;
smBool smTextureManager::isInitializedGL = false;
smBool smTextureManager::isDeleteImagesEnabled = false;

/// \brief
smTextureReturnType smTextureManager::initGLTextures()
{
    smString texManagerError;
    smTexture *texture;

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

        if (smGLUtils::queryGLError(texManagerError))
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
smTextureReturnType smTextureManager::loadTexture(const smString& p_fileName,
        const smString& p_textureReferenceName, smInt &p_textureId)
{
    smTextureReturnType ret = loadTexture(p_fileName, p_textureReferenceName, true);
    if (ret == SIMMEDTK_TEXTURE_OK)
    {
        p_textureId = textureIndexId[p_textureReferenceName];
    }
    return ret;
}

/// \brief
smTextureReturnType smTextureManager::loadTexture(const smString& p_fileName, const smString& p_textureReferenceName, smBool p_flipImage)
{
    smTexture *texture = nullptr;
    sf::Vector2u imageSize;

    assert(p_fileName != "");
    assert(p_textureReferenceName != "");

    if (!isInitialized)
    {
        return SIMMEDTK_TEXTURE_DRIVERNOTINITIALIZED;
    }

    texture = new smTexture();
    assert(texture);
    if (false == texture->image.loadFromFile(p_fileName))
    {
        std::cout << "[smTextureManager::loadTexture] Texture not found: \"" << p_fileName << "\"\n";
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
smTextureReturnType smTextureManager::findTextureId(const smString& p_textureReferenceName,
        smInt &p_textureId)
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
GLuint smTextureManager::activateTexture(const smString& p_textureReferenceName)
{

    smInt textureId;
    smTexture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    texture = textures[textureId];

    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glEnable(GL_TEXTURE_2D);
    return texture->textureGLId;
}

/// \brief activate the texture given the smTexture argument
GLuint smTextureManager::activateTexture(smTexture *p_texture)
{

    glBindTexture(GL_TEXTURE_2D, p_texture->textureGLId);
    glEnable(GL_TEXTURE_2D);
    return p_texture->textureGLId;
}

/// \brief This function binds the texture to the appropriate texture.
///For instance if the argument is 0, the it will bind to GL_TEXTURE0
GLuint smTextureManager::activateTexture(const smString& p_textureReferenceName, smInt p_textureGLOrder)
{

    smInt textureId;
    smTexture *texture;
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
GLuint smTextureManager::activateTexture(const smString& p_textureReferenceName,
        smInt p_textureGLOrder, smInt p_shaderBindGLId)
{

    smInt textureId;
    smTexture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    texture = textures[textureId];
    glActiveTexture(GL_TEXTURE0 + p_textureGLOrder);
    glUniform1iARB(p_shaderBindGLId, p_textureGLOrder);
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glEnable(GL_TEXTURE_2D);
    return texture->textureGLId;
}

/// \brief This function binds the smTexture to the appropriate shader texture.
/// For instance if the argument is 0, the it will bind to GL_TEXTURE0
/// Also for the shader the binded name will be p_shaderBindName
GLuint smTextureManager::activateTexture(smTexture *p_texture,
        smInt p_textureGLOrder, smInt p_shaderBindGLId)
{

    glActiveTexture(GL_TEXTURE0 + p_textureGLOrder);
    glUniform1iARB(p_shaderBindGLId, p_textureGLOrder);
    glBindTexture(GL_TEXTURE_2D, p_texture->textureGLId);
    glEnable(GL_TEXTURE_2D);
    return p_texture->textureGLId;
}

/// \brief
GLuint smTextureManager::activateTexture(smInt p_textureId)
{

    smTexture *texture;
    texture = textures[p_textureId];
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glEnable(GL_TEXTURE_2D);
    return texture->textureGLId;

}

/// \brief
GLuint smTextureManager::activateTexture(smInt p_textureId, smInt p_textureGLOrder)
{

    smTexture *texture;
    texture = textures[p_textureId];
    glActiveTexture(GL_TEXTURE0 + p_textureGLOrder);
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glEnable(GL_TEXTURE_2D);
    return texture->textureGLId;
}

/// \brief
void smTextureManager::activateTextureGL(GLuint  p_textureId, smInt p_textureGLOrder)
{

    glActiveTexture(GL_TEXTURE0 + p_textureGLOrder);
    glBindTexture(GL_TEXTURE_2D, p_textureId);
    glEnable(GL_TEXTURE_2D);
}

/// \brief
GLuint smTextureManager::disableTexture(const smString& p_textureReferenceName)
{

    GLuint textureId;
    smTexture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    texture = textures[textureId];
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glDisable(GL_TEXTURE_2D);
    return texture->textureGLId;
}

/// \brief
GLuint smTextureManager::disableTexture(const smString& p_textureReferenceName, smInt p_textureGLOrder)
{

    GLuint textureId;
    smTexture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    texture = textures[textureId];

    glActiveTexture(GL_TEXTURE0 + p_textureGLOrder);
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glDisable(GL_TEXTURE_2D);
    return texture->textureGLId;
}

/// \brief
GLuint smTextureManager::disableTexture(smInt p_textureId)
{

    smTexture *texture;
    texture = textures[p_textureId];
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glDisable(GL_TEXTURE_2D);
    return texture->textureGLId;
}

/// \brief
GLuint smTextureManager::getOpenglTextureId(const smString& p_textureReferenceName)
{

    smInt textureId;
    smTexture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    texture = textures[textureId];
    return texture->textureGLId;

}

/// \brief
GLuint smTextureManager::getOpenglTextureId(smInt p_textureId)
{

    smTexture *texture;
    texture = textures[p_textureId];
    return texture->textureGLId;

}

/// \brief
void smTextureManager::createDepthTexture(const smString& p_textureReferenceName, smInt p_width, smInt p_height)
{

    smTexture *tex;
    tex = new smTexture();
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
void smTextureManager::duplicateTexture(const smString& p_textureReferenceName, smTexture *p_texture, ImageColorType p_type)
{

    smTexture *tex;
    tex = new smTexture();
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
void smTextureManager::copyTexture(const smString& /*p_textureDestinationName*/, const smString& /*p_textureSourceName*/)
{
// WARNING: This function does nothing
//     smInt textureDstId;
//     smTexture *dstTex;
//     smInt textureSrcId;
//     smTexture *srcTex;
//
//     textureDstId = textureIndexId[p_textureDestinationName];
//     dstTex = textures[textureDstId];
//
//     textureSrcId = textureIndexId[p_textureSourceName];
//     srcTex = textures[textureSrcId];
}

/// \brief
void smTextureManager::createColorTexture(const smString& p_textureReferenceName, smInt p_width, smInt p_height)
{

    smTexture *tex;
    tex = new smTexture();
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
void smTextureManager::initDepthTexture(smTexture *p_texture)
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
void smTextureManager::initColorTexture(smTexture *p_texture)
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
smTexture *smTextureManager::getTexture(const smString& p_textureReferenceName)
{

    smInt textureId;
    smTexture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    return texture = textures[textureId];
}

/// \brief
void smTextureManager::generateMipMaps(smInt p_textureId)
{

    smTexture *texture;
    texture = textures[p_textureId];
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glGenerateMipmap(GL_TEXTURE_2D);
}

/// \brief
void smTextureManager::generateMipMaps(const smString& p_textureReferenceName)
{

    smInt textureId;
    smTexture *texture;
    textureId = textureIndexId[p_textureReferenceName];
    texture = textures[textureId];
    glBindTexture(GL_TEXTURE_2D, texture->textureGLId);
    glGenerateMipmap(GL_TEXTURE_2D);
}
