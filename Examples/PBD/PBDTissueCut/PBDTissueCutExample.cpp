/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "CutHelp.h"
#include "imstkCamera.h"
#include "imstkDirectionalLight.h"
#include "imstkGeometryUtilities.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdConstraintContainer.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPlane.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

static void
addDummyVertexPointSet(std::shared_ptr<PointSet> pointSet)
{
    // Add a dummy vertex to the vertices
    std::shared_ptr<VecDataArray<double, 3>> verticesPtr = pointSet->getVertexPositions();
    VecDataArray<double, 3>&                 vertices    = *verticesPtr;
    vertices.resize(vertices.size() + 1);
    for (int i = vertices.size() - 1; i >= 1; i--)
    {
        vertices[i] = vertices[i - 1];
    }
    vertices[0] = Vec3d(0.0, 0.0, 0.0);
    pointSet->setInitialVertexPositions(std::make_shared<VecDataArray<double, 3>>(*verticesPtr));
}

static void
addDummyVertex(std::shared_ptr<SurfaceMesh> surfMesh)
{
    addDummyVertexPointSet(surfMesh);

    // Then shift all indices by 1
    std::shared_ptr<VecDataArray<int, 3>> indicesPtr = surfMesh->getCells();
    VecDataArray<int, 3>&                 indices    = *indicesPtr;
    for (int i = 0; i < indices.size(); i++)
    {
        indices[i][0]++;
        indices[i][1]++;
        indices[i][2]++;
    }
}

static void
addDummyVertex(std::shared_ptr<TetrahedralMesh> tetMesh)
{
    addDummyVertexPointSet(tetMesh);

    // Then shift all indices by 1
    std::shared_ptr<VecDataArray<int, 4>> tissueIndicesPtr = tetMesh->getTetrahedraIndices();
    VecDataArray<int, 4>&                 tissueIndices    = *tissueIndicesPtr;
    for (int i = 0; i < tissueIndices.size(); i++)
    {
        tissueIndices[i][0]++;
        tissueIndices[i][1]++;
        tissueIndices[i][2]++;
        tissueIndices[i][3]++;
    }
}

///
/// \brief Creates tissue object
/// \param name
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of tissue block
///
static std::shared_ptr<PbdObject>
makeTissueObj(const std::string& name,
              const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tissueMesh = GeometryUtils::toTetGrid(center, size, dim);
    //std::shared_ptr<SurfaceMesh>     surfMesh   = tissueMesh->extractSurfaceMesh();

    addDummyVertex(tissueMesh);
    //addDummyVertex(surfMesh);

    // Add a mask of ints to denote how many elements are referencing this vertex
    imstkNew<DataArray<int>> referenceCountPtr(tissueMesh->getNumVertices());
    referenceCountPtr->fill(0);
    tissueMesh->setVertexAttribute("ReferenceCount", referenceCountPtr);

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    // Use FEMTet constraints
    pbdParams->m_femParams->m_YoungModulus = 50.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.4;
    pbdParams->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);
    pbdParams->m_doPartitioning   = false;
    pbdParams->m_uniformMassValue = 0.1;
    pbdParams->m_gravity    = Vec3d(0.0, -0.2, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = 5;

    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (x == 0 || /*z == 0 ||*/ x == dim[0] - 1 /*|| z == dim[2] - 1*/)
                {
                    pbdParams->m_fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z) + 1); // +1 for dummy vertex
                }
            }
        }
    }
    pbdParams->m_fixedNodeIds.push_back(0); // Fix dummy vertex

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(tissueMesh);
    pbdModel->configure(pbdParams);

    // Setup the material
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);

    // Setup the Object
    imstkNew<PbdObject> tissueObj(name);
    tissueObj->setPhysicsGeometry(tissueMesh);
    tissueObj->setVisualGeometry(tissueMesh);
    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setDynamicalModel(pbdModel);

    return tissueObj;
}

static std::shared_ptr<RigidObject2>
makeToolObj()
{
    imstkNew<Plane> toolGeometry(Vec3d(0.0, 1.0, 0.0));
    toolGeometry->setWidth(1.0);

    imstkNew<RigidObject2> toolObj("Tool");
    toolObj->setVisualGeometry(toolGeometry);
    toolObj->setCollidingGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Blue);
    toolObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    toolObj->getVisualModel(0)->getRenderMaterial()->setBackFaceCulling(false);
    toolObj->getVisualModel(0)->getRenderMaterial()->setLineWidth(1.0);

    std::shared_ptr<RigidBodyModel2> rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_maxNumIterations       = 8;
    rbdModel->getConfig()->m_velocityDamping        = 1.0;
    rbdModel->getConfig()->m_angularVelocityDamping = 1.0;
    rbdModel->getConfig()->m_maxNumConstraints      = 40;
    toolObj->setDynamicalModel(rbdModel);

    toolObj->getRigidBody()->m_mass = 1.0;
    toolObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    toolObj->getRigidBody()->m_initPos = Vec3d(0.0, 0.8, 0.0);

    return toolObj;
}

///
/// \brief This example demonstrates tetrahedral removal of a pbd simulated mesh
/// using a haptic device. Hold the button the device whilst moving it over elements
/// to remove
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene> scene("PBDTissueCut");
    scene->getActiveCamera()->setPosition(0.12, 4.51, 16.51);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    // Setup a tissue
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("Tissue",
        Vec3d(10.0, 3.0, 10.0), Vec3i(10, 3, 10), Vec3d(0.0, -1.0, 0.0));
    scene->addSceneObject(tissueObj);

    std::shared_ptr<RigidObject2> toolObj = makeToolObj();
    scene->addSceneObject(toolObj);

    /*auto interaction = std::make_shared<PbdObjectCollision>(tissueObj, toolObj, "ClosedSurfaceMeshToMeshCD");
    scene->addInteraction(interaction);*/

    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1.0);
    scene->addLight("Light", light);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setVtkLoggerMode(VTKViewer::VTKLoggerMode::MUTE);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.01);

        imstkNew<HapticDeviceManager> hapticManager;
        hapticManager->setSleepDelay(1.0); // Delay for 1ms (haptics thread is limited to max 1000hz)
        std::shared_ptr<HapticDeviceClient> hapticDeviceClient = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        imstkNew<RigidObjectController> controller;
        controller->setControlledObject(toolObj);
        controller->setDevice(hapticDeviceClient);
        controller->setTranslationScaling(0.06);
        controller->setLinearKs(1000.0);
        controller->setLinearKd(50.0);
        controller->setAngularKs(10000000.0);
        controller->setAngularKd(500000.0);
        controller->setForceScaling(0.001);
        scene->addControl(controller);

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                // Keep the tool moving in real time
                toolObj->getRigidBodyModel2()->getConfig()->m_dt = sceneManager->getDt();

                if (hapticDeviceClient->getButton(0))
                {
                    auto tissueMesh    = std::dynamic_pointer_cast<TetrahedralMesh>(tissueObj->getPhysicsGeometry());
                    auto toolPlaneGeom = std::dynamic_pointer_cast<Plane>(toolObj->getCollidingGeometry());

                    // Default config of the tool is pointing downwards on y
                    const Mat3d rot     = toolPlaneGeom->getOrientation().toRotationMatrix();
                    const Vec3d forward = (rot * Vec3d(0.0, 0.0, 1.0)).normalized();
                    const Vec3d left    = (rot * Vec3d(1.0, 0.0, 0.0)).normalized();
                    const Vec3d n       = (rot * Vec3d(0.0, 1.0, 0.0)).normalized();

                    const Vec3d planePos        = toolPlaneGeom->getPosition();
                    const Vec3d planeNormal     = n;
                    const double planeHalfWidth = toolPlaneGeom->getWidth() * 0.5;

                    std::shared_ptr<VecDataArray<double, 3>> tissueVerticesPtr = tissueMesh->getVertexPositions();
                    std::shared_ptr<VecDataArray<int, 4>> tissueIndicesPtr     = tissueMesh->getTetrahedraIndices();
                    VecDataArray<double, 3>& tissueVertices = *tissueVerticesPtr;
                    VecDataArray<int, 4>& tissueIndices     = *tissueIndicesPtr;

                    // Compute which tets should be removed
                    std::unordered_set<int> removedTets;
                    for (int i = 0; i < tissueIndices.size(); i++)
                    {
                        Vec4i& tet = tissueIndices[i];
                        std::array<Vec3d, 4> tetVerts;
                        tetVerts[0] = tissueVertices[tet[0]];
                        tetVerts[1] = tissueVertices[tet[1]];
                        tetVerts[2] = tissueVertices[tet[2]];
                        tetVerts[3] = tissueVertices[tet[3]];

                        if (splitTest(tetVerts, planePos, left, planeHalfWidth, forward, planeHalfWidth, n))
                        {
                            // Remove the tet being split
                            removedTets.insert(i);
                        }
                    }

                    // Deal with diffs
                    std::shared_ptr<PbdConstraintContainer> constraintsPtr = tissueObj->getPbdModel()->getConstraints();
                    const std::vector<std::shared_ptr<PbdConstraint>>& constraints = constraintsPtr->getConstraints();

                    // First process all removed tets by removing the constraints and setting the element to the dummy vertex
                    for (auto i : removedTets)
                    {
                        Vec4i& tet = tissueIndices[i];

                        // Find and remove the associated constraints
                        for (auto j = constraints.begin(); j != constraints.end(); j++)
                        {
                            std::vector<size_t>& vertexIds = (*j)->getVertexIds();
                            bool isSameTet = true;
                            for (int k = 0; k < 4; k++)
                            {
                                if (vertexIds[k] != tet[k])
                                {
                                    isSameTet = false;
                                    break;
                                }
                            }
                            if (isSameTet)
                            {
                                constraintsPtr->eraseConstraint(j);
                                break;
                            }
                        }

                        // Set removed tet to dummy vertex
                        tet = Vec4i(0, 0, 0, 0);
                    }

                    if (removedTets.size() > 0)
                    {
                        tissueIndicesPtr->postModified();
                        tissueMesh->postModified();
                    }
                }
        });

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            scene->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>();
            keyControl->setDevice(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            scene->addControl(keyControl);
        }

        driver->start();
    }

    return 0;
}