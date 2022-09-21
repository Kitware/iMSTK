/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDebugGeometryModel.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLogger.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkTextVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

static Vec3d
getRandomPositions(const double radius)
{
    return radius * Vec3d(
        2.0 * static_cast<double>(rand()) / static_cast<double>(RAND_MAX) - 1.0,
        2.0 * static_cast<double>(rand()) / static_cast<double>(RAND_MAX) - 1.0,
        2.0 * static_cast<double>(rand()) / static_cast<double>(RAND_MAX) - 1.0);
}

static Color
getRandomColor()
{
    return Color(
        static_cast<double>(rand()) / static_cast<double>(RAND_MAX),
        static_cast<double>(rand()) / static_cast<double>(RAND_MAX),
        static_cast<double>(rand()) / static_cast<double>(RAND_MAX),
        1.0);
}

class DebugGeometryGenerator : public Behaviour<double>
{
protected:
    void init() override
    {
        std::shared_ptr<Entity> entity = m_entity.lock();
        if (!entity->containsComponent(m_debugGeometryModel))
        {
            m_debugGeometryModel = std::make_shared<DebugGeometryModel>();
            entity->addComponent(m_debugGeometryModel);
        }

        if (!entity->containsComponent(m_textVisualModel))
        {
            m_textVisualModel = std::make_shared<TextVisualModel>("StatusText");
            m_textVisualModel->setPosition(TextVisualModel::DisplayPosition::UpperLeft);
            m_textVisualModel->setFontSize(30.0);
            m_textVisualModel->setTextColor(Color::Orange);
            entity->addComponent(m_textVisualModel);
        }
    }

public:
    void visualUpdate(const double& dt) override
    {
        m_t += dt;
        if (m_t > 1.0)
        {
            m_addPrimitive = true;
            m_t = 0.0;
        }

        if (m_addPrimitive)
        {
            m_mode++;
            if (m_mode % 3 == 0)
            {
                m_debugGeometryModel->addPoint(
                    getRandomPositions(15.0),
                    getRandomColor());
                m_addPrimitive = false;
            }
            else if (m_mode % 3 == 1)
            {
                Vec3d p     = getRandomPositions(50.0);
                Vec3d shift = getRandomPositions(1.0);
                m_debugGeometryModel->addLine(p + shift, -p + shift, getRandomColor());
                m_addPrimitive = false;
            }
            else if (m_mode % 3 == 2)
            {
                Vec3d shift = getRandomPositions(10.0);
                m_debugGeometryModel->addTriangle(
                    getRandomPositions(5.0) + shift,
                    getRandomPositions(5.0) + shift,
                    getRandomPositions(5.0) + shift,
                    getRandomColor());
                m_addPrimitive = false;
            }
        }

        m_textVisualModel->setText("Primitives: " +
            std::to_string(m_debugGeometryModel->getNumPoints()) + " (points) | " +
            std::to_string(m_debugGeometryModel->getNumLines()) + " (lines) | " +
            std::to_string(m_debugGeometryModel->getNumTriangles()) + " (triangles)"
            );
    }

    bool   m_addPrimitive = false;
    int    m_mode = -1;
    double m_t    = 0;
    std::shared_ptr<DebugGeometryModel> m_debugGeometryModel;
    std::shared_ptr<TextVisualModel>    m_textVisualModel;
};

///
/// \brief This example demonstrates debug rendering in iMSTK
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Create a scene
    auto scene = std::make_shared<Scene>("Debug rendering example");
    scene->getConfig()->debugCamBoundingBox = false;
    scene->getCamera("debug")->setPosition(0.0, 0.0, 50.0);

    // Setup a viewer to render in its own thread
    auto viewer = std::make_shared<VTKViewer>();
    viewer->setActiveScene(scene);
    viewer->setWindowTitle("Debug Rendering");
    viewer->setSize(1920, 1080);

    // Seed with system time
    srand(time(NULL));

    auto debugGeomObj = std::make_shared<Entity>();
    debugGeomObj->addComponent<DebugGeometryGenerator>();
    scene->addSceneObject(debugGeomObj);

    // Set Camera configuration
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 0.0, 50.0));

    // Light
    auto light = std::make_shared<DirectionalLight>();
    light->setFocalPoint(Vec3d(-1.0, -1.0, -1.0));
    light->setIntensity(1.0);
    scene->addLight("light", light);

    // Run the simulation
    {
        // Setup a scene manager to advance the scene in its own thread
        auto sceneManager = std::make_shared<SceneManager>();
        sceneManager->setSleepDelay(1.0);
        sceneManager->setActiveScene(scene);

        auto driver = std::make_shared<SimulationManager>();
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.1);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}
