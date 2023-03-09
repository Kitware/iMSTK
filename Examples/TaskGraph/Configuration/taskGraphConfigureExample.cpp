/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkColorFunction.h"
#include "imstkEntity.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdSystem.h"
#include "imstkPbdMethod.h"
#include "imstkPbdSystemConfig.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraph.h"
#include "imstkTaskGraphVizWriter.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

static std::shared_ptr<Entity>
makeClothObj(const std::string& name, double width, double height, int nRows, int nCols)
{
    std::shared_ptr<SurfaceMesh> clothMesh =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(width, height), Vec2i(nRows, nCols));

    // Setup the Parameters
    imstkNew<PbdSystemConfig> pbdParams;
    pbdParams->enableConstraint(PbdSystemConfig::ConstraintGenType::Distance, 1e2);
    pbdParams->enableConstraint(PbdSystemConfig::ConstraintGenType::Dihedral, 1e1);
    pbdParams->m_gravity    = Vec3d(0, -9.8, 0);
    pbdParams->m_dt         = 0.007;
    pbdParams->m_iterations = 5;

    // Setup the Model
    imstkNew<PbdSystem> pbdSystem;
    pbdSystem->configure(pbdParams);

    // Setup visual models
    imstkNew<VisualModel> clothModel;
    clothModel->setGeometry(clothMesh);
    clothModel->getRenderMaterial()->setBackFaceCulling(false);
    clothModel->getRenderMaterial()->setColor(Color::LightGray);
    clothModel->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

    imstkNew<VisualModel> clothSurfaceNormals;
    clothSurfaceNormals->setGeometry(clothMesh);
    clothSurfaceNormals->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::SurfaceNormals);
    clothSurfaceNormals->getRenderMaterial()->setPointSize(0.5);

    // Setup the Object
    auto clothObj = std::make_shared<Entity>(name);
    clothObj->addComponent(clothModel);
    clothObj->addComponent(clothSurfaceNormals);
    auto method = clothObj->addComponent<PbdMethod>();
    method->setGeometry(clothMesh);
    method->setPbdSystem(pbdSystem);
    method->setFixedNodes({ 0, nCols - 1 });
    method->setUniformMass(width * height / (nRows * nCols));
    return clothObj;
}

///
/// \brief This example demonstrates how to modify the task graph
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("PBDCloth");
    scene->getActiveCamera()->setFocalPoint(0.0, -5.0, 0.0);
    scene->getActiveCamera()->setPosition(0.0, 1.5, 25.0);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    std::shared_ptr<Entity> clothObj = makeClothObj("Cloth", 10.0, 10.0, 16, 16);
    scene->addSceneObject(clothObj);

    // Setup some scalars
    auto clothGeometry = std::dynamic_pointer_cast<SurfaceMesh>(clothObj->getComponent<PbdMethod>()->getGeometry());
    auto scalarsPtr    = std::make_shared<DataArray<double>>(clothGeometry->getNumVertices());
    std::fill_n(scalarsPtr->getPointer(), scalarsPtr->size(), 0.0);
    clothGeometry->setVertexScalars("scalars", scalarsPtr);

    // Setup the material for the scalars
    std::shared_ptr<RenderMaterial> material = clothObj->getComponentN<VisualModel>(0)->getRenderMaterial();
    material->setScalarVisibility(true);
    std::shared_ptr<ColorFunction> colorFunc = std::make_shared<ColorFunction>();
    colorFunc->setNumberOfColors(2);
    colorFunc->setColor(0, Color::Green);
    colorFunc->setColor(1, Color::Red);
    colorFunc->setColorSpace(ColorFunction::ColorSpace::RGB);
    colorFunc->setRange(0.0, 2.0);
    material->setColorLookupTable(colorFunc);

    // Adds a custom physics step to print out maximum velocity
    connect<Event>(scene, &Scene::configureTaskGraph,
        [&](Event*)
        {
            // Get the graph
            std::shared_ptr<TaskGraph> graph = scene->getTaskGraph();

            // First write the graph before we make modifications, just to show the changes
            imstkNew<TaskGraphVizWriter> writer;
            writer->setInput(graph);
            writer->setFileName("taskGraphConfigureExampleOld.svg");
            writer->write();

            // This node computes displacements and sets the color to the magnitude
            std::shared_ptr<TaskNode> computeVelocityScalars = std::make_shared<TaskNode>([&]()
                {
                    const VecDataArray<double, 3>& velocities =
                        *std::dynamic_pointer_cast<VecDataArray<double, 3>>(clothGeometry->getVertexAttribute("Velocities"));
                    DataArray<double>& scalars = *scalarsPtr;
                    for (int i = 0; i < velocities.size(); i++)
                    {
                        scalars[i] = velocities[i].norm();
                    }
        }, "ComputeVelocityScalars");

            // After IntegratePosition
            graph->insertAfter(clothObj->getComponent<PbdMethod>()->getUpdateGeometryNode(), computeVelocityScalars);

            // Write the modified graph
            writer->setFileName("taskGraphConfigureExampleNew.svg");
            writer->write();
    });

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause();

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        driver->start();
    }

    // Write out simulation geometry
    MeshIO::write(clothGeometry, "cloth.vtk");

    return 0;
}
