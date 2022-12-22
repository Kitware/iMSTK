/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

// Generate a fluid simulation example in with a sphere-shape fluid dropping onto the ground
#define SCENE_ID 1

#include "imstkCamera.h"
#include "imstkCollisionDetectionAlgorithm.h"
#include "imstkDirectionalLight.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSphObject.h"
#include "imstkSphObjectCollision.h"
#include "imstkTextVisualModel.h"
#include "imstkVTKViewer.h"

#include "Fluid.hpp"
#include "Solid.hpp"

using namespace imstk;

///
/// \brief Usage: ./SPHFluid [threads=<num_threads>] [radius=<particle_radius>]
/// \brief Example: ./SPHFluid threads=8 radius=0.01
///
int
main(int argc, char* argv[])
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    double particleRadius = 0.1;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i)
    {
        auto param = std::string(argv[i]);
        if (param.find("radius") == 0
                 && param.find_first_of("=") != std::string::npos)
        {
            particleRadius = std::stod(param.substr(param.find_first_of("=") + 1));
            LOG(INFO) << "Particle radius: " << particleRadius;
        }
        else
        {
            LOG(FATAL) << "Invalid argument";
        }
    }

    // Override particle radius for scene3 because particles in this scene were pre-generated using particle radius 0.08
    if (SCENE_ID == 3)
    {
        particleRadius = 0.08;
    }

    imstkNew<Scene> scene("SPH Fluid");

    // Generate fluid and solid objects
    std::shared_ptr<SphObject> fluidObj         = generateFluid(particleRadius);
    std::vector<std::shared_ptr<Entity>> solids = generateSolids(scene);

    scene->addSceneObject(fluidObj);
    for (size_t i = 0; i < solids.size(); i++)
    {
        scene->addSceneObject(solids[i]);
    }

    // Collision between fluid and solid objects
    for (auto& solid : solids)
    {
        scene->addInteraction(
            std::make_shared<SphObjectCollision>(fluidObj, solid));
    }

    // configure camera
    scene->getActiveCamera()->setPosition(-0.475, 8.116, -6.728);

    // configure light (white)
    imstkNew<DirectionalLight> whiteLight;
    whiteLight->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    whiteLight->setIntensity(1.5);
    scene->addLight("whitelight", whiteLight);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setWindowTitle("SPH Fluid");
        viewer->setSize(1920, 1080);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause();

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.01);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        auto txtStatus = std::make_shared<TextVisualModel>("StatusText");
        txtStatus->setPosition(TextVisualModel::DisplayPosition::UpperLeft);
        txtStatus->setFontSize(30);
        txtStatus->setTextColor(Color::Red);
        connect<Event>(viewer, &VTKViewer::preUpdate,
            [&](Event*)
            {
                txtStatus->setText("Number of particles: " +
                    std::to_string(fluidObj->getSphModel()->getCurrentState()->getNumParticles()) +
                    "\nNumber of solids: " + std::to_string(solids.size()));
            });
        mouseAndKeyControls->addComponent(txtStatus);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    MeshIO::write(std::dynamic_pointer_cast<PointSet>(fluidObj->getPhysicsGeometry()), "fluid.vtk");

    return 0;
}
