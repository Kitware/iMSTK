/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDataArray.h"
#include "imstkDirectionalLight.h"
#include "imstkDummyClient.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderDelegateObjectFactory.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSurfaceMesh.h"
#include "imstkVTKChartRenderDelegate.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Convert a deque to a data array
///
template<typename T>
static void
queueToArray(std::deque<T>& vals, DataArray<T>& arr)
{
    arr.resize(vals.size());
    size_t i = 0;
    for (auto val : vals)
    {
        arr[i++] = val;
    }
}

///
/// \brief Create a sequential array of type T
///
template<typename T>
static std::shared_ptr<DataArray<T>>
seqArray(int size)
{
    auto          arrPtr = std::make_shared<DataArray<T>>(size);
    DataArray<T>& arr    = *arrPtr;
    for (int i = 0; i < size; i++)
    {
        arr[i] = static_cast<T>(i);
    }
    return arrPtr;
}

///
/// \brief This example demonstrates addition of a subclassed RenderDelegate
/// which can be used to add new rendering functionality to objects
///
int
main()
{
    // Tell the factory to utilize the VTKChartRenderDelegate, for the delegate hint "Chart"
    RenderDelegateRegistrar<VTKChartRenderDelegate> registerChartDelegate("Chart");

    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Scene
    imstkNew<Scene> scene("RenderingCustomDelegate");

    const int                                           rbdCount = 2;
    std::vector<std::shared_ptr<RigidObject2>>          rbdObjs(rbdCount);
    std::vector<std::shared_ptr<RigidObjectController>> rbdControllers(rbdCount);
    std::vector<std::shared_ptr<DummyClient>>           deviceClients(rbdCount);
    for (int i = 0; i < rbdCount; i++)
    {
        rbdObjs[i] = std::make_shared<RigidObject2>("rbdObj" + std::to_string(i));
        {
            imstkNew<RigidBodyModel2> rbdModel;
            rbdModel->getConfig()->m_dt      = 0.001;
            rbdModel->getConfig()->m_gravity = Vec3d::Zero();
            rbdObjs[i]->setDynamicalModel(rbdModel);
            rbdObjs[i]->getRigidBody()->m_mass = 0.5;
            rbdObjs[i]->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 1000000.0;
            rbdObjs[i]->getRigidBody()->m_initPos = Vec3d(1.0, 0.0, 0.0); // Start the tool a unit off from controller

            auto surfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Scissors/Metzenbaum Scissors/Metz_Scissors.stl");
            rbdObjs[i]->setCollidingGeometry(surfMesh);
            rbdObjs[i]->setVisualGeometry(surfMesh);
            rbdObjs[i]->setPhysicsGeometry(surfMesh);

            std::shared_ptr<RenderMaterial> mat = rbdObjs[i]->getVisualModel(0)->getRenderMaterial();
            mat->setShadingModel(RenderMaterial::ShadingModel::PBR);
            mat->setRoughness(0.5);
            mat->setMetalness(1.0);
        }
        scene->addSceneObject(rbdObjs[i]);

        deviceClients[i] = std::make_shared<DummyClient>("test");

        // Create a virtual coupling controller
        rbdControllers[i] = std::make_shared<RigidObjectController>();
        rbdControllers[i]->setControlledObject(rbdObjs[i]);
        rbdControllers[i]->setDevice(deviceClients[i]);
        if (i == 0)
        {
            rbdControllers[i]->setLinearKs(10.0);
            rbdControllers[i]->setLinearKd(1.0);
        }
        else
        {
            rbdControllers[i]->setLinearKs(30.0);
            rbdControllers[i]->setLinearKd(1.0);
        }
        rbdControllers[i]->setAngularKs(1000000000.0);
        rbdControllers[i]->setAngularKd(300000000.0);
        rbdControllers[i]->setTranslationScaling(1.0);
        //controller->setTranslationScaling(0.02);
        rbdControllers[i]->setForceScaling(0.001);
        scene->addControl(rbdControllers[i]);
    }

    // Graph the springs with this object
    imstkNew<SceneObject>      graphObject("Graph");
    imstkNew<ChartVisualModel> chartModel;
    graphObject->addVisualModel(chartModel);
    chartModel->setViewBounds(Vec4d(0.0, 1000.0, 0.0, 300.0));
    scene->addSceneObject(graphObject);

    // Camera
    scene->getActiveCamera()->setPosition(Vec3d(0.0, 5.0, 10.0));
    scene->getActiveCamera()->setFocalPoint(Vec3d(0.0, -1.0, 0.0));
    scene->getActiveCamera()->setViewUp(Vec3d(0.0, 1.0, 0.0));

    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("light0", light);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);

        imstkNew<SimulationManager> driver;
        //driver->addModule(hapticsManager);
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        // Setup some queues to keep running track of spring forces
        const int recordSize = 5000;
        auto      timesPtr   = std::make_shared<DataArray<double>>(recordSize);

        std::deque<double> timesQueue;
        for (int j = 0; j < recordSize; j++)
        {
            timesQueue.push_back(0.0);
        }
        queueToArray(timesQueue, *timesPtr);

        std::vector<std::deque<double>>                 springForceQueues(rbdCount);
        std::vector<std::shared_ptr<DataArray<double>>> springForcesPtrs(rbdCount);
        std::vector<Plot2d>                             plots(rbdCount);
        for (int i = 0; i < rbdCount; i++)
        {
            springForcesPtrs[i] = std::make_shared<DataArray<double>>(recordSize);
            for (int j = 0; j < recordSize; j++)
            {
                springForceQueues[i].push_back(0.0);
            }
            queueToArray(springForceQueues[i], *springForcesPtrs[i]);

            Plot2d plot;
            plot.xVals = timesPtr;
            plot.yVals = springForcesPtrs[i];
            if (i == 0)
            {
                plot.lineColor = Color::Red;
            }
            else if (i == 1)
            {
                plot.lineColor = Color::Green;
            }
            else if (i == 2)
            {
                plot.lineColor = Color::Blue;
            }
            chartModel->addPlot(plot);
        }

        double t = 0.0;
        connect<Event>(sceneManager, &SceneManager::postUpdate,
            [&](Event*)
            {
                t += sceneManager->getDt();

                timesQueue.pop_front();
                timesQueue.push_back(t);
                queueToArray(timesQueue, *timesPtr);

                for (int i = 0; i < rbdCount; i++)
                {
                    springForceQueues[i].pop_front();
                    springForceQueues[i].push_back(rbdControllers[i]->getSpringForce().norm());
                    queueToArray(springForceQueues[i], *springForcesPtrs[i]);
                }
            });

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                // Run the rbd model in real time
                for (int i = 0; i < rbdCount; i++)
                {
                    rbdObjs[i]->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();
                }
            });

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    return 0;
}
