/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSimulationUtils.h"
#include "imstkSpotLight.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshSubdivide.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Creates cloth object
/// \param name
/// \param cloth width
/// \param cloth height
/// \param cloth row count
/// \param cloth column count
///
static std::shared_ptr<PbdObject>
makeClothObj(const std::string& name,
             const double       width,
             const double       height,
             const int          nRows,
             const int          nCols)
{
    imstkNew<PbdObject> clothObj(name);

    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> clothMesh =
        GeometryUtils::toTriangleGrid(Vec3d::Zero(),
            Vec2d(width, height), Vec2i(nRows, nCols));

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0e2);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1.0e1);
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.01;
    pbdParams->m_iterations = 5;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(clothMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    clothObj->addVisualModel(visualModel);
    clothObj->setPhysicsGeometry(clothMesh);
    clothObj->setDynamicalModel(pbdModel);
    clothObj->getPbdBody()->fixedNodeIds     = { 0, nCols - 1 };
    clothObj->getPbdBody()->uniformMassValue = width * height / (nRows * nCols);

    return clothObj;
}

///
/// \brief This example demonstrates replacement of geometry on a pbd
/// simualted cloth
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    // Setup a scene
    imstkNew<Scene> scene("PBDCloth");
    scene->getActiveCamera()->setFocalPoint(0.0, -5.0, 0.0);
    scene->getActiveCamera()->setPosition(0.0, 1.5, 25.0);
    scene->getActiveCamera()->setViewUp(0.0, 1.0, 0.0);

    std::shared_ptr<PbdObject> clothObj = makeClothObj("Cloth", 10.0, 10.0, 8, 8);
    scene->addSceneObject(clothObj);

    // Light (white)
    imstkNew<DirectionalLight> whiteLight;
    whiteLight->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    whiteLight->setIntensity(1.0);
    scene->addLight("whitelight", whiteLight);

    // Light (red)
    imstkNew<SpotLight> colorLight;
    colorLight->setPosition(Vec3d(-5.0, -3.0, 5.0));
    colorLight->setFocalPoint(Vec3d(0.0, -5.0, 5.0));
    colorLight->setIntensity(100.);
    colorLight->setColor(Color::Red);
    colorLight->setSpotAngle(30.0);
    scene->addLight("colorlight", colorLight);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.005);

        // Add default mouse and keyboard controls to the viewer
        std::shared_ptr<Entity> mouseAndKeyControls =
            SimulationUtils::createDefaultSceneControl(driver);
        scene->addSceneObject(mouseAndKeyControls);

        // Queue keypress to be called after scene thread
        queueConnect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress, sceneManager, [&](KeyEvent* e)
            {
                // When i is pressed replace the PBD cloth with a subdivided one
                if (e->m_key == 'i')
                {
                    // This has a number of issues that make it not physically realistic
                    // - Mass is not conservative when interpolated from subdivide
                    // - Constraint resting lengths are not correctly reinited
                    std::shared_ptr<SurfaceMesh> clothMesh = std::dynamic_pointer_cast<SurfaceMesh>(clothObj->getPhysicsGeometry());
                    imstkNew<SurfaceMeshSubdivide> subdiv;
                    subdiv->setInputMesh(clothMesh);
                    subdiv->setNumberOfSubdivisions(1);
                    subdiv->setSubdivisionType(SurfaceMeshSubdivide::Type::LINEAR);
                    subdiv->update();
                    std::shared_ptr<SurfaceMesh> newClothMesh = subdiv->getOutputMesh();

                    // RenderDelegates cannot visually have entire geometries swapped yet, so even though we could just set the geometry
                    // on the model, you would not visually see it. Instead we replace the vertex and index buffers of the existing one
                    // Another issue here is that initial geometry is not remapped so reset will not reset to undeformed config
                    clothMesh->setInitialVertexPositions(std::make_shared<VecDataArray<double, 3>>(*newClothMesh->getVertexPositions()));
                    clothMesh->setVertexPositions(newClothMesh->getVertexPositions());
                    clothMesh->setCells(newClothMesh->getCells());
                    clothMesh->setVertexAttribute("Velocities", newClothMesh->getVertexAttribute("Velocities"));
                    clothMesh->postModified();

                    // Re-setup the constraints on the object
                    clothObj->initialize();
                    clothObj->getPbdModel()->initialize();
                }
        });

        driver->start();
    }

    return 0;
}
