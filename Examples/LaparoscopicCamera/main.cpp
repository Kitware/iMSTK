//---------------------------------------------------------------------------
//
// Camera navigation application build on iMSTK
//
//---------------------------------------------------------------------------

#include <memory>
#include <string>
#include <cmath>

// Core SimMedTK includes
#include "Core/SDK.h"
#include "Core/StaticSceneObject.h"
#include "Devices/VRPNForceDevice.h"
#include "Devices/VRPNDeviceServer.h"
#include "VirtualTools/ToolCoupler.h"
#include "VirtualTools/LaparoscopicCameraController.h"
#include "Simulators/DefaultSimulator.h"
#include "Collision/PlaneCollisionModel.h"
#include "Collision/MeshCollisionModel.h"
#include "IO/initIO.h"

// VTK includes
#include "VTKRendering/initVTKRendering.h"
#include "VTKRendering/VTKViewer.h"
#include "vtkWindowToImageFilter.h"
#include "vtkPNGWriter.h"
#include "vtkChartXY.h"
#include "vtkContextScene.h"
#include "vtkContextActor.h"
#include "vtkFloatArray.h"
#include "vtkPlotPoints.h"
#include "vtkTable.h"
#include "vtkAxis.h"
#include "vtkNew.h"

#define SPACE_EXPLORER_DEVICE true

///
/// \brief Create camera navigation scene
///
bool createCameraNavigationScene(
    std::shared_ptr<SDK> sdk,
    char* fileName,
    char* fileNameTarget)
{
    //-------------------------------------------------------
    // Create plane
    //-------------------------------------------------------
    auto staticSimulator = std::make_shared<DefaultSimulator>(sdk->getErrorLog());

    // create a static plane scene object of given normal and position
    auto staticObject = std::make_shared<StaticSceneObject>();

    auto plane = std::make_shared<PlaneCollisionModel>(
        core::Vec3d(0.0, -0.01, 0.0),
        core::Vec3d(0.0, 1.0, 0.0));
    plane->getPlaneModel()->setWidth(5);
    staticObject->setModel(plane);

    auto planeRendDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_NORMALS);

    planeRendDetail->setAmbientColor(Color(0.5, 0.5, 0.5, 1.0));
    planeRendDetail->setDiffuseColor(Color(0.5, 0.5, 0.5, 1.0));
    planeRendDetail->setSpecularColor(Color(0.4, 0.4, 0.4, 1.0));
    planeRendDetail->setShininess(50.0);

    plane->getPlaneModel()->setRenderDetail(planeRendDetail);

    sdk->getScene(0)->addSceneObject(staticObject);

    //-------------------------------------------------------
    // Create target blocks
    //-------------------------------------------------------

    Color grey(0.32, 0.32, 0.32, 1.0);

    auto meshRenderDetail = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_NORMALS);

    meshRenderDetail->setAmbientColor(grey);
    meshRenderDetail->setDiffuseColor(grey);
    meshRenderDetail->setSpecularColor(grey);
    meshRenderDetail->setShininess(100.0);

    double radius = 3.0;
    for (int i = 0; i < 6; i++)
    {
        auto staticBlock = std::make_shared<StaticSceneObject>();

        auto targetBlock = std::make_shared<MeshCollisionModel>();
        targetBlock->loadTriangleMesh(fileName);
        targetBlock->getMesh()->scale(Eigen::UniformScaling<double>(0.15));//0.2
        staticBlock->setModel(targetBlock);

        targetBlock->setRenderDetail(meshRenderDetail);

        sdk->getScene(0)->addSceneObject(staticBlock);

        targetBlock->getMesh()->translate(Eigen::Translation3d(0, 0, -radius));

        Eigen::Quaterniond q(cos(i*22.0/42), 0, sin(i*22.0/42), 0);
        q.normalize();
        targetBlock->getMesh()->rotate(q);
    }

    //-------------------------------------------------------
    // Create targets
    //-------------------------------------------------------
    auto meshRenderDetail2 = std::make_shared<RenderDetail>(SIMMEDTK_RENDER_NORMALS);

    meshRenderDetail2->setAmbientColor(Color(0.2, 0.2, 0.2, 1.0));
    meshRenderDetail2->setDiffuseColor(Color(0.8, 0.0, 0.0, 1.0));
    meshRenderDetail2->setSpecularColor(Color(0.4, 0.4, 0.4, 1.0));
    meshRenderDetail2->setShininess(100.0);

    for (int i = 0; i < 6; i++)
    {
        auto staticTarget = std::make_shared<StaticSceneObject>();

        auto targetModel = std::make_shared<MeshCollisionModel>();
        targetModel->loadTriangleMesh(fileNameTarget);
        targetModel->getMesh()->scale(Eigen::UniformScaling<double>(0.1));//0.2
        targetModel->getMesh()->scale(Eigen::UniformScaling<double>(0.15));//0.2
        staticTarget->setModel(targetModel);

        targetModel->setRenderDetail(meshRenderDetail2);

        sdk->getScene(0)->addSceneObject(staticTarget);

        targetModel->getMesh()->translate(Eigen::Translation3d(0, 0, -radius));

        Eigen::Quaterniond q(cos(i*22.0 / 42), 0, sin(i*22.0 / 42), 0);
        q.normalize();
        targetModel->getMesh()->rotate(q);
    }

    return true;
}

///
///	 \brief Add a 2D overlay of target markers on a 3D scene
///
void add2DOverlay(std::shared_ptr<VTKViewer> vtkViewer)
{
    vtkNew<vtkChartXY> chart;
    vtkNew<vtkContextScene> chartScene;
    vtkNew<vtkContextActor> chartActor;

    // configure the chart
    chart->SetAutoSize(true);
    chart->SetSize(vtkRectf(0.0, 0.0, 300, 200));

    chart->GetAxis(0)->SetGridVisible(false);
    chart->GetAxis(1)->SetGridVisible(false);

    chart->GetAxis(0)->SetAxisVisible(false);
    chart->GetAxis(1)->SetAxisVisible(false);

    chart->GetAxis(0)->SetTicksVisible(false);
    chart->GetAxis(1)->SetTicksVisible(false);

    chart->GetAxis(0)->SetLabelsVisible(false);
    chart->GetAxis(1)->SetLabelsVisible(false);

    chart->GetAxis(0)->SetTitle("");
    chart->GetAxis(1)->SetTitle("");

    chartScene->AddItem(chart.GetPointer());
    chartActor->SetScene(chartScene.GetPointer());

    //Add both to the renderer
    vtkViewer->addChartActor(chartActor.GetPointer(), chartScene.GetPointer());

    //----------------------------------------------------
    // Add data points representing the concentric circles
    //----------------------------------------------------
    vtkNew<vtkTable> table;

    vtkNew<vtkFloatArray> circle1X;
    circle1X->SetName("Circle 1 X");
    table->AddColumn(circle1X.GetPointer());

    vtkNew<vtkFloatArray> circle1Y;
    circle1Y->SetName("Circle 1 Y");
    table->AddColumn(circle1Y.GetPointer());

    vtkNew<vtkFloatArray> circle2X;
    circle2X->SetName("Circle 2 X");
    table->AddColumn(circle2X.GetPointer());

    vtkNew<vtkFloatArray> circle2Y;
    circle2Y->SetName("Circle 2 Y");
    table->AddColumn(circle2Y.GetPointer());

    // Test charting with a few more points...
    int numPoints = 100;
    float range = 10.0;
    double xRange[2] = { 0.0, range };
    double yRange[2] = { 0.0, range };

    chart->GetAxis(0)->SetRange(xRange);
    chart->GetAxis(1)->SetRange(yRange);

    /*chart->GetAxis(0)->SetUnscaledRange(xRange);
    chart->GetAxis(1)->SetUnscaledRange(yRange);*/

    table->SetNumberOfRows(numPoints + 1);

    double radius1 = range / 6;
    double radius2 = range / 8;
    double theta;

    for (int i = 0; i < numPoints; i++)
    {
        theta = 11.0 / 7 + i * 44.0 / (7 * numPoints);

        table->SetValue(i, 0, range / 2 + radius1*cos(theta));
        table->SetValue(i, 1, range / 2 + radius1*sin(theta));

        table->SetValue(i, 2, range / 2 + radius2*cos(theta));
        table->SetValue(i, 3, range / 2 + radius2*sin(theta));
    }
    theta = 11.0 / 7;
    table->SetValue(numPoints, 0, range / 2 + radius1*cos(theta));
    table->SetValue(numPoints, 1, range / 2 + radius1*sin(theta));
    table->SetValue(numPoints, 2, range / 2 + radius2*cos(theta));
    table->SetValue(numPoints, 3, range / 2 + radius2*sin(theta));

    double lineWidth = 2.0;
    vtkPlot *points1 = chart->AddPlot(vtkChart::LINE);
    points1->SetInputData(table.GetPointer(), 0, 1);
    points1->SetColor(1, 0, 0, 255);
    points1->SetWidth(lineWidth);

    vtkPlot *points2 = chart->AddPlot(vtkChart::LINE);
    points2->SetInputData(table.GetPointer(), 2, 3);
    points2->SetColor(1, 0, 0, 255);
    points2->SetWidth(lineWidth);

    //--------------------------------------------------
    // Add data points representing the parallel lines
    //--------------------------------------------------
    vtkNew<vtkTable> table2;

    vtkNew<vtkFloatArray> parallelLineTopX;
    parallelLineTopX->SetName("parallel Line Top X");
    table2->AddColumn(parallelLineTopX.GetPointer());

    vtkNew<vtkFloatArray> parallelLineTopY;
    parallelLineTopY->SetName("parallel Line Top Y");
    table2->AddColumn(parallelLineTopY.GetPointer());

    vtkNew<vtkFloatArray> parallelLineBottomX;
    parallelLineBottomX->SetName("parallel Line Bottom X");
    table2->AddColumn(parallelLineBottomX.GetPointer());

    vtkNew<vtkFloatArray> parallelLineBottomY;
    parallelLineBottomY->SetName("parallel Line Bottom Y");
    table2->AddColumn(parallelLineBottomY.GetPointer());

    table2->SetNumberOfRows(2);
    double deltaTheta=0.07;

    theta = 11.0 / 7 + (1 - deltaTheta)*11.0 / 7;
    table2->SetValue(0, 0, range / 2 + radius1*cos(theta));
    table2->SetValue(0, 1, range / 2 + radius1*sin(theta));

    theta = deltaTheta*11.0 / 7;
    table2->SetValue(1, 0, range / 2 + radius1*cos(theta));
    table2->SetValue(1, 1, range / 2 + radius1*sin(theta));

    theta = 11.0 / 7 + (1 + deltaTheta)*11.0 / 7;
    table2->SetValue(0, 2, range / 2 + radius1*cos(theta));
    table2->SetValue(0, 3, range / 2 + radius1*sin(theta));

    theta = -deltaTheta*11.0 / 7;
    table2->SetValue(1, 2, range / 2 + radius1*cos(theta));
    table2->SetValue(1, 3, range / 2 + radius1*sin(theta));

    vtkPlot *points3 = chart->AddPlot(vtkChart::LINE);
    points3->SetInputData(table2.GetPointer(), 0, 1);
    points3->SetColor(1, 0, 0, 255);
    points3->SetWidth(lineWidth);

    vtkPlot *points4 = chart->AddPlot(vtkChart::LINE);
    points4->SetInputData(table2.GetPointer(), 2, 3);
    points4->SetColor(1, 0, 0, 255);
    points4->SetWidth(lineWidth);

    //-----------------------------------------------------------
    // Add data points to hack disabling the autoscaling of chart
    //-----------------------------------------------------------
    vtkNew<vtkTable> table3;

    vtkNew<vtkFloatArray> cornerPointX;
    cornerPointX->SetName("Corner point X");
    table3->AddColumn(cornerPointX.GetPointer());

    vtkNew<vtkFloatArray> cornerPointY;
    cornerPointY->SetName("Corner point Y");
    table3->AddColumn(cornerPointY.GetPointer());

    table3->SetNumberOfRows(2);
    table3->SetValue(0, 0, 0);
    table3->SetValue(0, 1, 0);
    table3->SetValue(1, 0, range);
    table3->SetValue(1, 1, range);

    vtkPlot *points5 = chart->AddPlot(vtkChart::POINTS);
    points5->SetInputData(table3.GetPointer(), 0, 1);
    points5->SetColor(255, 255, 255, 0);
}

///
/// \brief Create a Laparoscopic camera controller and connect
/// it to the vtk viewer
///
std::shared_ptr<LaparoscopicCameraController> addCameraController(std::shared_ptr<SDK> sdk)
{
    auto viewer = sdk->getViewerInstance();

    auto camClient = std::make_shared<VRPNForceDevice>();
    std::shared_ptr<VRPNDeviceServer> server;
    std::string input;

    if (SPACE_EXPLORER_DEVICE)
    {
        server = std::make_shared<VRPNDeviceServer>();

        //get some user input and setup device url
        input = "navigator@localhost";
    }
    else
    {
        //get some user input and setup device url
        std::string input = "Phantom0@localhost";//"Phantom@10.171.2.217"
        std::cout << "Enter the VRPN device URL(" << camClient->getDeviceURL() << "): ";
        std::getline(std::cin, input);
    }

    if (!input.empty())
    {
        camClient->setDeviceURL(input);
    }
    auto camController = std::make_shared<LaparoscopicCameraController>(camClient);
    camController->setScalingFactor(40.0);

    viewer->init(); // viewer should be initialized to be able to retrieve the camera

    std::shared_ptr<VTKViewer> vtkViewer = std::static_pointer_cast<VTKViewer>(viewer);

    vtkCamera* cam = vtkViewer->getVtkCamera();

    // set the view angle of the camera. 80 deg for laparoscopic camera
    cam->SetViewAngle(80.0);

    // set the camera to be controlled by the camera controller
    camController->setCamera(cam);

    // Connect the camera controller to the vtk viewer to enable camera manipulation
    vtkViewer->setCameraControllerData(camController->getCameraData());

    sdk->registerModule(camClient);
    sdk->registerModule(camController);

    if (SPACE_EXPLORER_DEVICE)
    {
        sdk->registerModule(server);
    }

    return camController;
}

int main(int ac, char** av)
{
    initVTKRendering();
    initIODelegates();
    const bool useVTKRenderer = true;

    auto sdk = SDK::createStandardSDK();

    // Create camera navigation scene
    createCameraNavigationScene(sdk, "./Target.vtk", "./Target-marker.vtk");

    //-------------------------------------------------------
    // Set up the viewer
    //-------------------------------------------------------

    auto viewer = sdk->getViewerInstance();

    viewer->setViewerRenderDetail(
        SIMMEDTK_VIEWERRENDER_GLOBALAXIS
        | SIMMEDTK_VIEWERRENDER_FADEBACKGROUND
        | SIMMEDTK_DISABLE_MOUSE_INTERACTION
        );

    // Get Scene
    auto scene = sdk->getScene(0);
    viewer->registerScene(scene, SMRENDERTARGET_SCREEN, "Collision pipeline demo");

    // Setup Scene lighting
    auto light1 = Light::getDefaultLighting();
    light1->lightPos.setPosition(core::Vec3d(-25.0, 10.0, 10.0));
    scene->addLight(light1);

    auto light2 = Light::getDefaultLighting();
    light2->lightPos.setPosition(core::Vec3d(25.0, 10.0, 10.0));
    scene->addLight(light2);

    auto light3 = Light::getDefaultLighting();
    light3->lightPos.setPosition(core::Vec3d(0.0, 25.0, 0.0));
    scene->addLight(light3);

    //-------------------------------------------------------

    // Add a camera controller
    std::shared_ptr<LaparoscopicCameraController> camController =
        addCameraController(sdk);

    // Enable screenshot capture
    camController->enableScreenCapture();

    std::shared_ptr<VTKViewer> vtkViewer = std::static_pointer_cast<VTKViewer>(viewer);
    vtkViewer->setScreenCaptureData(camController->getScreenCaptureData());

    // Add a 2D overlay on the 3D scene
    add2DOverlay(vtkViewer);

    // Run the SDK
    sdk->run();

    // Cleanup
    sdk->releaseScene(scene);

    return 0;
}