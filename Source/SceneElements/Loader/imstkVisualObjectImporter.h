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

#pragma once

#include "assimp/Importer.hpp"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include <memory>
#include <fstream>
#include <initializer_list>

#include "imstkSceneObject.h"
#include "imstkSurfaceMesh.h"
#include "imstkAssimpMeshIO.h"

namespace imstk
{
///
/// \class VisualObjectImporter
///
/// \brief Importer for VisualObject
/// Meshes often contain multiple materials in the real world, and each
/// material requires a separate mesh. Since each VisualObject can contain
/// multiple VisualModels, VisualObjectImporter can import these meshes and
/// assign them to the same VisualObject. RenderMaterial creation is also done
/// automatically, with texture searching using common naming conventions.
///
class VisualObjectImporter
{
public:
    ///
    /// \brief Import visual object
    /// \param objName Name for scene object
    /// \param modelFilename File name for visual object to import
    /// \param textureFolderPath Texture folder path
    /// \param scale Scale for visual objects
    /// \param translation Translation for visual objects
    /// \param fileExtension File extension for texture to load
    /// By default, fileExtension will load the extension extracted from the
    /// mesh's texture information. However, this can be inconvenient when
    /// textures are converted to production-ready formats such as .dds.
    /// \returns Visual object
    ///
    static std::shared_ptr<VisualObject> importVisualObject(
        const std::string& objName,
        const std::string& modelFilePath,
        const std::string& textureFolderPath,
        const double scale = 1.0,
        const Vec3d& translation = Vec3d(0, 0, 0),
        const std::string& fileExtension = "");

private:
    ///
    /// \brief Find and add texture to material
    /// \param renderMaterial Material to add the texture to
    /// \param baseFileName Common file name among all textures in the set
    /// \param textureType Texture type
    ///
    static void findAndAddTexture(
        std::shared_ptr<RenderMaterial> renderMaterial,
        const std::string& textureFolderPath,
        const std::string& textureCoreFileName,
        const std::string& textureFileExtension,
        Texture::Type textureType);

    ///
    /// \brief Helper function for getting substring
    /// \param input Input string
    /// \param delimiter Delimiter string for creating the substring
    /// \param lastInstance If true, last instance of the found string is returned
    /// \returns Substring, if delimiter is not found, the input string is returned
    ///
    static std::string getSubstringGivenString(
        const std::string& input,
        const std::string& delimiter,
        const bool lastInstance = false);

    ///
    /// \brief Find and add texture to material
    /// \param renderMaterial Material to add the texture to
    /// \param textureFolderPath Path to folder containing textures
    /// \param textureCoreFileName Common file name among all textures in the set
    /// \param textureFileExtension Extension to file (e.g., ".png")
    /// \param textureType Texture type
    /// \param extensionList List of extentions for labeling textures
    /// This function searches for all files with the following names:
    /// baseFileName.main + extensionList[i] + baseFileName.fileExtension.
    /// The extensionList variables are various naming conventions that can
    /// be interpreted as a specific texture. For example, a texture named
    /// "texture.png" will be expanded to "texture_Normal.png".
    ///
    static void findAndAddTextureWithExtensions(
        std::shared_ptr<RenderMaterial> renderMaterial,
        const std::string& textureFolderPath,
        const std::string& textureCoreFileName,
        const std::string& textureFileExtension,
        Texture::Type textureType,
        std::initializer_list<const char *> extensionList);
}; // VisualObjectImporter
} // imstk
