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

#ifndef RENDERING_TEXTUREMANAGER_H
#define RENDERING_TEXTUREMANAGER_H

// STL includes
#include <unordered_map>
#include <cstring>

// 3rd Party includes
#include <SFML/Graphics/Image.hpp>

// iMSTK includes
#include "Core/Config.h"
#include "ConfigRendering.h"
#include "Core/Texture.h"
#include "Core/ErrorLog.h"

/// \brief texture manager. It loads any image format and initializes in the GL context
class TextureManager: public CoreClass
{

    static std::shared_ptr<ErrorLog> errorLog;
    static std::vector<std::shared_ptr<Texture>> textures;
    static std::unordered_map<std::string, int> textureIndexId;
    static int activeTextures;
    static bool isInitialized;
    static bool isInitializedGL;
    static bool isDeleteImagesEnabled;

public:
    /// \brief init function called by the renderer thread
    static TextureReturnType initGLTextures();
    /// \brief initialization function
    static void init(std::shared_ptr<ErrorLog> p_errorLog = nullptr)
    {
        if(!isInitialized)
        {
            errorLog = p_errorLog;
            textures.resize(IMSTK_MAX_TEXTURENBR);
            textures.clear();
            activeTextures = 0;
            isDeleteImagesEnabled = true;
            isInitialized = true;
        }
    }
    /// \brief adds texture with file name, reference that that is assigned to it, and returned texture id
    static TextureReturnType addTexture(const std::string& p_fileName,
                                        const std::string& p_textureReferenceName);

    /// \brief load textures with file name, reference that that is assigned to it, and returned texture id
    static TextureReturnType loadTexture(const std::string& p_fileName,
                                           const std::string& p_textureReferenceName,
                                           int &p_textureId);

    static TextureReturnType loadTexture(const std::string& p_fileName,
                                           const std::string& p_textureReferenceName,
                                           bool p_flipImage = false);

    static TextureReturnType findTextureId(const std::string& p_textureReferenceName,
            int &p_textureId);

    /// \brief activate textures based on texture reference name, texture reference, texture id and GL order
    static GLuint activateTexture(std::shared_ptr<Texture> p_texture);
    static GLuint activateTexture(const std::string& p_textureReferenceName);
    static GLuint activateTexture(int p_textureId);

    static GLuint activateTexture(const std::string& p_textureReferenceName,
                                  int p_textureGLOrder);

    static GLuint activateTexture(std::shared_ptr<Texture> p_texture, int p_textureGLOrder,
                                  int p_shaderBindGLId);

    static GLuint activateTexture(const std::string& p_textureReferenceName,
                                  int p_textureGLOrder, int p_shaderBindName);

    static GLuint activateTexture(int p_textureId, int p_textureGLOrder);
    static void  activateTextureGL(GLuint  p_textureId, int p_textureGLOrder);
    /// \brief disable textures
    static GLuint disableTexture(const std::string& p_textureReferenceName);

    static GLuint disableTexture(const std::string& p_textureReferenceName,
                                 int p_textureGLOrder);

    static GLuint disableTexture(int p_textureId);

    /// \brief functions return GL texture ID
    static GLuint getOpenglTextureId(const std::string& p_textureReferenceName);
    static GLuint getOpenglTextureId(int p_textureId);
    /// \brief to get texture with given texture reference name
    static std::shared_ptr<Texture>  getTexture(const std::string& p_textureReferenceName);
    static std::shared_ptr<Texture>  getTexture(const int& id);
    /// \brief to create a depth texture
    static void createDepthTexture(const std::string& p_textureReferenceName,
                                   int p_width, int p_height);

    /// \brief to create a color texture
    static void  createColorTexture(const std::string& p_textureReferenceName,
                                    int p_width, int p_height);
    /// \brief initialize depth texture and color texture
    static void initDepthTexture(std::shared_ptr<Texture> p_texture);
    static void initColorTexture(std::shared_ptr<Texture> p_texture);
    /// \brief generate mip maps
    static void generateMipMaps(int p_textureId);
    static void generateMipMaps(const std::string& p_textureReferenceName);
    /// \brief to duplicate the texture
    static void duplicateTexture(const std::string& p_textureReferenceName,
                                 std::shared_ptr<Texture> p_texture, ImageColorType p_type);

    /// \brief copy the  texture specified with p_textureSourceName to the  texture specified with p_textureDestinationName
    static void copyTexture(const std::string& p_textureDestinationName,
                            const std::string& p_textureSourceName);
};

#endif
