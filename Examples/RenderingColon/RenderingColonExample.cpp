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

#include "imstkCamera.h"
#include "imstkImageData.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkSpotLight.h"
#include "imstkSurfaceMesh.h"
#include "imstkVecDataArray.h"
#include "imstkVisualModel.h"
#include "imstkVTKRenderer.h"
#include "imstkVTKViewer.h"

#include <vtkImageData.h>
#include <vtkJPEGReader.h>
#include <vtkOpenGLRenderer.h>
#include <vtkSkybox.h>
#include <vtkTexture.h>

using namespace imstk;

///
/// \brief Returns interpolated point between p1 and p2, given
/// a set of 4 vertices on the spline
///
static Vec3d
catmullRom(
    const Vec3d& p0, const Vec3d& p1,
    const Vec3d& p2, const Vec3d& p3,
    const double t)
{
    const double t2 = t * t;
    const double t3 = t2 * t;
    return p1 + 0.5 * ((-p0 + p2) * t
                       + (2.0 * p0 - 5.0 * p1 + 4.0 * p2 - p3) * t2
                       + (-p0 + 3.0 * p1 - 3.0 * p2 + p3) * t3);
}

///
/// \brief Gets position on the line mesh interpreted as a piecewise spline.
/// The spline is split up per 4 verts
///
static Vec3d
getSplinePositionFromLineMesh(double dist, std::shared_ptr<LineMesh> lineMesh)
{
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = lineMesh->getVertexPositions();
    const VecDataArray<double, 3>&           vertices    = *verticesPtr;
    CHECK(vertices.size() >= 2) << "Must have at least 2 vertices";
    auto                     vertexDistPtr = std::dynamic_pointer_cast<DataArray<double>>(lineMesh->getVertexAttribute("distances"));
    const DataArray<double>& vertexDist    = *vertexDistPtr;
    const double             startLength   = vertexDist[0];
    const double             endLength     = vertexDist[vertexDist.size() - 1];

    // Extrapolates
    if (dist <= startLength)
    {
        const Vec3d m = (vertices[1] - vertices[0]).normalized();
        return vertices[0] + m * -dist;
    }
    if (dist >= endLength)
    {
        const Vec3d m = (vertices[vertices.size() - 1] - vertices[vertices.size() - 2]).normalized();
        return vertices[vertices.size() - 1] + m * dist;
    }

    // Find the corresponding segment we are in
    int j = 0;
    for (int i = 0; i < vertices.size() - 1; i++)
    {
        if (dist > vertexDist[i] && dist <= vertexDist[i + 1])
        {
            j = i;
            break;
        }
    }

    // If the user wants a position at either end make sure to clamp
    const int i0 = std::max(std::min(j - 1, vertices.size() - 1), 0);
    const int i1 = std::max(std::min(j, vertices.size() - 1), 0);
    const int i2 = std::max(std::min(j + 1, vertices.size() - 1), 0);
    const int i3 = std::max(std::min(j + 2, vertices.size() - 1), 0);

    // Get the 4 points on the line, with sample
    // point fractionally between b & c
    const Vec3d a = vertices[i0];
    const Vec3d b = vertices[i1];
    const Vec3d c = vertices[i2];
    const Vec3d d = vertices[i3];

    // Compute the fractional distance between b & c
    const double distb = vertexDist[j];
    const double distc = vertexDist[j + 1];
    const double frac  = (dist - distb) / (distc - distb);

    const Vec3d results = catmullRom(a, b, c, d, frac);
    return results;
}

///
/// \brief Demonstrates PBR rendering with clearcoat model
/// for an organ
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    imstkNew<Scene> scene("RenderingColon");
    auto            colonObject = std::make_shared<SceneObject>("colon");
    {
        imstkNew<RenderMaterial> colonMaterial;
        colonMaterial->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        colonMaterial->setShadingModel(RenderMaterial::ShadingModel::PBR);
        auto diffuseTexImg =
            MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/Organs/Colon/colon_BaseColor.png");
        colonMaterial->addTexture(std::make_shared<Texture>(diffuseTexImg, Texture::Type::Diffuse));
        auto normalTexImg =
            MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/Organs/Colon/colon_Normal.png");
        colonMaterial->addTexture(std::make_shared<Texture>(normalTexImg, Texture::Type::Normal));
        colonMaterial->setRecomputeVertexNormals(true);
        colonMaterial->setBackFaceCulling(true);
        colonMaterial->setMetalness(0.0);
        colonMaterial->setRoughness(0.26);
        colonMaterial->setNormalStrength(5.0);
        colonMaterial->setOcclusionStrength(0.0);

        colonMaterial->addTexture(std::make_shared<Texture>(normalTexImg, Texture::Type::CoatNormal));
        colonMaterial->setCoatRoughness(0.1);
        colonMaterial->setCoatStrength(1.0);
        colonMaterial->setCoatColor(Color::White);
        colonMaterial->setCoatIOR(3.0);
        colonMaterial->setBaseIOR(3.0);
        colonMaterial->setCoatNormalScale(0.5);
        colonMaterial->setEdgeTint(Color::White);

        auto surfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Organs/Colon/colon.obj");

        imstkNew<VisualModel> visualModel;
        visualModel->setGeometry(surfMesh);
        visualModel->setRenderMaterial(colonMaterial);
        colonObject->addVisualModel(visualModel);
    }
    scene->addSceneObject(colonObject);

    auto   colonMedialMesh = MeshIO::read<LineMesh>(iMSTK_DATA_ROOT "/Organs/Colon/colonMedialMesh.obj");
    double totalLength     = 0.0;
    {
        // Compute lengths to each vertex along the line
        std::shared_ptr<VecDataArray<double, 3>> verticesPtr   = colonMedialMesh->getVertexPositions();
        const VecDataArray<double, 3>&           vertices      = *verticesPtr;
        auto                                     vertexDistPtr = std::make_shared<DataArray<double>>(verticesPtr->size());
        DataArray<double>&                       vertexDist    = *vertexDistPtr;
        colonMedialMesh->setVertexScalars("distances", vertexDistPtr);
        vertexDist[0] = 0.0;
        for (int i = 1; i < vertices.size(); i++)
        {
            const double length = (vertices[i] - vertices[i - 1]).norm();
            vertexDist[i] = vertexDist[i - 1] + length;
        }
        totalLength = vertexDist[vertexDist.size() - 1];
    }

    // Lights
    // Here we use a falloff on the light (via quadratic function)
    imstkNew<SpotLight> light;
    light->setSpotAngle(40.0);
    light->setAttenuationValues(3000.0, 1.0, 0.01);
    light->setIntensity(10.0);
    scene->addLight("light", light);

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setBackgroundColors(Color::Black);
        // Enable SSAO
        Vec3d l, u;
        scene->computeBoundingBox(l, u);
        const double sceneSize = (u - l).norm();

        auto renderConfig = std::make_shared<RendererConfig>();
        renderConfig->m_ssaoConfig.m_enableSSAO = true;
        renderConfig->m_ssaoConfig.m_SSAOBlur   = true;
        renderConfig->m_ssaoConfig.m_SSAORadius = 50.0 * sceneSize;
        renderConfig->m_ssaoConfig.m_SSAOBias   = 0.03 * sceneSize;
        renderConfig->m_ssaoConfig.m_KernelSize = 128;
        viewer->getActiveRenderer()->setConfig(renderConfig);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>();
            keyControl->setDevice(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        double                  t   = 0.0;
        std::shared_ptr<Camera> cam = scene->getActiveCamera();
        {
            // Initialize the camera
            const Vec3d eyePos  = getSplinePositionFromLineMesh(0.0, colonMedialMesh);
            const Vec3d focalPt = getSplinePositionFromLineMesh(0.07, colonMedialMesh);
            cam->setPosition(eyePos);
            cam->setFocalPoint(focalPt);
            light->setPosition(eyePos);
            light->setFocalPoint(focalPt);
        }

        // Advance the camera along the line
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                t += sceneManager->getDt();

                const double velocity = 0.1;
                const double dist     = std::min(t * velocity, totalLength);
                const Vec3d eyePos    = getSplinePositionFromLineMesh(dist, colonMedialMesh);
                const Vec3d focalPt   = getSplinePositionFromLineMesh(dist + 0.07, colonMedialMesh);

                cam->setPosition(eyePos);
                cam->setFocalPoint(focalPt);
                light->setPosition(eyePos);
                light->setFocalPoint(focalPt);
            });

        connect<Event>(driver, &SimulationManager::starting, [&](Event*)
            {
                vtkSmartPointer<vtkRenderer> ren = std::dynamic_pointer_cast<VTKRenderer>(viewer->getActiveRenderer())->getVtkRenderer();
                vtkSmartPointer<vtkOpenGLRenderer> oRen = vtkOpenGLRenderer::SafeDownCast(ren);
                vtkNew<vtkJPEGReader> reader;
                reader->SetFileName(iMSTK_DATA_ROOT "/Organs/Colon/colon_irradiance_environment_map.jpg");
                reader->Update();

                vtkNew<vtkTexture> texture;
                // Enable mipmapping to handle HDR image
                texture->MipmapOn();
                texture->InterpolateOn();
                texture->SetInputData(reader->GetOutput());
                texture->SetColorModeToDirectScalars();
                texture->SetCubeMap(false);
                texture->Update();

                /* vtkNew<vtkSkybox> skybox;
                 skybox->SetTexture(texture);
                 ren->AddActor(skybox);*/

                ren->AutomaticLightCreationOff();
                oRen->UseSphericalHarmonicsOff();
                ren->UseImageBasedLightingOn();
                ren->SetEnvironmentTexture(texture);
            });

        driver->start();
    }

    return 0;
}
