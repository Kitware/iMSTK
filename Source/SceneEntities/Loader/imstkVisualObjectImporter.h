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

#include "imstkTexture.h"
#include "imstkMath.h"

struct aiMaterial;

namespace imstk
{
class AnimatedObject;
class RenderMaterial;
class SceneObject;

///
/// \class ObjectIO
///
/// \brief ObjectIO provides SceneObject reading capabilities
/// It is used to read complex visual objects or animated objects
///
class ObjectIO
{
public:
    ///
    /// \brief Import a scene object
    /// \param objName Name for the object
    /// \param modelFilename File name for visual object to import
    /// \param textureFolderPath Texture folder path
    /// \param scale Scale for visual objects
    /// \param translation Translation for visual objects
    ///
    static std::shared_ptr<SceneObject> importSceneObject(
        const std::string& objName,
        const std::string& modelFilePath,
        const std::string& textureFolderPath,
        const Mat4d&       transform = Mat4d::Identity());

private:
    ///
    /// \brief Returns texture with file name and type, checks it exists
    ///
    static std::shared_ptr<Texture> createTexture(std::string textureFolderPath, std::string textureFilePath, Texture::Type textureType);

    ///
    /// \brief Converts aiMaterial to RenderMaterial
    ///
    static std::shared_ptr<RenderMaterial> readMaterial(aiMaterial* mat, std::string textureFolderPath);

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
        const bool         lastInstance = false);
};
} // namespace imstk