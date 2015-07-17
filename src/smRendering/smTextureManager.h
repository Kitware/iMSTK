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

#ifndef SMTEXTUREMANAGER_H
#define SMTEXTUREMANAGER_H

// STL includes
#include <unordered_map>
#include <cstring>

// 3rd Party includes
#include <SFML/Graphics/Image.hpp>

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smConfigRendering.h"
#include "smCore/smTexture.h"
#include "smCore/smErrorLog.h"

/// \brief texture manager. It loads any image format and initializes in the GL context
class smTextureManager: public smCoreClass
{

    static std::shared_ptr<smErrorLog> errorLog;
    static std::vector<smTexture*> textures;
    static std::unordered_map<smString, smInt> textureIndexId;
    static smInt activeTextures;
    static smBool isInitialized;
    static smBool isInitializedGL;
    static smBool isDeleteImagesEnabled;

public:
    /// \brief init function called by the renderer thread
    static smTextureReturnType initGLTextures();
    /// \brief initialization function
    static void init(std::shared_ptr<smErrorLog> p_errorLog = nullptr)
    {
        if(!isInitialized)
        {
            errorLog = p_errorLog;
            textures.resize(SIMMEDTK_MAX_TEXTURENBR);
            textures.clear();
            activeTextures = 0;
            isDeleteImagesEnabled = true;
            isInitialized = true;
        }
    }
    /// \brief load textures with file name, reference that that is assigned to it, and returned texture id
    static smTextureReturnType loadTexture(const smString& p_fileName,
                                           const smString& p_textureReferenceName,
                                           smInt &p_textureId);

    static smTextureReturnType loadTexture(const smString& p_fileName,
                                           const smString& p_textureReferenceName,
                                           smBool p_flipImage = false);

    static smTextureReturnType findTextureId(const smString& p_textureReferenceName,
            smInt &p_textureId);
    /// \brief activate textures based on texture reference name, texture reference, texture id and GL order
    static GLuint activateTexture(smTexture *p_texture);
    static GLuint activateTexture(const smString& p_textureReferenceName);
    static GLuint activateTexture(smInt p_textureId);

    static GLuint activateTexture(const smString& p_textureReferenceName,
                                  smInt p_textureGLOrder);

    static GLuint activateTexture(smTexture *p_texture, smInt p_textureGLOrder,
                                  smInt p_shaderBindGLId);

    static GLuint activateTexture(const smString& p_textureReferenceName,
                                  smInt p_textureGLOrder, smInt p_shaderBindName);

    static GLuint activateTexture(smInt p_textureId, smInt p_textureGLOrder);
    static void  activateTextureGL(GLuint  p_textureId, smInt p_textureGLOrder);
    /// \brief disable textures
    static GLuint disableTexture(const smString& p_textureReferenceName);

    static GLuint disableTexture(const smString& p_textureReferenceName,
                                 smInt p_textureGLOrder);

    static GLuint disableTexture(smInt p_textureId);

    /// \brief functions return GL texture ID
    static GLuint getOpenglTextureId(const smString& p_textureReferenceName);
    static GLuint getOpenglTextureId(smInt p_textureId);
    /// \brief to get texture with given texture reference name
    static smTexture * getTexture(const smString& p_textureReferenceName);
    /// \brief to create a depth texture
    static void createDepthTexture(const smString& p_textureReferenceName,
                                   smInt p_width, smInt p_height);

    /// \brief to create a color texture
    static void  createColorTexture(const smString& p_textureReferenceName,
                                    smInt p_width, smInt p_height);
    /// \brief initialize depth texture and color texture
    static void initDepthTexture(smTexture *p_texture);
    static void initColorTexture(smTexture *p_texture);
    /// \brief generate mip maps
    static void generateMipMaps(smInt p_textureId);
    static void generateMipMaps(const smString& p_textureReferenceName);
    /// \brief to duplicate the texture
    static void duplicateTexture(const smString& p_textureReferenceName,
                                 smTexture *p_texture, ImageColorType p_type);

    /// \brief copy the  texture specified with p_textureSourceName to the  texture specified with p_textureDestinationName
    static void copyTexture(const smString& p_textureDestinationName,
                            const smString& p_textureSourceName);
};

#endif
