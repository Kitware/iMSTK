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

#include "imstkCamera.h"
#include "imstkCapsule.h"
#include "imstkDirectionalLight.h"
#include "imstkImageData.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOneToOneMap.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectCollision.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkPbdBaryPointToPointConstraint.h"
#include "imstkPointPicker.h"
#include "imstkPbdObjectGrasping.h"
#include "imstkSphere.h"
#include "imstkHapticDeviceClient.h"
#include "imstkHapticDeviceManager.h"
#include "imstkRigidObjectController.h"
#include "imstkRigidObject2.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRbdConstraint.h"

using namespace imstk;

//#define USE_FEM

///
/// \brief Creates a tetraheral grid
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of grid
///
static std::shared_ptr<TetrahedralMesh>
makeTetGrid(const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    imstkNew<TetrahedralMesh> tissueMesh;

    imstkNew<VecDataArray<double, 3>> verticesPtr(dim[0] * dim[1] * dim[2]);
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    const Vec3d                       dx       = size.cwiseQuotient((dim - Vec3i(1, 1, 1)).cast<double>());
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                vertices[x + dim[0] * (y + dim[1] * z)] = Vec3i(x, y, z).cast<double>().cwiseProduct(dx) - size * 0.5 + center;
            }
        }
    }

    // Add connectivity data
    imstkNew<VecDataArray<int, 4>> indicesPtr;
    VecDataArray<int, 4>&          indices = *indicesPtr.get();
    for (int z = 0; z < dim[2] - 1; z++)
    {
        for (int y = 0; y < dim[1] - 1; y++)
        {
            for (int x = 0; x < dim[0] - 1; x++)
            {
                int cubeIndices[8] =
                {
                    x + dim[0] * (y + dim[1] * z),
                    (x + 1) + dim[0] * (y + dim[1] * z),
                    (x + 1) + dim[0] * (y + dim[1] * (z + 1)),
                    x + dim[0] * (y + dim[1] * (z + 1)),
                    x + dim[0] * ((y + 1) + dim[1] * z),
                    (x + 1) + dim[0] * ((y + 1) + dim[1] * z),
                    (x + 1) + dim[0] * ((y + 1) + dim[1] * (z + 1)),
                    x + dim[0] * ((y + 1) + dim[1] * (z + 1))
                };

                // Alternate the pattern so the edges line up on the sides of each voxel
                if ((z % 2 ^ x % 2) ^ y % 2)
                {
                    indices.push_back(Vec4i(cubeIndices[0], cubeIndices[7], cubeIndices[5], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[7], cubeIndices[2], cubeIndices[0]));
                    indices.push_back(Vec4i(cubeIndices[2], cubeIndices[7], cubeIndices[5], cubeIndices[0]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[2], cubeIndices[0], cubeIndices[5]));
                    indices.push_back(Vec4i(cubeIndices[2], cubeIndices[6], cubeIndices[7], cubeIndices[5]));
                }
                else
                {
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[7], cubeIndices[6], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[3], cubeIndices[6], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[3], cubeIndices[6], cubeIndices[2], cubeIndices[1]));
                    indices.push_back(Vec4i(cubeIndices[1], cubeIndices[6], cubeIndices[5], cubeIndices[4]));
                    indices.push_back(Vec4i(cubeIndices[0], cubeIndices[3], cubeIndices[1], cubeIndices[4]));
                }
            }
        }
    }

    imstkNew<VecDataArray<float, 2>> uvCoordsPtr(dim[0] * dim[1] * dim[2]);
    VecDataArray<float, 2>&          uvCoords = *uvCoordsPtr.get();
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                uvCoords[x + dim[0] * (y + dim[1] * z)] = Vec2f(static_cast<float>(x) / dim[0], static_cast<float>(z) / dim[2]) * 3.0;
            }
        }
    }

    // Ensure correct windings
    for (int i = 0; i < indices.size(); i++)
    {
        if (tetVolume(vertices[indices[i][0]], vertices[indices[i][1]], vertices[indices[i][2]], vertices[indices[i][3]]) < 0.0)
        {
            std::swap(indices[i][0], indices[i][2]);
        }
    }

    tissueMesh->initialize(verticesPtr, indicesPtr);
    tissueMesh->setVertexTCoords("uvs", uvCoordsPtr);

    return tissueMesh;
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
    imstkNew<PbdObject> tissueObj(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tissueMesh = makeTetGrid(size, dim, center);
    std::shared_ptr<SurfaceMesh>     surfMesh   = tissueMesh->extractSurfaceMesh();

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
#ifdef USE_FEM
    // Use FEMTet constraints (42k - 85k for tissue, but we want
    // something much more stretchy to wrap)
    pbdParams->m_femParams->m_YoungModulus = 100000.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.4; // 0.48 for tissue
    pbdParams->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);
#else
    // Use volume+distance constraints, worse results. More performant (can use larger mesh)
    // Handles inversion better
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 100000.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 100000.0);
#endif
    pbdParams->m_doPartitioning   = false;
    pbdParams->m_uniformMassValue = 100.0;
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = 0.001;
    pbdParams->m_iterations = 5;
    pbdParams->m_contactStiffness = 0.1;
    pbdParams->m_viscousDampingCoeff = 0.05;

    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (x == 0 || /*z == 0 ||*/ x == dim[0] - 1 /*|| z == dim[2] - 1*/)
                {
                    pbdParams->m_fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z));
                }
            }
        }
    }

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(tissueMesh);
    pbdModel->configure(pbdParams);

    // Setup the material
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setColor(Color(0.77, 0.53, 0.34));
    material->setEdgeColor(Color(0.87, 0.63, 0.44));
    //material->setOpacity(0.5);

    // Setup the Object
    tissueObj->setVisualGeometry(surfMesh);
    tissueObj->getVisualModel(0)->setRenderMaterial(material);
    tissueObj->setPhysicsGeometry(tissueMesh);
    tissueObj->setCollidingGeometry(surfMesh);
    tissueObj->setPhysicsToCollidingMap(std::make_shared<OneToOneMap>(tissueMesh, surfMesh));
    tissueObj->setDynamicalModel(pbdModel);

    return tissueObj;
}

static std::shared_ptr<RigidObject2>
makeToolObj()
{
    imstkNew<LineMesh> toolGeom;
    imstkNew<VecDataArray<double, 3>> verticesPtr(2);
    (*verticesPtr)[0] = Vec3d(0.0, -0.05, 0.0);
    (*verticesPtr)[1] = Vec3d(0.0, 0.05, 0.0);
    imstkNew<VecDataArray<int, 2>> indicesPtr(1);
    (*indicesPtr)[0] = Vec2i(0, 1);
    toolGeom->initialize(verticesPtr, indicesPtr);

    imstkNew<RigidObject2> toolObj("ToolObj");
    toolObj->setVisualGeometry(toolGeom);
    toolObj->setCollidingGeometry(toolGeom);
    toolObj->setPhysicsGeometry(toolGeom);
    toolObj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.9, 0.9, 0.9));
    toolObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::PBR);
    toolObj->getVisualModel(0)->getRenderMaterial()->setRoughness(0.5);
    toolObj->getVisualModel(0)->getRenderMaterial()->setMetalness(1.0);
    toolObj->getVisualModel(0)->getRenderMaterial()->setIsDynamicMesh(false);

    std::shared_ptr<RigidBodyModel2> rbdModel = std::make_shared<RigidBodyModel2>();
    rbdModel->getConfig()->m_gravity = Vec3d::Zero();
    rbdModel->getConfig()->m_maxNumIterations = 5;
    toolObj->setDynamicalModel(rbdModel);

    toolObj->getRigidBody()->m_mass = 0.3;
    toolObj->getRigidBody()->m_intertiaTensor = Mat3d::Identity() * 10000.0;
    toolObj->getRigidBody()->m_initPos = Vec3d(0.0, 0.0, 0.0);

    return toolObj;
}

///
/// \brief This example demonstrates collision interaction with a 3d pbd
/// simulated tissue (tetrahedral)
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene> scene("PbdTissueStitch");
    scene->getActiveCamera()->setPosition(0.0012, 0.0451, 0.1651);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    // Setup a tet tissue
    std::shared_ptr<PbdObject> tissueObj = makeTissueObj("Tissue",
        Vec3d(0.1, 0.01, 0.07), Vec3i(12, 2, 8), Vec3d(0.0, 0.0, 0.0));
    scene->addSceneObject(tissueObj);

    auto cdObj       = std::make_shared<CollidingObject>("Bone");
    auto capsuleGeom = std::make_shared<Capsule>();
    capsuleGeom->setPosition(0.0, 0.03, 0.0);
    capsuleGeom->setRadius(0.01);
    capsuleGeom->setLength(0.08);
    capsuleGeom->setOrientation(Quatd(Rotd(PI_2, Vec3d(1.0, 0.0, 0.0))));
    cdObj->setVisualGeometry(capsuleGeom);
    cdObj->getVisualModel(0)->getRenderMaterial()->setColor(
        Color(246.0 / 255.0, 127.0 / 255.0, 123.0 / 255.0));
    cdObj->setCollidingGeometry(capsuleGeom);
    scene->addSceneObject(cdObj);

    std::shared_ptr<RigidObject2> toolObj = makeToolObj();
    scene->addSceneObject(toolObj);

    // Setup CD with a cylinder CD object
    auto interaction = std::make_shared<PbdObjectCollision>(tissueObj, cdObj, "SurfaceMeshToCapsuleCD");
    scene->addInteraction(interaction);

    auto grasping = std::make_shared<PbdObjectGrasping>(tissueObj);
    scene->addInteraction(grasping);

    // Light
    imstkNew<DirectionalLight> light1;
    light1->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light1->setIntensity(0.5);
    scene->addLight("Light1", light1);

    // Light
    imstkNew<DirectionalLight> light2;
    light2->setFocalPoint(Vec3d(-5.0, -8.0, -5.0));
    light2->setIntensity(0.5);
    scene->addLight("Light2", light2);

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setDebugAxesLength(0.001, 0.001, 0.001);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.001);

        bool performStitch = false;

#ifdef iMSTK_USE_OPENHAPTICS
        imstkNew<HapticDeviceManager> hapticManager;
        hapticManager->setSleepDelay(0.1); // Delay for 1ms (haptics thread is limited to max 1000hz)
        std::shared_ptr<HapticDeviceClient> hapticDeviceClient = hapticManager->makeDeviceClient();
        driver->addModule(hapticManager);

        imstkNew<RigidObjectController> controller(toolObj, hapticDeviceClient);
        controller->setTranslationScaling(0.001);
        controller->setLinearKs(1000.0);
        controller->setAngularKs(10000000.0);
        controller->setUseCritDamping(true);
        controller->setForceScaling(0.0045);
        controller->setSmoothingKernelSize(15);
        controller->setUseForceSmoothening(true);
        scene->addController(controller);

        connect<ButtonEvent>(hapticDeviceClient, &HapticDeviceClient::buttonStateChanged,
            [&](ButtonEvent* e)
            {
                if (e->m_button == 0 && e->m_buttonState == BUTTON_PRESSED)
                {
                    performStitch = true;
                }
            });
#endif

        // Record the tool position relative to the camera
        // As the camera moves, reapply that relative transform

        const std::vector<size_t>& fixedNodes = tissueObj->getPbdModel()->getConfig()->m_fixedNodeIds;
        std::vector<Vec3d>         initPositions;

        auto tetMesh =
            std::dynamic_pointer_cast<TetrahedralMesh>(tissueObj->getPhysicsGeometry());
        std::shared_ptr<VecDataArray<double, 3>> verticesPtr = tetMesh->getVertexPositions();
        VecDataArray<double, 3>&                 vertices    = *verticesPtr;
        for (size_t i = 0; i < fixedNodes.size(); i++)
        {
            initPositions.push_back(vertices[fixedNodes[i]]);
        }

        connect<KeyEvent>(viewer->getKeyboardDevice(), &KeyboardDeviceClient::keyPress,
            [&](KeyEvent* e)
            {
                if (e->m_key == 'g')
                {
                    tissueObj->getPbdModel()->getConfig()->m_gravity = Vec3d(0.0, -1.0, 0.0);
                }
#ifdef iMSTK_USE_OPENHAPTICS
                else if (e->m_key == 's')
                {
                    performStitch = true;
                }
#endif
            });

        // Script the movement of the tissues fixed points
        // Move upwards, then inwards
       
        std::shared_ptr<PbdBaryPointToPointConstraint> stitchConstraint = nullptr;
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                const double dt = sceneManager->getDt();

                for (size_t i = 0; i < fixedNodes.size(); i++)
                {
                    Vec3d initPos = initPositions[i];
                    Vec3d& pos    = vertices[fixedNodes[i]];

                    const double dy = std::abs(pos[1] - initPos[1]);
                    const double dx = std::abs(pos[0] - initPos[0]);
                    if (dy < 0.04)
                    {
                        pos[1] += 0.01 * dt;
                    }
                    else if (dx < 0.03)
                    {
                        if (initPos[0] < 0.0)
                        {
                            pos[0] += 0.01 * dt;
                        }
                        else
                        {
                            pos[0] -= 0.01 * dt;
                        }
                        if (initPos[1] < 0.0)
                        {
                            pos[1] += 0.005 * dt;
                        }
                    }
                    else
                    {
                        tissueObj->getPbdModel()->setPointUnfixed(fixedNodes[i]);
                    }
                }

                if (performStitch && stitchConstraint == nullptr)
                {
                    std::shared_ptr<VecDataArray<int, 4>> indicesPtr     = tetMesh->getTetrahedraIndices();

                    auto velocitiesPtr =
                        std::dynamic_pointer_cast<VecDataArray<double, 3>>(tetMesh->getVertexAttribute("Velocities"));
                    CHECK(velocitiesPtr != nullptr) << "Trying to stitch with geometry that has no Velocities";

                    auto invMassesPtr =
                        std::dynamic_pointer_cast<DataArray<double>>(tetMesh->getVertexAttribute("InvMass"));
                    CHECK(invMassesPtr != nullptr) << "Trying to stitch with geometry that has no InvMass";

                    auto toolGeom = std::dynamic_pointer_cast<LineMesh>(toolObj->getCollidingGeometry());

                    // Perform line intersection to place a stitch along intersection points in line
                    PointPicker picker;
                    const Vec3d& v1 = toolGeom->getVertexPosition(0);
                    const Vec3d& v2 = toolGeom->getVertexPosition(1);
                    picker.setPickingRay(v1, (v2 - v1).normalized());
                    picker.setUseFirstHit(false);
                    /*const std::vector<PickData>& pickData = picker.pick(tetMesh);

                    // Select the intersection points closest together but on opposites sides of the origin
                    std::pair<PickData, PickData> closestPair;
                    double minDist = IMSTK_DOUBLE_MAX;
                    for (size_t i = 0; i < pickData.size(); i++)
                    {
                        const Vec3d& pickPt1 = pickData[i].pickPoint;
                        if (pickPt1[0] > 0.0)
                        {
                            continue;
                        }
                        for (size_t j = 0; j < pickData.size(); j++)
                        {
                            const Vec3d& pickPt2 = pickData[j].pickPoint;
                            if (i == j || pickPt2[0] < 0.0)
                            {
                                continue;
                            }

                            const double dist    = (pickPt2 - pickPt1).norm();
                            if (dist < minDist)
                            {
                                minDist     = dist;
                                closestPair = { pickData[i], pickData[j] };
                            }
                        }
                    }*/

                    {
                        // Perform the picking only on surface data
                        std::shared_ptr<SurfaceMesh> surfMesh = tetMesh->extractSurfaceMesh();
                        surfMesh->computeTrianglesNormals();

                        const std::vector<PickData>& pickData = picker.pick(surfMesh);

                        // ** Warning **, surface triangles are not 100% garunteed to tell inside/out
                        // Use angle-weighted psuedonormals as done in MeshToMeshBruteForceCD
                        std::shared_ptr<VecDataArray<double, 3>> faceNormalsPtr = surfMesh->getCellNormals();
                        const VecDataArray<double, 3>& faceNormals = *faceNormalsPtr;

                        // Find all neighbor pairs with normals facing each other
                        std::vector<std::pair<PickData, PickData>> constraintPair;
                        for (size_t i = 0, j = 1; i < pickData.size() - 1; i++, j++)
                        {
                            const Vec3d& pt_i = pickData[i].pickPoint;
                            const Vec3d& pt_j = pickData[j].pickPoint;
                            const Vec3d& normal_i = faceNormals[pickData[i].ids[0]];
                            const Vec3d& normal_j = faceNormals[pickData[j].ids[0]];
                            const Vec3d diff = pt_j - pt_i;

                            //bool faceOpposite = (normal_i.dot(normal_j) < 0.0);
                            std::cout << "diff: " << diff.transpose() << std::endl;
                            std::cout << "normal_i: " << normal_i.transpose() << std::endl;
                            std::cout << "normal_j: " << normal_j.transpose() << std::endl;
                            bool faceInwards = (diff.dot(normal_i) > 0.0) && (diff.dot(normal_j) < 0.0);

                            // If they face in opposite directions
                            if (faceInwards)
                            {
                                constraintPair.push_back({ pickData[i], pickData[j] });
                            }
                        }
                        
                        for (size_t i = 0; i < constraintPair.size(); i++)
                        {
                            // Get the tet id from the triangle id
                            size_t triId1 = constraintPair[i].first.ids[0];
                            size_t tetId1 = 0;
                            size_t triId2 = constraintPair[i].second.ids[0];
                            size_t tetId2 = 0;

                            std::vector<VertexMassPair> cell1(4);
                            for (size_t i = 0; i < 4; i++)
                            {
                                const int vertexId = (*indicesPtr)[tetId1][i];
                                cell1[i] = { &(*verticesPtr)[vertexId], (*invMassesPtr)[vertexId], &(*velocitiesPtr)[vertexId] };
                            }
                            const Vec4d bcCoord1 = baryCentric(constraintPair[i].first.pickPoint,
                                *cell1[0].vertex, *cell1[1].vertex, *cell1[2].vertex, *cell1[3].vertex);
                            std::vector<double> weights1 = { bcCoord1[0], bcCoord1[1], bcCoord1[2], bcCoord1[3] };

                            std::vector<VertexMassPair> cell2(4);
                            for (size_t i = 0; i < 4; i++)
                            {
                                const int vertexId = (*indicesPtr)[tetId2][i];
                                cell2[i] = { &(*verticesPtr)[vertexId], (*invMassesPtr)[vertexId], &(*velocitiesPtr)[vertexId] };
                            }
                            const Vec4d bcCoord2 = baryCentric(constraintPair[i].second.pickPoint,
                                *cell2[0].vertex, *cell2[1].vertex, *cell2[2].vertex, *cell2[3].vertex);
                            std::vector<double> weights2 = { bcCoord2[0], bcCoord2[1], bcCoord2[2], bcCoord2[3] };

                            stitchConstraint = std::make_shared<PbdBaryPointToPointConstraint>();
                            stitchConstraint->initConstraint(cell1, weights1, cell2, weights2, 0.1, 0.1);
                        }
                    }

                    // Assume tet to tet
                   /* std::vector<VertexMassPair> cell1(4);
                    for (size_t i = 0; i < 4; i++)
                    {
                        const int vertexId = (*indicesPtr)[closestPair.first.ids[0]][i];
                        cell1[i] = { &(*verticesPtr)[vertexId], (*invMassesPtr)[vertexId], &(*velocitiesPtr)[vertexId] };
                    }
                    const Vec4d bcCoord1 = baryCentric(closestPair.first.pickPoint,
                        *cell1[0].vertex, *cell1[1].vertex, *cell1[2].vertex, *cell1[3].vertex);
                    std::vector<double> weights1 = { bcCoord1[0], bcCoord1[1], bcCoord1[2], bcCoord1[3] };

                    std::vector<VertexMassPair> cell2(4);
                    for (size_t i = 0; i < 4; i++)
                    {
                        const int vertexId = (*indicesPtr)[closestPair.second.ids[0]][i];
                        cell2[i] = { &(*verticesPtr)[vertexId], (*invMassesPtr)[vertexId], &(*velocitiesPtr)[vertexId] };
                    }
                    const Vec4d bcCoord2 = baryCentric(closestPair.second.pickPoint,
                        *cell2[0].vertex, *cell2[1].vertex, *cell2[2].vertex, *cell2[3].vertex);
                    std::vector<double> weights2 = { bcCoord2[0], bcCoord2[1], bcCoord2[2], bcCoord2[3] };

                    stitchConstraint = std::make_shared<PbdBaryPointToPointConstraint>();
                    stitchConstraint->initConstraint(cell1, weights1, cell2, weights2, 0.1, 0.1);*/
                }
                if (stitchConstraint != nullptr)
                {
                    stitchConstraint->solvePosition();
                }
            });

        // Add mouse and keyboard controls to the viewer
        {
            imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            viewer->addControl(keyControl);
        }

        driver->start();
    }

    return 0;
}