/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkRenderTest.h"
#include "imstkCamera.h"
#include "imstkColorFunction.h"
#include "imstkGeometry.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneObject.h"
#include "imstkSimulationManager.h"
#include "imstkTimer.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

void
RenderTest::createScene()
{
    m_scene = std::make_shared<Scene>("Render Test Scene");
    m_scene->getActiveCamera()->setPosition(Vec3d(0.0, 1.0, -3.0));

    renderMaterial = std::make_shared<RenderMaterial>();

    ASSERT_NE(geom, nullptr) << "ERROR: No geometry";

    sceneObj    = std::make_shared<SceneObject>("SceneObject");
    visualModel = sceneObj->addComponent<VisualModel>();
    visualModel->setGeometry(geom);
    visualModel->setRenderMaterial(renderMaterial);
    m_scene->addSceneObject(sceneObj);
}

void
RenderTest::runAllMaterials()
{
    double elapsedTime = 0.0;
    displayMode  = 0;
    color        = 0;
    shadingModel = 0;
    blendMode    = 0;
    updateMaterial();

    const double updateMaterialTimeSecs = 0.05;
    const double angularVel = 5.0;
    connect<Event>(m_viewer, &VTKViewer::preUpdate,
        [&](Event*)
        {
            const double dt = m_viewer->getDt();
            elapsedTime    += dt;
            if (elapsedTime > updateMaterialTimeSecs)
            {
                elapsedTime = 0.0;
                updateMaterial();
            }
            geom->rotate(Vec3d(0.0, 1.0, 0.0), dt * angularVel);
            geom->postModified();
        });
    // Run nonstop (define our own stopping criteria above,
    // when all materials have cycled)
    runFor(-1.0, 0.01);
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

    //std::cout << "DisplayMode=" << dm << " Color=" << c << " Shading Model=" << sm << " Blend Mode=" << bm << "\n";
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
        m_driver->requestStatus(ModuleDriverStopped);
    }
}
