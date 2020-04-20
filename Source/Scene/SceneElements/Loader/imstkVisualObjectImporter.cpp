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

#include "imstkVisualObjectImporter.h"
#include "imstkRenderMaterial.h"

#include "assimp/Importer.hpp"
#include "assimp/mesh.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "imstkSurfaceMesh.h"
#include "imstkAssimpMeshIO.h"

#include "imstkLogger.h"

namespace imstk
{
std::shared_ptr<VisualObject>
VisualObjectImporter::importVisualObject(
    const std::string& objName,
    const std::string& modelFilePath,
    const std::string& textureFolderPath,
    const double       scale /*= 1.0*/,
    const Vec3d&       translation /*= Vec3d(0, 0, 0)*/,
    const std::string& fileExtension /*= fileExtension*/)
{
    auto type = MeshIO::getFileType(modelFilePath);

    // Check if mesh is supported by Assimp
    if (type != MeshFileType::_3DS
        && type != MeshFileType::OBJ
        && type != MeshFileType::FBX
        && type != MeshFileType::DAE)
    {
        LOG(FATAL) << "File type not supported";
        return nullptr;
    }

    auto visualObject = std::make_shared<VisualObject>(objName);

    // Import mesh(es) and apply some clean-up operations
    Assimp::Importer importer;
    auto             scene = importer.ReadFile(modelFilePath, AssimpMeshIO::getDefaultPostProcessSteps());

    // Check if there is actually a mesh or if the file can be read
    CHECK(scene != nullptr && scene->HasMeshes()) << "AssimpMeshIO::readMeshData error: could not read with reader.";

    std::vector<std::string> materialTextureCoreFileNames;
    std::vector<std::string> materialTextureFileExtensions;

    //std::string texturePath;

    // Normalizes, for examples ".png" to "png"
    auto cleanFileExtension = VisualObjectImporter::getSubstringGivenString(fileExtension, ".", true);

    // Iterate over each material
    for (unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
        auto material = scene->mMaterials[i];

        aiString fileName;
        material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &fileName);

        auto textureFileName = std::string(fileName.C_Str());
        textureFileName = VisualObjectImporter::getSubstringGivenString(textureFileName, "/", true);
        textureFileName = VisualObjectImporter::getSubstringGivenString(textureFileName, "\\", true);

        auto coreFileName = VisualObjectImporter::getSubstringGivenString(textureFileName, ".", false);
        coreFileName = VisualObjectImporter::getSubstringGivenString(coreFileName, "_", false);
        materialTextureCoreFileNames.push_back(coreFileName);

        auto extension = VisualObjectImporter::getSubstringGivenString(textureFileName, ".", true);
        extension = cleanFileExtension == "" ? extension : cleanFileExtension;
        materialTextureFileExtensions.push_back(extension);
    }

    std::vector<Texture::Type> textureTypes;
    textureTypes.push_back(Texture::Type::Diffuse);
    textureTypes.push_back(Texture::Type::Normal);
    textureTypes.push_back(Texture::Type::Roughness);
    textureTypes.push_back(Texture::Type::Metalness);
    textureTypes.push_back(Texture::Type::AmbientOcclusion);
    textureTypes.push_back(Texture::Type::SubsurfaceScattering);

    // Iterate over each mesh
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        // Get first mesh
        auto importedMesh = scene->mMeshes[i];

        auto mesh = AssimpMeshIO::convertAssimpMesh(importedMesh);
        mesh->setTranslation(translation);
        mesh->setScaling(scale);

        auto visualModel    = std::make_shared<VisualModel>(mesh);
        auto renderMaterial = std::make_shared<RenderMaterial>();

        auto index = importedMesh->mMaterialIndex;

        // Find and add available textures
        for (auto textureType : textureTypes)
        {
            VisualObjectImporter::findAndAddTexture(
                renderMaterial,
                textureFolderPath,
                materialTextureCoreFileNames[index],
                materialTextureFileExtensions[index],
                textureType);
        }

        visualModel->setRenderMaterial(renderMaterial);

        visualObject->addVisualModel(visualModel);
    }

    return visualObject;
}

void
VisualObjectImporter::findAndAddTexture(
    std::shared_ptr<RenderMaterial> renderMaterial,
    const std::string&              textureFolderPath,
    const std::string&              textureCoreFileName,
    const std::string&              textureFileExtension,
    Texture::Type                   textureType)
{
    // To support other texture naming conventions, simply add to each list.
    // Right now, only extensions that don't contain an underscore in the
    // middle are supported (e.g., "_BaseColor" is supported but _"Base_Color"
    // is not supported).

    switch (textureType)
    {
    case Texture::Type::Diffuse:
        VisualObjectImporter::findAndAddTextureWithExtensions(
                renderMaterial,
                textureFolderPath,
                textureCoreFileName,
                textureFileExtension,
                textureType,
            { "_BaseColor", "_Diffuse", "_Albedo" });
        break;
    case Texture::Type::Normal:
        VisualObjectImporter::findAndAddTextureWithExtensions(
                renderMaterial,
                textureFolderPath,
                textureCoreFileName,
                textureFileExtension,
                textureType,
            { "_Normal" });
        break;
    case Texture::Type::Roughness:
        VisualObjectImporter::findAndAddTextureWithExtensions(
                renderMaterial,
                textureFolderPath,
                textureCoreFileName,
                textureFileExtension,
                textureType,
            { "_Roughness" });
        break;
    case Texture::Type::Metalness:
        VisualObjectImporter::findAndAddTextureWithExtensions(
                renderMaterial,
                textureFolderPath,
                textureCoreFileName,
                textureFileExtension,
                textureType,
            { "_Metalness", "_Metallic" });
        break;
    case Texture::Type::AmbientOcclusion:
        VisualObjectImporter::findAndAddTextureWithExtensions(
                renderMaterial,
                textureFolderPath,
                textureCoreFileName,
                textureFileExtension,
                textureType,
            { "_AO", "_AmbientOcclusion" });
        break;
    case Texture::Type::SubsurfaceScattering:
        VisualObjectImporter::findAndAddTextureWithExtensions(
                renderMaterial,
                textureFolderPath,
                textureCoreFileName,
                textureFileExtension,
                textureType,
            { "_SSS", "_SubsurfaceScattering" });
        break;
    default:
        break;
    }
}

std::string
VisualObjectImporter::getSubstringGivenString(
    const std::string& input,
    const std::string& delimiter,
    const bool         lastInstance /*= false*/)
{
    unsigned long long index = 0;
    unsigned long long tempIndex;

    if (lastInstance)
    {
        tempIndex = input.rfind(delimiter) + 1;
        if (tempIndex >= input.length())
        {
            return input;
        }
    }
    else
    {
        tempIndex = input.find(delimiter);
    }

    if (tempIndex == std::string::npos)
    {
        index = lastInstance ? 0 : input.length();
    }
    else
    {
        index = tempIndex;
    }

    if (lastInstance)
    {
        return input.substr(index);
    }

    return input.substr(0, index);
}

void
VisualObjectImporter::findAndAddTextureWithExtensions(
    std::shared_ptr<RenderMaterial>    renderMaterial,
    const std::string&                 textureFolderPath,
    const std::string&                 textureCoreFileName,
    const std::string&                 textureFileExtension,
    Texture::Type                      textureType,
    std::initializer_list<const char*> extensions)
{
    for (auto extension : extensions)
    {
        auto fileName = textureFolderPath + textureCoreFileName + extension + "." + textureFileExtension;

        // Check if file exists
        std::ifstream file(fileName);
        if (file.good())
        {
            file.close();
            auto texture = std::make_shared<Texture>(fileName, textureType);
            renderMaterial->addTexture(texture);
            return;
        }
    }
}
} // imstk
