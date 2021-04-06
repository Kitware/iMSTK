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

#include "gtest/gtest.h"

#include <memory>

#include "imstkAPIUtilities.h"
#include "imstkCamera.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVTKViewer.h"
#include "imstkVecDataArray.h"
#include "imstkVisualModel.h"
#include "imstkColorFunction.h"
#include "imstkEventObject.h"

using namespace imstk;

namespace {

void run_for(SimulationManager* driver, int i)
{
    std::thread t(&SimulationManager::start, driver);

    std::this_thread::sleep_for(std::chrono::seconds(i));

    driver->requestStatus(ModuleDriverStopped);
    t.join();
}
}

class RenderTest : public testing::Test
{
public:

    void SetUp() override
    {
        scene = std::make_shared<Scene>("Render Test Scene");
        viewer = std::make_shared<VTKViewer>("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        sceneManager = std::make_shared<SceneManager>("Scene Manager");
        sceneManager->setActiveScene(scene);

        driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);

        mouseControl = std::make_shared<MouseSceneControl>(viewer->getMouseDevice());
        mouseControl->setSceneManager(sceneManager);
        viewer->addControl(mouseControl);

        keyControl = std::make_shared<KeyboardSceneControl>(viewer->getKeyboardDevice());
        keyControl->setSceneManager(sceneManager);
        keyControl->setModuleDriver(driver);
        viewer->addControl(keyControl);

        driver->requestStatus(ModuleDriverRunning);

    }

    std::shared_ptr<Scene> scene;
    std::shared_ptr<VTKViewer> viewer;
    std::shared_ptr<SceneManager> sceneManager;
    std::shared_ptr<SimulationManager> driver;
    std::shared_ptr<MouseSceneControl> mouseControl;
    std::shared_ptr<KeyboardSceneControl> keyControl;

};

TEST_F(RenderTest, plain_mesh)
{
    auto sceneObj = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT "/textured_organs/heart.obj", "Heart");

    ASSERT_TRUE(sceneObj != nullptr) << "ERROR: Unable to create scene object";

    run_for(driver.get(), 2);
}

TEST_F(RenderTest, mesh_material)
{
    auto sceneObj = apiutils::createAndAddVisualSceneObject(scene, iMSTK_DATA_ROOT "/textured_organs/heart.obj", "Heart");

    std::shared_ptr<RenderMaterial> material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    material->setPointSize(6.0);
    material->setLineWidth(4.0);
    sceneObj->getVisualModel(0)->setRenderMaterial(material);

    ASSERT_TRUE(sceneObj != nullptr) << "ERROR: Unable to create scene object";

    run_for(driver.get(), 2);
}

TEST_F(RenderTest, material_color_function)
{
    imstk::VecDataArray<double, 3> points;
    imstk::DataArray<float> scalars;

    for (int i = 0; i < 6; ++i)
    {
        points.push_back({0, 0, static_cast<double>(i)});
        scalars.push_back(i);
        points.push_back({1, 0, static_cast<double>(i)});
        scalars.push_back(i);
    }

    imstk::SurfaceMesh mesh;

    imstk::VecDataArray<int, 3> tris;
    for (int i = 0; i < 5; ++i)
    {
        int j = i * 2;
        tris.push_back({ j+2,   j+1, j });
        tris.push_back({ j + 3, j + 1, j + 2 });
    }

    mesh.initialize(std::make_shared<VecDataArray<double, 3>>(points), std::make_shared<VecDataArray<int, 3>>(tris));
    mesh.setVertexAttribute("scalars", std::make_shared <DataArray<float>>(scalars));
    mesh.setVertexScalars("scalars");

    auto visualModel = std::make_shared<VisualModel>(std::make_shared<SurfaceMesh>(mesh));

    std::shared_ptr<ColorFunction> colorFunc = std::make_shared<ColorFunction>();
    colorFunc->setNumberOfColors(3);
    colorFunc->setColor(0, imstk::Color::Green);
    colorFunc->setColor(1, imstk::Color::Blue);
    colorFunc->setColor(2, imstk::Color::Red);
    colorFunc->setColorSpace(imstk::ColorFunction::ColorSpace::RGB);
    colorFunc->setRange(0,6);

    auto material = std::make_shared<imstk::RenderMaterial>();
    material->setScalarVisibility(true);
    material->setColorLookupTable(colorFunc);
    visualModel->setRenderMaterial(material);

    auto sceneObj = std::make_shared<SceneObject>("plains");
    sceneObj->addVisualModel(visualModel);
    scene->addSceneObject(sceneObj);

    scene->getActiveCamera()->setPosition(Vec3d(0, 12, 3));
    scene->getActiveCamera()->setFocalPoint(Vec3d(0, 0, 3.01));

    run_for(driver.get(), 2);
}

TEST_F(RenderTest, material_color_function_dynamical)
{

    auto mesh = std::make_shared<imstk::SurfaceMesh>();
    auto points = std::make_shared<imstk::VecDataArray<double, 3>>();
    auto tris = std::make_shared<imstk::VecDataArray<int, 3>>();
    auto scalars = std::make_shared<imstk::DataArray<float>>();
    mesh->initialize(points, tris);

    auto updateMesh = [mesh](Event*)
    {
        // ODD ... updating the values in the original arrays fails somewhere (no surface can be seen)
        // , this is unexpected, when the data is in a new array then the update call successfully
        // produces a surface
        auto points = std::make_shared<imstk::VecDataArray<double, 3>>();
        auto tris = std::make_shared<imstk::VecDataArray<int, 3>>();
        auto scalars = std::make_shared<imstk::DataArray<float>>();
        mesh->clear();
        for (int i = 0; i < 6; ++i)
        {
            points->push_back({ 0, 0, static_cast<double>(i) });
            scalars->push_back(i);
            points->push_back({ 1 , 0, static_cast<double>(i) });
            scalars->push_back(i);
        }

        for (int i = 0; i < 5; ++i)
        {
            int j = i * 2;
            tris->push_back({ j + 2,   j + 1, j });
            tris->push_back({ j + 3, j + 1, j + 2 });
        }

        mesh->initialize(points, tris);
        mesh->setVertexAttribute("scalars", scalars);
        mesh->setVertexScalars("scalars");
        mesh->computeVertexNormals();
        mesh->modified();
    };

    connect<Event>(viewer, viewer->preUpdate, updateMesh);

    auto visualModel = std::make_shared<VisualModel>(mesh);
    imstk::Event e("test");
    updateMesh(&e);

    std::shared_ptr<ColorFunction> colorFunc = std::make_shared<ColorFunction>();
    colorFunc->setNumberOfColors(3);
    colorFunc->setColor(0, imstk::Color::Green);
    colorFunc->setColor(1, imstk::Color::Blue);
    colorFunc->setColor(2, imstk::Color::Red);
    colorFunc->setColorSpace(imstk::ColorFunction::ColorSpace::RGB);
    colorFunc->setRange(0, 6);

    auto material = std::make_shared<imstk::RenderMaterial>();
    material->setScalarVisibility(true);
    material->setColorLookupTable(colorFunc);
    visualModel->setRenderMaterial(material);

    auto sceneObj = std::make_shared<SceneObject>("plains");
    sceneObj->addVisualModel(visualModel);
    scene->addSceneObject(sceneObj);

    scene->getActiveCamera()->setPosition(Vec3d(0, 12, 3));
    scene->getActiveCamera()->setFocalPoint(Vec3d(0, 0, 3.01));

    run_for(driver.get(), 2);
}


int
imstkMeshRenderTest(int argc, char* argv[])
{
    // Init Google Test
    ::testing::InitGoogleTest(&argc, argv);

    // Run tests with gtest
    return RUN_ALL_TESTS();
}