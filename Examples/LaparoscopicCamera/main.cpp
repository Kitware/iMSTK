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

//---------------------------------------------------------------------------
//
// Camera navigation application build on iMSTK
//
//---------------------------------------------------------------------------

#include <memory>
#include <string>
#include <cmath>

// Core iMSTK includes
#include "SimulationManager/SDK.h"
#include "SceneModels/StaticSceneObject.h"
#include "Devices/VRPNForceDevice.h"
#include "Devices/VRPNDeviceServer.h"
#include "VirtualTools/ToolCoupler.h"
#include "VirtualTools/LaparoscopicCameraController.h"
#include "Collision/PlaneCollisionModel.h"
#include "Collision/MeshCollisionModel.h"
#include "IO/InitIO.h"
#include "Rendering/InitVTKRendering.h"
#include "Rendering/VTKViewer.h"

// VTK includes
#include <vtkNew.h>
// Screenshot
#include <vtkObjectFactory.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
// 2d overlay
#include <vtkPNGReader.h>
#include <vtkImageResize.h>
#include <vtkImageTranslateExtent.h>
#include <vtkImageMapper.h>
#include <vtkActor2D.h>
#include <vtkRendererCollection.h>
#include <vtkRenderer.h>

const imstk::DeviceType controllerType = imstk::DeviceType::NAVIGATOR_3DCONNEXION;
const float X = 8;
const float Y = 6;
const float Z = 6;
const float pY = 0.25;
const float pZ = 0.25;
const double radius = 3.0;
const double scaling = 0.15;
const double planeWidth = 10;
const int overlaySize = 400;

// Define custom interaction style
class ScreenCaptureInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
    struct screenShotData
    {
        vtkNew<vtkWindowToImageFilter> windowToImageFilter;
        vtkNew<vtkPNGWriter> pngWriter;
        bool triggerScreenCapture;
        int screenShotNumber;

        screenShotData() : triggerScreenCapture(false), screenShotNumber(0)
        {
            windowToImageFilter->SetMagnification(1);

            windowToImageFilter->SetInputBufferTypeToRGB();

            windowToImageFilter->ReadFrontBufferOff();

            windowToImageFilter->Update();

            pngWriter->SetInputConnection(windowToImageFilter->GetOutputPort());
        };

        ~screenShotData(){};
    };

    static ScreenCaptureInteractorStyle* New();
    vtkTypeMacro(ScreenCaptureInteractorStyle, vtkInteractorStyleTrackballCamera);

    virtual void OnKeyPress()
    {
        // Get the key press
        vtkRenderWindowInteractor *rwi = this->Interactor;
        std::string keySym = rwi->GetKeySym();

        // Capture the screen
        if (keySym == "space")
        {
            this->screenCaptureData->triggerScreenCapture = true;

            if (this->screenCaptureData != nullptr)
            {
                if (this->screenCaptureData->triggerScreenCapture)
                {
                    this->screenCaptureData->windowToImageFilter->Modified();

                    std::string captureName = "screenShot-"
                        + std::to_string(this->screenCaptureData->screenShotNumber)
                        + ".png";

                    this->screenCaptureData->pngWriter->SetFileName(
                        captureName.data());

                    this->screenCaptureData->pngWriter->Write();

                    std::cout <<
                        "Screen shot "<<
                        this->screenCaptureData->screenShotNumber <<" saved.\n";

                    this->screenCaptureData->screenShotNumber++;
                    this->screenCaptureData->triggerScreenCapture = false;
                }
            }
        }

        // Forward events
        vtkInteractorStyleTrackballCamera::OnKeyPress();
    }

    void initialize(vtkWindow* rw)
    {
        this->screenCaptureData = std::make_shared<screenShotData>();
        this->screenCaptureData->windowToImageFilter->SetInput(rw);
    }

private:

    std::shared_ptr<screenShotData> screenCaptureData;
};
vtkStandardNewMacro(ScreenCaptureInteractorStyle);

///
/// \brief Create camera navigation scene
///
bool createCameraNavigationScene(
    std::shared_ptr<imstk::Scene> scene,
    const char* targetFileName)
{

    //-------------------------------------------------------
    // Lights
    //-------------------------------------------------------
    auto light1 = imstk::Light::getDefaultLighting();
    light1->lightPos.setPosition(imstk::Vec3d(-25.0, 10.0, 10.0));
    scene->addLight(light1);

    auto light2 = imstk::Light::getDefaultLighting();
    light2->lightPos.setPosition(imstk::Vec3d(25.0, 10.0, 10.0));
    scene->addLight(light2);

    auto light3 = imstk::Light::getDefaultLighting();
    light3->lightPos.setPosition(imstk::Vec3d(0.0, 25.0, 0.0));
    scene->addLight(light3);

    //-------------------------------------------------------
    // Plane
    //-------------------------------------------------------

    imstk::Color planeColor(0.4, 0.4, 0.4, 1.0);

    auto planeRenderDetail = std::make_shared<imstk::RenderDetail>(IMSTK_RENDER_NORMALS);
    planeRenderDetail->setAmbientColor(planeColor);
    planeRenderDetail->setDiffuseColor(planeColor);
    planeRenderDetail->setSpecularColor(planeColor);
    planeRenderDetail->setShininess(50.0);

    auto planeModel = std::make_shared<imstk::PlaneCollisionModel>(
          imstk::Vec3d(0.0, -0.01, 0.0),
          imstk::Vec3d(0.0, 1.0, 0.0));
    planeModel->getPlaneModel()->setRenderDetail(planeRenderDetail);
    planeModel->getPlaneModel()->setWidth(planeWidth);

    auto planeObject = std::make_shared<imstk::StaticSceneObject>();
    planeObject->setModel(planeModel);

    scene->addSceneObject(planeObject);

    //-------------------------------------------------------
    // Blocks
    //-------------------------------------------------------

    imstk::Color meshColor(0.32, 0.32, 0.32, 1.0);

    auto targetRenderDetail = std::make_shared<imstk::RenderDetail>(IMSTK_RENDER_TEXTURE);
    targetRenderDetail->addTexture("target", targetFileName, "", "");

    auto blockRenderDetail = std::make_shared<imstk::RenderDetail>(IMSTK_RENDER_NORMALS);
    blockRenderDetail->setAmbientColor(meshColor);
    blockRenderDetail->setDiffuseColor(meshColor);
    blockRenderDetail->setSpecularColor(meshColor);
    blockRenderDetail->setShininess(100.0);

    for (int i = 0; i < 6; i++)
    {
        // transformations
        Eigen::UniformScaling<double> s(scaling);
        Eigen::Translation3d t1(0, 0, -radius);
        Eigen::Translation3d t2(0, 0, -radius+0.01);
        Eigen::Quaterniond q(cos(i*22.0/42), 0, sin(i*22.0/42), 0);
        q.normalize();

        // BLOCKS
        // surface mesh
        std::vector<imstk::Vec3d> blockPts = {imstk::Vec3d( X/2, 0, -Z/2), imstk::Vec3d( X/2, 0, Z/2),
                                             imstk::Vec3d( -X/2, 0, Z/2), imstk::Vec3d( -X/2, 0, -Z/2),
                                             imstk::Vec3d( -X/2, Y, -Z/2), imstk::Vec3d( X/2, Y, -Z/2),
                                             imstk::Vec3d( -X/2, Y, Z*(pZ-0.5)), imstk::Vec3d( X/2, Y, Z*(pZ-0.5)),
                                             imstk::Vec3d( -X/2, Y*pY, Z/2), imstk::Vec3d( X/2, Y*pY, Z/2)};
        std::vector<std::array<size_t,3>> blockTriangles = {{{ 0, 1, 2}}, {{ 0, 2, 3}},
                                                            {{ 0, 3, 4}}, {{ 5, 0, 4}},
                                                            {{ 5, 4, 6}}, {{ 7, 5, 6}},
                                                            {{ 6, 8, 9}}, {{ 6, 9, 7}},
                                                            {{ 2, 1, 9}}, {{ 8, 2, 9}},
                                                            {{ 3, 6, 4}}, {{ 3, 8, 6}},
                                                            {{ 3, 2, 8}}, {{ 5, 7, 0}},
                                                            {{ 7, 9, 0}}, {{ 9, 1, 0}}};
        auto blockMesh = std::make_shared<imstk::SurfaceMesh>();
        blockMesh->setVertices(blockPts);
        blockMesh->setTriangles(blockTriangles);

        // model
        auto blockModel = std::make_shared<imstk::MeshCollisionModel>();
        blockModel->setMesh(blockMesh);
        blockModel->setRenderDetail(blockRenderDetail);
        blockModel->getMesh()->scale(s);
        blockModel->getMesh()->translate(t1);
        blockModel->getMesh()->rotate(q);

        // object
        auto blockObject = std::make_shared<imstk::StaticSceneObject>();
        blockObject->setModel(blockModel);
        scene->addSceneObject(blockObject);

        // TARGETS
        // surface mesh
        imstk::Vec3d topLeftEdge(-X/2, Y, Z*(pZ-0.5));
        imstk::Vec3d topRightEdge(X/2, Y, Z*(pZ-0.5));
        imstk::Vec3d bottomLeftEdge(-X/2, Y*pY, Z/2);
        imstk::Vec3d bottomRightEdge(X/2, Y*pY, Z/2);
        std::vector<imstk::Vec3d> points = {topLeftEdge, topRightEdge, bottomLeftEdge, bottomRightEdge};
        std::array<size_t, 3> tri1 = {{ 0, 1, 2}};
        std::array<size_t, 3> tri2 = {{ 1, 2, 3}};
        std::vector<std::array<size_t,3>> triArray = {tri1, tri2};
        auto surfaceMesh = std::make_shared<imstk::SurfaceMesh>();
        surfaceMesh->setVertices(points);
        surfaceMesh->setTriangles(triArray);

        // Texture Coordinates
        double height = (bottomLeftEdge - topLeftEdge).norm();
        double width = (bottomLeftEdge - bottomRightEdge).norm();
        double dist = std::min(height, width);
        double dx = 0.0, dy = 0.0;
        if(dist == height)
        {
            double x_a = bottomLeftEdge[0];
            double x_b = bottomRightEdge[0];
            dx = std::abs((x_b-x_a-dist)/(2*dist));
        }
        else
        {
            double y_a = bottomLeftEdge[1];
            double y_b = topLeftEdge[1];
            dy = std::abs((y_b-y_a-dist)/(2*dist));
        }
        surfaceMesh->addTextureCoordinate(1.0+dx,-dy);
        surfaceMesh->addTextureCoordinate(-dx,-dy);
        surfaceMesh->addTextureCoordinate(1.0+dx,1.0+dy);
        surfaceMesh->addTextureCoordinate(-dx,1.0+dy);

        // model
        auto targetModel = std::make_shared<imstk::MeshCollisionModel>();
        targetModel->setMesh(surfaceMesh);
        targetModel->setRenderDetail(targetRenderDetail);
        targetModel->getMesh()->scale(s);
        targetModel->getMesh()->translate(t2);
        targetModel->getMesh()->rotate(q);

        // object
        auto targetObject = std::make_shared<imstk::StaticSceneObject>();
        targetObject->setModel(targetModel);
        scene->addSceneObject(targetObject);
    }
    return true;
}

///
///	 \brief Add a 2D overlay of target markers on a 3D scene
///
void add2DOverlay(std::shared_ptr<imstk::VTKViewer> vtkViewer,
                  const char* fileName)
{
    // Read the image
    vtkNew<vtkPNGReader> reader;
    reader->SetFileName(fileName);
    reader->Update();

    int dim[3] = {overlaySize, overlaySize, 1};

    // Resize image
    vtkNew<vtkImageResize> resize;
    resize->SetInputConnection(reader->GetOutputPort());
    resize->SetOutputDimensions(dim);

    // Translate image extent (origin to its center)
    vtkNew<vtkImageTranslateExtent> translateExtent;
    translateExtent->SetInputConnection(resize->GetOutputPort());
    translateExtent->SetTranslation(-dim[0]/2,-dim[1]/2,0);

    // Mapper
    vtkNew<vtkImageMapper> imageMapper;
    imageMapper->SetInputConnection(translateExtent->GetOutputPort());
    imageMapper->SetColorWindow(255);
    imageMapper->SetColorLevel(127);

    // Actor
    vtkNew<vtkActor2D> imageActor;
    imageActor->SetMapper(imageMapper.GetPointer());
    imageActor->GetPositionCoordinate()->SetCoordinateSystemToNormalizedDisplay();
    imageActor->SetPosition(0.5, 0.5);

    // Renderer
    vtkRenderer* rendererVtk = vtkViewer->getVtkRenderWindow()->GetRenderers()->GetFirstRenderer();
    rendererVtk->AddActor2D(imageActor.GetPointer());
}

///
/// \brief Create a Laparoscopic camera controller and connect
/// it to the vtk viewer
///
int addCameraController(std::shared_ptr<imstk::SDK> sdk)
{
    std::string deviceURL;

    // Set Camera Client
    switch( controllerType )
    {
        case imstk::DeviceType::NAVIGATOR_3DCONNEXION:
        {
            deviceURL = "navigator@localhost";
        break;
        }
        case imstk::DeviceType::SPACE_EXPLORER_3DCONNEXION:
        {
            deviceURL = "device0@localhost";
        break;
        }
        case imstk::DeviceType::PHANTOM_OMNI:
        {
            deviceURL = "Phantom0@localhost"; //"Phantom@10.171.2.217"
        break;
        }
        case imstk::DeviceType::XKEYS_XK3:
        {
            deviceURL = "xkeys0@localhost";
        break;
        }
        case imstk::DeviceType::OSVR_HDK:
        {
            deviceURL = "com_osvr_Multiserver/OSVRHackerDevKit0@localhost";
        break;
        }
        case imstk::DeviceType::RAZER_HYDRA:
        {
            deviceURL = "razer@localhost";
            break;
        }
        default:
        {
            std::cerr << "Unknown controller type" << std::endl;
        }
    }
    auto camClient = std::make_shared<imstk::VRPNDeviceClient>(controllerType, deviceURL);
    std::cout<<"Device URL = "<<camClient->getDeviceURL()<<std::endl;

    // Set server
    std::shared_ptr<imstk::VRPNDeviceServer> server = std::make_shared<imstk::VRPNDeviceServer>();
    server->addDeviceClient(camClient);

    // Get vtkCamera
    auto viewer = sdk->getViewerInstance();
    auto vtkViewer = std::static_pointer_cast<imstk::VTKViewer>(viewer);
    vtkCamera* cam = vtkViewer->getVtkCamera();
    cam->SetViewAngle(80.0);

    // Set Camera Controller
    auto camController = std::make_shared<imstk::LaparoscopicCameraController>(camClient);
    camController->setScalingFactor(8);
    camController->setCamera(cam);

    // Register modules
    sdk->registerModule(server);
    sdk->registerModule(camClient);
    sdk->registerModule(camController);
    return 0;
}

int main()
{
    imstk::InitVTKRendering();
    imstk::InitIODelegates();

    //-------------------------------------------------------
    // Set up the viewer
    //-------------------------------------------------------
    std::shared_ptr<imstk::SDK> sdk = imstk::SDK::createStandardSDK();
    std::shared_ptr<imstk::ViewerBase> viewer = sdk->getViewerInstance();
    std::shared_ptr<imstk::VTKViewer> vtkViewer = std::static_pointer_cast<imstk::VTKViewer>(viewer);

    // Set Render details
    viewer->setViewerRenderDetail(
        IMSTK_VIEWERRENDER_GLOBALAXIS
        | IMSTK_VIEWERRENDER_FADEBACKGROUND
        | IMSTK_DISABLE_MOUSE_INTERACTION
        );

    //-------------------------------------------------------
    // Set up the scene
    //-------------------------------------------------------
    std::shared_ptr<imstk::Scene> scene = sdk->getScene(0);
    viewer->registerScene(scene, imstk::IMSTK_RENDERTARGET_SCREEN, "Collision pipeline demo");

    // Create camera navigation scene
    createCameraNavigationScene(scene, "./CameraNavAppData/target.png");

    //-------------------------------------------------------
    // Enable screenshot capture
    //-------------------------------------------------------
    // NOTE : Needs to be done before the viewer initialisation
    // not to erase the changes on the observers made to the
    // interactor in vtkViewer::addRenderer()
    vtkNew<ScreenCaptureInteractorStyle> style;
    style->initialize(vtkViewer->getVtkRenderWindow());
    vtkViewer->getVtkRenderWindowInteractor()->SetInteractorStyle(style.GetPointer());
    style->SetCurrentRenderer(vtkViewer->getVtkRenderer());

    //-------------------------------------------------------
    // Initialize viewer with scene objects
    //-------------------------------------------------------
    // NOTE : Needs to be done before VTK Add ons since
    // init create the needed renderer in the VTKViewer
    viewer->init();

    //-------------------------------------------------------
    // Add ons (VTK)
    //-------------------------------------------------------
    // Add a camera controller
    addCameraController(sdk);

    // Add a 2D overlay on the 3D scene
    add2DOverlay(vtkViewer,"./CameraNavAppData/viewfinder.png");

    //-------------------------------------------------------
    // Start
    //-------------------------------------------------------
    // Run the SDK
    sdk->run();

    // Cleanup
    sdk->releaseScene(scene);

    return 0;
}
