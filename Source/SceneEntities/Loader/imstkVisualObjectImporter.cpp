/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVisualObjectImporter.h"
#include "imstkAssimpMeshIO.h"
#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkRenderMaterial.h"
#include "imstkSceneObject.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"
#include "imstkVisualModel.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <fstream>
#include <stack>
#include <unordered_map>

namespace imstk
{
static Mat4d
aiMatToMat4d(const aiMatrix4x4& m)
{
    Mat4d results;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            results(i, j) = static_cast<double>(m[i][j]);
        }
    }
    return results;
}

static aiMatrix4x4
mat4dToAiMat(const Mat4d& m)
{
    aiMatrix4x4 results;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            results[i][j] = static_cast<float>(m(i, j));
        }
    }
    return results;
}

std::shared_ptr<SceneObject>
ObjectIO::importSceneObject(
    const std::string& objName,
    const std::string& modelFilePath,
    const std::string& textureFolderPath,
    const Mat4d&       transform)
{
    auto type = MeshIO::getFileType(modelFilePath);

    // Check if mesh is supported by Assimp
    if (type != MeshFileType::_3DS
        && type != MeshFileType::OBJ
        && type != MeshFileType::FBX
        && type != MeshFileType::DAE)
    {
        LOG(FATAL) << "Error: File type not supported! Input model file path: " << modelFilePath;
        return nullptr;
    }

    auto visualObject = std::make_shared<SceneObject>(objName);

    // Import mesh(es) and apply some clean-up operations
    Assimp::Importer importer;
    const aiScene*   scene = importer.ReadFile(modelFilePath, AssimpMeshIO::getDefaultPostProcessSteps());

    // Check if there is actually a mesh or if the file can be read
    CHECK(scene != nullptr && scene->HasMeshes()) << "Error: could not read model with Assimp reader! Input model file path: "
                                                  << modelFilePath;

    // Iterate over each material
    std::vector<std::shared_ptr<RenderMaterial>> materials(scene->mNumMaterials);
    for (unsigned int i = 0; i < scene->mNumMaterials; i++)
    {
        materials[i] = readMaterial(scene->mMaterials[i], textureFolderPath);
    }

    // Read all meshes
    std::vector<std::shared_ptr<PointSet>>       meshes(scene->mNumMeshes);
    std::vector<std::shared_ptr<RenderMaterial>> meshMaterials(scene->mNumMeshes);
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* importedMesh = scene->mMeshes[i];
        meshes[i] = AssimpMeshIO::convertAssimpMesh(importedMesh);
        meshMaterials[i] = materials[importedMesh->mMaterialIndex];
    }

    // Iterate through assimp graph
    std::stack<aiNode*> nodes;
    nodes.push(scene->mRootNode);
    scene->mRootNode->mTransformation = mat4dToAiMat(transform) * scene->mRootNode->mTransformation;
    std::unordered_map<aiNode*, Mat4d> worldTransforms;
    while (!nodes.empty())
    {
        aiNode* currNode = nodes.top();
        nodes.pop();

        Mat4d parentWorldTransform = Mat4d::Identity();
        if (currNode->mParent != nullptr)
        {
            parentWorldTransform = worldTransforms[currNode->mParent];
        }
        Mat4d localTransform     = aiMatToMat4d(currNode->mTransformation);
        Mat4d currWorldTransform = parentWorldTransform * localTransform;
        worldTransforms[currNode] = currWorldTransform;

        for (unsigned int i = 0; i < currNode->mNumMeshes; i++)
        {
            const unsigned int meshIndex = currNode->mMeshes[i];

            // Copy, transform, and insert the mesh (\todo: Better deep copy support)
            std::shared_ptr<PointSet> copyMesh = meshes[meshIndex]->clone();

            auto visualModel = visualObject->addComponent<VisualModel>();
            visualModel->setGeometry(copyMesh);
            visualModel->setName(std::string(currNode->mName.C_Str()));

            copyMesh->transform(currWorldTransform, Geometry::TransformType::ApplyToData);
            visualModel->setRenderMaterial(meshMaterials[meshIndex]);
        }

        for (unsigned int i = 0; i < currNode->mNumChildren; i++)
        {
            nodes.push(currNode->mChildren[i]);
        }
    }

    return visualObject;
}

std::shared_ptr<Texture>
ObjectIO::createTexture(std::string textureFolderPath, std::string textureFilePath, Texture::Type textureType)
{
    textureFilePath = getSubstringGivenString(textureFilePath, "/", true);
    textureFilePath = getSubstringGivenString(textureFilePath, "\\", true);

    std::string fileName = getSubstringGivenString(textureFilePath, ".", false);

    const std::string fileExt = getSubstringGivenString(textureFilePath, ".", true);

    const std::string filePath = textureFolderPath + fileName + "." + fileExt;

    // Check if file exists
    std::ifstream file(filePath);
    if (file.good())
    {
        file.close();
        return std::make_shared<Texture>(filePath, textureType);
    }
    return nullptr;
}

std::shared_ptr<RenderMaterial>
ObjectIO::readMaterial(aiMaterial* material, std::string textureFolderPath)
{
    // Create our material
    auto renderMaterial = std::make_shared<RenderMaterial>();
    renderMaterial->setShadingModel(RenderMaterial::ShadingModel::Phong);

    // Go through all the ai properties, not all included here
    aiString name;
    aiReturn ret = material->Get(AI_MATKEY_NAME, name);
    if (ret == AI_SUCCESS)
    {
        renderMaterial->setName(std::string(name.C_Str()));
    }

    aiColor3D ambientColor;
    ret = material->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);
    if (ret == AI_SUCCESS)
    {
        renderMaterial->setAmbientColor(Color(ambientColor.r, ambientColor.g, ambientColor.b));
    }

    aiColor3D diffuseColor;
    ret = material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
    if (ret == AI_SUCCESS)
    {
        renderMaterial->setDiffuseColor(Color(diffuseColor.r, diffuseColor.g, diffuseColor.b));
    }

    aiColor3D specularColor;
    ret = material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
    if (ret == AI_SUCCESS)
    {
        renderMaterial->setSpecularColor(Color(specularColor.r, specularColor.g, specularColor.b));
    }

    int useWireframe;
    ret = material->Get(AI_MATKEY_ENABLE_WIREFRAME, useWireframe);
    if (ret == AI_SUCCESS)
    {
        if (useWireframe == 1)
        {
            renderMaterial->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
        }
    }

    int useBackfaceRendering;
    ret = material->Get(AI_MATKEY_TWOSIDED, useBackfaceRendering);
    if (ret == AI_SUCCESS)
    {
        renderMaterial->setBackFaceCulling(static_cast<bool>(useBackfaceRendering));
    }

    float opacity;
    ret = material->Get(AI_MATKEY_OPACITY, opacity);
    if (ret == AI_SUCCESS)
    {
        renderMaterial->setOpacity(opacity);
    }

    float shininess;
    ret = material->Get(AI_MATKEY_SHININESS, shininess);
    if (ret == AI_SUCCESS)
    {
        renderMaterial->setSpecular(shininess);
    }

    float shininessStrength;
    ret = material->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrength);
    if (ret == AI_SUCCESS)
    {
        renderMaterial->setSpecularPower(shininessStrength);
    }

    float reflectivity;
    ret = material->Get(AI_MATKEY_REFLECTIVITY, reflectivity);
    if (ret == AI_SUCCESS)
    {
        // Not supported yet
    }

    renderMaterial->setRecomputeVertexNormals(false);

    aiString texFilePath;
    ret = material->GetTexture(aiTextureType::aiTextureType_AMBIENT, 0, &texFilePath);
    if (ret == AI_SUCCESS)
    {
        std::shared_ptr<Texture> tex = createTexture(textureFolderPath, std::string(texFilePath.C_Str()), Texture::Type::AmbientOcclusion);
        if (tex != nullptr)
        {
            renderMaterial->addTexture(tex);
        }
    }
    ret = material->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0, &texFilePath);
    if (ret == AI_SUCCESS)
    {
        std::shared_ptr<Texture> tex = createTexture(textureFolderPath, std::string(texFilePath.C_Str()), Texture::Type::Diffuse);
        if (tex != nullptr)
        {
            renderMaterial->addTexture(tex);
        }
    }
    ret = material->GetTexture(aiTextureType::aiTextureType_EMISSIVE, 0, &texFilePath);
    if (ret == AI_SUCCESS)
    {
        std::shared_ptr<Texture> tex = createTexture(textureFolderPath, std::string(texFilePath.C_Str()), Texture::Type::Emissive);
        if (tex != nullptr)
        {
            renderMaterial->addTexture(tex);
        }
    }
    ret = material->GetTexture(aiTextureType::aiTextureType_NORMALS, 0, &texFilePath);
    if (ret == AI_SUCCESS)
    {
        std::shared_ptr<Texture> tex = createTexture(textureFolderPath, std::string(texFilePath.C_Str()), Texture::Type::Normal);
        if (tex != nullptr)
        {
            renderMaterial->addTexture(tex);
        }
    }
    ret = material->GetTexture(aiTextureType::aiTextureType_SPECULAR, 0, &texFilePath);
    if (ret == AI_SUCCESS)
    {
        std::shared_ptr<Texture> tex = createTexture(textureFolderPath, std::string(texFilePath.C_Str()), Texture::Type::Metalness);
        if (tex != nullptr)
        {
            renderMaterial->addTexture(tex);
        }
    }
    return renderMaterial;
}

std::string
ObjectIO::getSubstringGivenString(
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
} // namespace imstk