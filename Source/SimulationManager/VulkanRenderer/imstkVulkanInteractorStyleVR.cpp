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

#include "imstkVulkanInteractorStyleVR.h"

#ifdef iMSTK_ENABLE_VR

#include "imstkSimulationManager.h"

namespace imstk
{
VulkanInteractorStyleVR::VulkanInteractorStyleVR()
{
}

void
VulkanInteractorStyleVR::initialize(std::shared_ptr<VulkanRenderer> renderer)
{
    m_renderer = renderer;

    // Load render models
    for (uint32_t i = 0; i < vr::VRRenderModels()->GetRenderModelCount(); i++)
    {
        // Extract model name
        auto nameLength = vr::VRRenderModels()->GetRenderModelName(i, nullptr, 0);
        std::vector<char> name(nameLength);
        vr::VRRenderModels()->GetRenderModelName(i, &name[0], nameLength);
        auto nameString = std::string(name.begin(), std::prev(name.end()));
        m_VRModels[nameString] = nullptr;

        // Load models and textures
        auto result = vr::VRRenderModels()->LoadRenderModel_Async(nameString.c_str(), &m_VRModels[nameString]);
        while (result == vr::VRRenderModelError_Loading)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            result = vr::VRRenderModels()->LoadRenderModel_Async(nameString.c_str(), &m_VRModels[nameString]);
        }

        if (result != vr::VRRenderModelError_InvalidModel)
        {
            auto textureID = m_VRModels[nameString]->diffuseTextureId;
            //vr::VRRenderModels()->LoadTexture_Async(textureID, &m_VRTextures[textureID]);
        }
    }
}

void
VulkanInteractorStyleVR::updateVRDevices()
{
    // Iterate over maximum number of devices
    for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; i++)
    {
        auto name = this->getDeviceID(i);
        if (name == "")
        {
            continue;
        }

        // If device hasn't been created, then create it
        if (m_devices.find(name) == m_devices.end())
        {
            m_devices[name] = std::unique_ptr<VulkanVRDevice>(new VulkanVRDevice());
            m_devices[name]->m_ID = name;
            m_devices[name]->m_type = m_renderer->m_VRSystem->GetTrackedDeviceClass(i);
            m_devices[name]->m_pose = m_devicePoses[i];
            m_devices[name]->m_renderModelName
                = this->getDeviceStringProperty(i, vr::Prop_RenderModelName_String);
            m_devices[name]->m_renderModel = m_VRModels[m_devices[name]->m_renderModelName];

            // Display device only if controller
            auto deviceClass = m_renderer->m_VRSystem->GetTrackedDeviceClass(i);
            if(i != vr::k_unTrackedDeviceIndex_Hmd
               && deviceClass == vr::TrackedDeviceClass::TrackedDeviceClass_Controller)
            {
                m_devices[name]->m_rendered = true;
                this->addVisualVRObject(m_devices[name]);
            }
        }

        m_devices[name]->m_pose = m_devicePoses[i];

        if (!m_devices[name]->m_rendered)
        {
            continue;
        }

        auto deviceMatrix = m_devices[name]->m_pose.mDeviceToAbsoluteTracking.m;

        glm::mat4 matrix(
            deviceMatrix[0][0], deviceMatrix[1][0], deviceMatrix[2][0], 0.0f,
            deviceMatrix[0][1], deviceMatrix[1][1], deviceMatrix[2][1], 0.0f,
            deviceMatrix[0][2], deviceMatrix[1][2], deviceMatrix[2][2], 0.0f,
            deviceMatrix[0][3], deviceMatrix[1][3], deviceMatrix[2][3], 1.0f);

        glm::vec3 scale;
        glm::quat orientation;
        glm::vec3 translation;
        glm::vec3 skew; // ignore
        glm::vec4 perspective; // ignore

        glm::decompose(matrix, scale, orientation, translation, skew, perspective);
        orientation = glm::conjugate(orientation);

        // Convert to iMSTK data formats
        imstk::Quatd orientationQuaternion(orientation.w, orientation.x, orientation.y, orientation.z);

        m_devices[name]->m_visualModel->getGeometry()->setTranslation(translation.x, translation.y, translation.z);
        m_devices[name]->m_visualModel->getGeometry()->setRotation(orientationQuaternion);
        m_devices[name]->m_visualModel->getGeometry()->setScaling(scale.x);
    }
}

void
VulkanInteractorStyleVR::addVisualVRObject(std::unique_ptr<VulkanVRDevice>& device)
{
    auto surfaceMesh = std::make_shared<SurfaceMesh>();
    auto visualModel = std::make_shared<VisualModel>(surfaceMesh);
    auto renderModel = device->m_renderModel;
    auto vertexData = renderModel->rVertexData;
    auto triangleData = renderModel->rIndexData;
    auto numVertices = renderModel->unVertexCount;
    auto numTriangles = renderModel->unTriangleCount;

    StdVectorOfVec3d vertices(numVertices);
    StdVectorOfVec3d normals(numVertices);
    std::vector<SurfaceMesh::TriangleArray> triangles(numTriangles);

    // Extract positions and vertices
    for (unsigned int i = 0; i < numVertices; i++)
    {
        auto position = vertexData[i].vPosition.v;
        auto normal = vertexData[i].vNormal.v;

        vertices[i] = Vec3d(position[0], position[1], position[2]);
        normals[i] = Vec3d(normal[0], normal[1], normal[2]);
    }

    // Extract triangles
    for (unsigned int i = 0; i < numTriangles; i++)
    {
        triangles[i][0] = triangleData[i * 3];
        triangles[i][1] = triangleData[i * 3 + 1];
        triangles[i][2] = triangleData[i * 3 + 2];
    }

    surfaceMesh->initialize(vertices, triangles);

    // Extract UV coordinates
    StdVectorOfVectorf UVs(numVertices);
    for (unsigned int i = 0; i < numVertices; i++)
    {
        auto texCoord = vertexData[i].rfTextureCoord;

        Vectorf UV(2);
        UV[0] = texCoord[0];
        UV[1] = texCoord[1];
        UVs[i] = UV;
    }
    surfaceMesh->setDefaultTCoords("tCoords");
    surfaceMesh->setPointDataArray("tCoords", UVs);

    // Add material
    auto renderMaterial = std::make_shared<RenderMaterial>();
    renderMaterial->setColor(Color(0.1, 0.1, 0.1, 1.0));
    visualModel->setRenderMaterial(renderMaterial);

    device->m_visualModel = visualModel;
    m_renderer->loadVisualModel(visualModel, SceneObject::Type::Visual);
}

const std::string
VulkanInteractorStyleVR::getDeviceID(const uint32_t index)
{
    auto modelNumber = this->getDeviceStringProperty(index, vr::Prop_ModelNumber_String);
    auto serialNumber = this->getDeviceStringProperty(index, vr::Prop_SerialNumber_String);

    if (modelNumber.length() == 0 && serialNumber.length() == 0)
    {
        return "";
    }

    return modelNumber + ": " + serialNumber;
}

const std::string
VulkanInteractorStyleVR::getDeviceStringProperty(
    const uint32_t index,
    vr::ETrackedDeviceProperty stringProperty)
{
    // Get length of returned string value
    uint32_t stringPropertyLength = m_renderer->m_VRSystem->GetStringTrackedDeviceProperty(
        index,
        stringProperty,
        nullptr,
        0);

    if (stringPropertyLength == 0)
    {
        return std::string("");
    }

    // Create temporary char array
    std::vector<char> stringPropertyTemp(stringPropertyLength);

    // Fill in temporary char array
    m_renderer->m_VRSystem->GetStringTrackedDeviceProperty(
        index,
        stringProperty,
        &stringPropertyTemp[0],
        stringPropertyLength);

    return std::string(&stringPropertyTemp[0]);
}

void
VulkanInteractorStyleVR::OnTimer()
{
    vr::VRCompositor()->WaitGetPoses(m_devicePoses, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
    auto scene = m_simManager->getActiveScene();
    auto indexHMD = vr::k_unTrackedDeviceIndex_Hmd;
    auto nameHMD = this->getDeviceID(indexHMD);

    this->updateVRDevices();

    // Handle HDM first
    auto HMDMatrixNative = m_devices[nameHMD]->m_pose.mDeviceToAbsoluteTracking.m;

    glm::mat4 HMDMatrix(
        HMDMatrixNative[0][0], HMDMatrixNative[1][0], HMDMatrixNative[2][0], 0.0f,
        HMDMatrixNative[0][1], HMDMatrixNative[1][1], HMDMatrixNative[2][1], 0.0f,
        HMDMatrixNative[0][2], HMDMatrixNative[1][2], HMDMatrixNative[2][2], 0.0f,
        HMDMatrixNative[0][3], HMDMatrixNative[1][3], HMDMatrixNative[2][3], 1.0f);

    auto projectionMatrixLeftNative = m_renderer->m_VRSystem->GetProjectionMatrix(vr::Eye_Left, m_renderer->m_nearPlane, m_renderer->m_farPlane).m;
    auto projectionMatrixRightNative = m_renderer->m_VRSystem->GetProjectionMatrix(vr::Eye_Right, m_renderer->m_nearPlane, m_renderer->m_farPlane).m;

    glm::mat4 projectionMatrixLeft(
        projectionMatrixLeftNative[0][0], projectionMatrixLeftNative[1][0], projectionMatrixLeftNative[2][0], projectionMatrixLeftNative[3][0],
        projectionMatrixLeftNative[0][1], projectionMatrixLeftNative[1][1], projectionMatrixLeftNative[2][1], projectionMatrixLeftNative[3][1],
        projectionMatrixLeftNative[0][2], projectionMatrixLeftNative[1][2], projectionMatrixLeftNative[2][2], projectionMatrixLeftNative[3][2],
        projectionMatrixLeftNative[0][3], projectionMatrixLeftNative[1][3], projectionMatrixLeftNative[2][3], projectionMatrixLeftNative[3][3]);

    glm::mat4 projectionMatrixRight(
        projectionMatrixRightNative[0][0], projectionMatrixRightNative[1][0], projectionMatrixRightNative[2][0], projectionMatrixRightNative[3][0],
        projectionMatrixRightNative[0][1], projectionMatrixRightNative[1][1], projectionMatrixRightNative[2][1], projectionMatrixRightNative[3][1],
        projectionMatrixRightNative[0][2], projectionMatrixRightNative[1][2], projectionMatrixRightNative[2][2], projectionMatrixRightNative[3][2],
        projectionMatrixRightNative[0][3], projectionMatrixRightNative[1][3], projectionMatrixRightNative[2][3], projectionMatrixRightNative[3][3]);

    auto eyeMatrixLeftNative = m_renderer->m_VRSystem->GetEyeToHeadTransform(vr::Eye_Left).m;
    auto eyeMatrixRightNative = m_renderer->m_VRSystem->GetEyeToHeadTransform(vr::Eye_Right).m;

    glm::mat4 eyeMatrixLeftOffset(
        eyeMatrixLeftNative[0][0], eyeMatrixLeftNative[1][0], eyeMatrixLeftNative[2][0], 0.0f,
        eyeMatrixLeftNative[0][1], eyeMatrixLeftNative[1][1], eyeMatrixLeftNative[2][1], 0.0f,
        eyeMatrixLeftNative[0][2], eyeMatrixLeftNative[1][2], eyeMatrixLeftNative[2][2], 0.0f,
        eyeMatrixLeftNative[0][3], eyeMatrixLeftNative[1][3], eyeMatrixLeftNative[2][3], 1.0f);

    glm::mat4 eyeMatrixRightOffset(
        eyeMatrixRightNative[0][0], eyeMatrixRightNative[1][0], eyeMatrixRightNative[2][0], 0.0f,
        eyeMatrixRightNative[0][1], eyeMatrixRightNative[1][1], eyeMatrixRightNative[2][1], 0.0f,
        eyeMatrixRightNative[0][2], eyeMatrixRightNative[1][2], eyeMatrixRightNative[2][2], 0.0f,
        eyeMatrixRightNative[0][3], eyeMatrixRightNative[1][3], eyeMatrixRightNative[2][3], 1.0f);

    auto eyeMatrixLeft = HMDMatrix * eyeMatrixLeftOffset;
    auto eyeMatrixRight = HMDMatrix * eyeMatrixRightOffset;

    m_renderer->m_viewMatrices[0] = glm::inverse(eyeMatrixLeft);
    m_renderer->m_viewMatrices[1] = glm::inverse(eyeMatrixRight);

    m_renderer->m_projectionMatrices[0] = projectionMatrixLeft;
    m_renderer->m_projectionMatrices[1] = projectionMatrixRight;

    m_renderer->m_cameraPositions[0] = glm::vec4(
        eyeMatrixLeft[3][0],
        eyeMatrixLeft[3][1],
        eyeMatrixLeft[3][2],
        eyeMatrixLeft[3][3]);
    m_renderer->m_cameraPositions[1] = glm::vec4(
        eyeMatrixRight[3][0],
        eyeMatrixRight[3][1],
        eyeMatrixRight[3][2],
        eyeMatrixRight[3][3]);

    auto cameraPosition = glm::vec4(HMDMatrix[3]);
    m_renderer->m_scene->getCamera()->setPosition(cameraPosition.x, cameraPosition.y, cameraPosition.z);
}
}

#endif