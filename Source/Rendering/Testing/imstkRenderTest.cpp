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

#include "imstkRenderTest.h"

#include "imstkColorFunction.h"

void
RenderTest::SetUp()
{
    scene = std::make_shared<Scene>("Render Test Scene");
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 1.0, -3.0));

    viewer = std::make_shared<VTKViewer>("Viewer");
    viewer->setActiveScene(scene);

    // Setup a scene manager to advance the scene in its own thread
    sceneManager = std::make_shared<SceneManager>("Scene Manager");
    sceneManager->setExecutionType(Module::ExecutionType::ADAPTIVE);
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

    renderMaterial = std::make_shared<RenderMaterial>();

    createGeometry();

    ASSERT_TRUE(geom != nullptr) << "ERROR: No geometry";

    visualModel = std::make_shared<VisualModel>(geom);
    visualModel->setRenderMaterial(renderMaterial);

    sceneObj = std::make_shared<SceneObject>("SceneObject");
    sceneObj->addVisualModel(visualModel);
    scene->addSceneObject(sceneObj);

    driver->requestStatus(ModuleDriverRunning);
}

void
RenderTest::runFor(const int seconds)
{
    std::thread t(&SimulationManager::start, driver);
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
    driver->requestStatus(ModuleDriverStopped);
    t.join();
}

void
RenderTest::runAllMaterials()
{
    complete     = false;
    displayMode  = 0;
    color        = 0;
    shadingModel = 0;
    blendMode    = 0;
    updateMaterial();
    connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
    {
        double Dt    = sceneManager->getDt();
        elapsedTime += Dt;
        if (elapsedTime > 0.05)
        {
            elapsedTime = 0;
            updateMaterial();
        }
        geom->rotate(Vec3d(0.0, 1.0, 0.0), PI * Dt);
        geom->postModified();
  });

    std::thread t(&SimulationManager::start, driver);
    while (!complete) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    driver->requestStatus(ModuleDriverStopped);
    t.join();
}

void
RenderTest::applyColor()
{
    auto colorFunc = std::make_shared<ColorFunction>();
    colorFunc->setNumberOfColors(3);
    colorFunc->setColor(0, imstk::Color::Green);
    colorFunc->setColor(1, imstk::Color::Blue);
    colorFunc->setColor(2, imstk::Color::Red);
    colorFunc->setColorSpace(imstk::ColorFunction::ColorSpace::RGB);
    colorFunc->setRange(0, 6);

    renderMaterial->setScalarVisibility(true);
    renderMaterial->setColorLookupTable(colorFunc);
}

void
RenderTest::updateMaterial()
{
    switch (displayMode)
    {
    case 0:
        renderMaterial->setDisplayMode(RenderMaterial::DisplayMode::Surface);
        dm = "Surface ";
        break;
    case 1:
        renderMaterial->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
        dm = "Wireframe ";
        break;
    case 2:
        renderMaterial->setDisplayMode(RenderMaterial::DisplayMode::Points);
        dm = "Points ";
        break;
    case 3:
        renderMaterial->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
        dm = "WireframeSurface ";
        break;
    case 4:
        renderMaterial->setDisplayMode(RenderMaterial::DisplayMode::VolumeRendering);
        dm = "VolumeRendering ";
        break;
    case 5:
        renderMaterial->setDisplayMode(RenderMaterial::DisplayMode::Fluid);
        dm = "Fluid ";
        break;
    case 6:
        renderMaterial->setDisplayMode(RenderMaterial::DisplayMode::Image);
        dm = "Image ";
        break;
    case 7:
        renderMaterial->setDisplayMode(RenderMaterial::DisplayMode::SurfaceNormals);
        dm = "SurfaceNormals ";
        break;
    }

    switch (color)
    {
    case 0:
        renderMaterial->setColor(Color::Blue);
        c = "Blue ";
        break;
    case 1:
        renderMaterial->setColor(Color::Green);
        c = "Green ";
        break;
    case 2:
        renderMaterial->setColor(Color::Red);
        c = "Red ";
        break;
    }

    switch (shadingModel)
    {
    case 0:
        renderMaterial->setShadingModel(RenderMaterial::ShadingModel::None);
        sm = "None ";
        break;
    case 1:
        renderMaterial->setShadingModel(RenderMaterial::ShadingModel::Phong);
        sm = "Phong ";
        break;
    case 2:
        renderMaterial->setShadingModel(RenderMaterial::ShadingModel::Gouraud);
        sm = "Gouraud ";
        break;
    case 3:
        renderMaterial->setShadingModel(RenderMaterial::ShadingModel::Flat);
        sm = "Flat ";
        break;
    case 4:
        renderMaterial->setShadingModel(RenderMaterial::ShadingModel::PBR);
        sm = "PBR ";
        break;
    }

    switch (blendMode)
    {
    case 0:
        renderMaterial->setBlendMode(RenderMaterial::BlendMode::Alpha);
        bm = "Alpha ";
        break;
    case 1:
        renderMaterial->setBlendMode(RenderMaterial::BlendMode::Additive);
        bm = "Additive ";
        break;
    case 2:
        renderMaterial->setBlendMode(RenderMaterial::BlendMode::MaximumIntensity);
        bm = "MaximumIntensity ";
        break;
    case 3:
        renderMaterial->setBlendMode(RenderMaterial::BlendMode::MinimumIntensity);
        bm = "MinimumIntensity ";
        break;
    }

    std::cout << "DisplayMode=" << dm << " Color=" << c << " Shading Model=" << sm << " Blend Mode=" << bm << "\n";
    blendMode++;
    if (blendMode == 4)
    {
        blendMode = 0;
        shadingModel++;
    }
    if (shadingModel == 5)
    {
        shadingModel = 0;
        color++;
    }
    if (color == 1)// Only check one color
    {
        color = 0;
        displayMode++;
    }
    if (displayMode == 8)
    {
        complete = true;
    }
}
