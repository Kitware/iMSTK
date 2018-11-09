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

#include "imstkVTKCustomPolyDataMapper.h"

namespace imstk
{
vtkStandardNewMacro(VTKCustomPolyDataMapper);

void
VTKCustomPolyDataMapper::BuildBufferObjects(vtkRenderer * renderer, vtkActor * actor)
{
    auto polyData = this->GetInput();

    auto renderWindow = (vtkOpenGLRenderWindow*)renderer->GetRenderWindow();
    auto VBOCache = renderWindow->GetVBOCache();

    this->VBOs->CacheDataArray("inputPosition", polyData->GetPoints()->GetData(), VBOCache, VTK_FLOAT);
    this->VBOs->CacheDataArray("inputNormal", polyData->GetPointData()->GetNormals(), VBOCache, VTK_FLOAT);
    this->VBOs->CacheDataArray("inputUV", polyData->GetPointData()->GetTCoords(), VBOCache, VTK_FLOAT);
    this->VBOs->CacheDataArray("inputColor", polyData->GetPointData()->GetScalars(), VBOCache, VTK_UNSIGNED_CHAR);

    if (polyData->GetPointData()->GetArray("tangents"))
    {
        this->VBOs->CacheDataArray("inputTangent", polyData->GetPointData()->GetArray("tangents"), VBOCache, VTK_FLOAT);
    }

    this->VBOs->BuildAllVBOs(VBOCache);

    this->BuildIBO(renderer, actor, polyData);
    this->VBOBuildTime.Modified();
}

void
VTKCustomPolyDataMapper::ReplaceShaderValues(
    std::map<vtkShader::Type, vtkShader*> vtkNotUsed(shaders),
    vtkRenderer * vtkNotUsed(renderer),
    vtkActor * vtkNotUsed(actor))
{
}

void
VTKCustomPolyDataMapper::GetShaderTemplate(
    std::map<vtkShader::Type, vtkShader*> shaders,
    vtkRenderer * vtkNotUsed(renderer),
    vtkActor * actor)
{
    this->loadShader("./Shaders/VTKShaders/mesh.vert", m_vertexShaderSource);
    this->loadShader("./Shaders/VTKShaders/mesh.frag", m_fragmentShaderSource);

    //auto renderMaterial = m_geometry->getRenderMaterial();

    auto diffuseTexture = m_renderMaterial->getTexture(Texture::DIFFUSE);
    auto cubemapTexture = m_renderMaterial->getTexture(Texture::CUBEMAP);

    //auto surfaceMesh = std::dynamic_pointer_cast<SurfaceMesh>(m_geometry);

    if (this->GetOpenGLMode(actor->GetProperty()->GetRepresentation(), this->LastBoundBO->PrimitiveType) == GL_TRIANGLES)
    {
        m_fragmentShaderSource = "#define SHADED\n" + m_fragmentShaderSource;
    }

    if (diffuseTexture->getPath() != "")
    {
        m_fragmentShaderSource = "#define DIFFUSE_TEXTURE\n" + m_fragmentShaderSource;
    }
    if (cubemapTexture->getPath() != "")
    {
        m_fragmentShaderSource = "#define CUBEMAP_TEXTURE\n" + m_fragmentShaderSource;
    }

    m_vertexShaderSource = "#version 330\n" + m_vertexShaderSource;
    m_fragmentShaderSource = "#version 330\n" + m_fragmentShaderSource;
    shaders[vtkShader::Vertex]->SetSource(m_vertexShaderSource.c_str());
    shaders[vtkShader::Fragment]->SetSource(m_fragmentShaderSource.c_str());
}

void
VTKCustomPolyDataMapper::SetMapperShaderParameters(
    vtkOpenGLHelper& helper,
    vtkRenderer * renderer,
    vtkActor * actor)
{
    auto textures = this->GetTextures(actor);
    auto material = m_renderMaterial;

    helper.VAO->Bind();

    this->VBOs->AddAllAttributesToVAO(helper.Program, helper.VAO);
    auto camera = renderer->GetActiveCamera();

    vtkMatrix4x4 * viewMatrix;
    vtkMatrix4x4 * projectionMatrix;
    vtkMatrix3x3 * rotationMatrix;
    vtkMatrix4x4 * viewProjectionMatrix;
    vtkMatrix3x3 * modelRotationMatrix;
    vtkMatrix4x4 * modelMatrix;

    ((vtkOpenGLCamera*)camera)->GetKeyMatrices(renderer, viewMatrix, rotationMatrix, projectionMatrix, viewProjectionMatrix);
    ((vtkOpenGLActor*)actor)->GetKeyMatrices(modelMatrix, modelRotationMatrix);

    // Per renderer: 16 light limit for iMSTK, VTK supports 6
    float lightPosition[16][3]; // 3 float position
    int lightType[16]; // 1 bool directional vs. point light
    float lightColor[16][4]; // 3 float color, 1 float intensity
    float lightDirection[16][4]; // 3 float direction, 1 float angle

    auto lights = renderer->GetLights();
    lights->InitTraversal();

    for (unsigned int i = 0; i < 16; i++)
    {
        auto light = lights->GetNextItem();

        if (light)
        {
            lightPosition[i][0] = light->GetPosition()[0];
            lightPosition[i][1] = light->GetPosition()[1];
            lightPosition[i][2] = light->GetPosition()[2];

            if (light->GetPositional())
            {
                lightType[i] = 2;

                if (light->GetConeAngle() < 178.0)
                {
                    lightType[i] = 3;
                }
            }
            else
            {
                lightType[i] = 1;
            }

            lightColor[i][0] = light->GetDiffuseColor()[0];
            lightColor[i][1] = light->GetDiffuseColor()[1];
            lightColor[i][2] = light->GetDiffuseColor()[2];
            lightColor[i][3] = light->GetIntensity();

            lightDirection[i][0] = light->GetFocalPoint()[0] - light->GetPosition()[0];
            lightDirection[i][1] = light->GetFocalPoint()[1] - light->GetPosition()[1];
            lightDirection[i][2] = light->GetFocalPoint()[2] - light->GetPosition()[2];
            lightDirection[i][3] = std::abs(std::cos(vtkMath::RadiansFromDegrees(light->GetConeAngle())));
        }
        else
        {
            lightType[i] = 0;
        }
    }

    helper.Program->SetUniform3fv("lightPosition", 16,
        lightPosition);
    helper.Program->SetUniform1iv("lightType", 16,
        lightType);
    helper.Program->SetUniform4fv("lightColor", 16,
        lightColor);
    helper.Program->SetUniform4fv("lightDirection", 16,
        lightDirection);

    // Per camera
    helper.Program->SetUniformMatrix("projectionMatrix",
        projectionMatrix);
    helper.Program->SetUniformMatrix("viewMatrix",
        viewMatrix);

    auto cameraPosition = camera->GetPosition();
    float cameraPos[3] = { (float)cameraPosition[0],
                           (float)cameraPosition[1],
                           (float)cameraPosition[2] };

    helper.Program->SetUniform3f("cameraPosition",
        cameraPos);
    helper.Program->SetUniformi("numLights", lights->GetNumberOfItems());

    // Per model
    helper.Program->SetUniformMatrix("modelMatrix",
        modelMatrix);
    helper.Program->SetUniformMatrix("normalMatrix",
        modelRotationMatrix);

    helper.AttributeUpdateTime.Modified();

    helper.Program->SetUniformf("emissivity", material->getEmissivity());
    helper.Program->SetUniformf("metalness", material->getMetalness());
    helper.Program->SetUniformf("roughness", material->getRoughness());

    unsigned int textureCount = 0;
    unsigned int currentTexture = 0;

    auto renderWindow = (vtkOpenGLRenderWindow*)(renderer->GetRenderWindow());

    if (this->GetOpenGLMode(actor->GetProperty()->GetRepresentation(), helper.PrimitiveType) == GL_TRIANGLES)
    {
        auto diffuseColorTemp = material->getColor();
        float diffuseColor[3] = {(float)diffuseColorTemp.r,
                                 (float)diffuseColorTemp.g,
                                 (float)diffuseColorTemp.b};
        helper.Program->SetUniform3f("diffuseColorUniform", diffuseColor);

        auto diffuseTexture = material->getTexture(Texture::DIFFUSE);
        if (diffuseTexture->getPath() != "" && textureCount < textures.size())
        {
            auto texture = (vtkOpenGLTexture*)textures[currentTexture];
            helper.Program->SetUniformi("diffuseTexture", texture->GetTextureUnit());
            renderWindow->DeactivateTexture(texture->GetTextureObject());
            currentTexture++;
        }

        auto cubemapTexture = material->getTexture(Texture::CUBEMAP);
        if (cubemapTexture->getPath() != "" && textureCount < textures.size())
        {
            auto texture = (vtkOpenGLTexture*)textures[currentTexture];
            helper.Program->SetUniformi("cubemapTexture", texture->GetTextureUnit());
            renderWindow->DeactivateTexture(texture->GetTextureObject());
            currentTexture++;
        }
    }
    else
    {
        auto debugColorTemp = material->getDebugColor();
        float debugColor[3] = {(float)debugColorTemp.r,
                               (float)debugColorTemp.g,
                               (float)debugColorTemp.b};
        helper.Program->SetUniform3f("debugColor", debugColor);
    }
}

void
VTKCustomPolyDataMapper::SetPropertyShaderParameters(
    vtkOpenGLHelper& vtkNotUsed(helper),
    vtkRenderer * vtkNotUsed(renderer),
    vtkActor * vtkNotUsed(actor))
{
}

void
VTKCustomPolyDataMapper::SetLightingShaderParameters(
    vtkOpenGLHelper& vtkNotUsed(helper),
    vtkRenderer * vtkNotUsed(renderer),
    vtkActor * vtkNotUsed(actor))
{
}

void
VTKCustomPolyDataMapper::SetCameraShaderParameters(
    vtkOpenGLHelper& vtkNotUsed(helper),
    vtkRenderer * vtkNotUsed(renderer),
    vtkActor * vtkNotUsed(actor))
{
}

void
VTKCustomPolyDataMapper::UpdateShaders(
    vtkOpenGLHelper& helper,
    vtkRenderer * renderer,
    vtkActor * actor)
{
    vtkOpenGLPolyDataMapper::UpdateShaders(helper, renderer, actor);
}

void
VTKCustomPolyDataMapper::loadShader(const std::string filename, std::string& source)
{
    std::stringstream stream;
    std::ifstream file(filename);
    stream << file.rdbuf();
    source = stream.str();
    file.close();
}

void
VTKCustomPolyDataMapper::setRenderMaterial(std::shared_ptr<RenderMaterial> renderMat)
{
    m_renderMaterial = renderMat;
}
}