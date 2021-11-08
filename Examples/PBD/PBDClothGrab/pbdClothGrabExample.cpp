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
#include "imstkImageData.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"
#include "imstkPbdObjectCollision.h"
#include "imstkPlane.h"
#include "imstkCollisionGraph.h"
#include "imstkMouseDeviceClient.h"
#include "imstkCollisionUtils.h"
#include "imstkSphere.h"
#include "imstkTaskGraph.h"

using namespace imstk;

static void
setFabricTextures(std::shared_ptr<RenderMaterial> material)
{
    auto diffuseTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fabricDiffuse.jpg");
    material->addTexture(std::make_shared<Texture>(diffuseTex, Texture::Type::Diffuse));
    auto normalTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fabricNormal.jpg");
    material->addTexture(std::make_shared<Texture>(normalTex, Texture::Type::Normal));
    auto ormTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fabricORM.jpg");
    material->addTexture(std::make_shared<Texture>(ormTex, Texture::Type::ORM));
}

static void
setFleshTextures(std::shared_ptr<RenderMaterial> material)
{
    auto diffuseTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshDiffuse.jpg");
    material->addTexture(std::make_shared<Texture>(diffuseTex, Texture::Type::Diffuse));
    auto normalTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshNormal.jpg");
    material->addTexture(std::make_shared<Texture>(normalTex, Texture::Type::Normal));
    auto ormTex = MeshIO::read<ImageData>(iMSTK_DATA_ROOT "/textures/fleshORM.jpg");
    material->addTexture(std::make_shared<Texture>(ormTex, Texture::Type::ORM));
}

///
/// \brief Creates cloth geometry
/// \param cloth width
/// \param cloth height
/// \param cloth row count
/// \param cloth column count
///
static std::shared_ptr<SurfaceMesh>
makeClothGeometry(const Vec2d  size,
                  const Vec2i  dim,
                  const Vec3d  shift,
                  const double uvScale)
{
    imstkNew<SurfaceMesh> clothMesh;

    imstkNew<VecDataArray<double, 3>> verticesPtr(dim[0] * dim[1]);
    VecDataArray<double, 3>&          vertices  = *verticesPtr.get();
    const Vec2d                       dx        = size.cwiseQuotient((dim - Vec2i(1, 1)).cast<double>());
    const Vec3d                       halfShift = Vec3d(size[0], 0.0, size[1]) * 0.5;
    for (int i = 0; i < dim[1]; i++)
    {
        for (int j = 0; j < dim[0]; j++)
        {
            vertices[i * dim[0] + j] = Vec3d(dx[0] * static_cast<double>(i), 0.0, dx[1] * static_cast<double>(j)) - halfShift + shift;
        }
    }

    // Add connectivity data
    imstkNew<VecDataArray<int, 3>> indicesPtr;
    VecDataArray<int, 3>&          indices = *indicesPtr.get();
    for (int i = 0; i < dim[1] - 1; i++)
    {
        for (int j = 0; j < dim[0] - 1; j++)
        {
            const int index1 = i * dim[0] + j;
            const int index2 = index1 + dim[0];
            const int index3 = index1 + 1;
            const int index4 = index2 + 1;

            // Interleave [/][\]
            if (i % 2 ^ j % 2)
            {
                indices.push_back(Vec3i(index1, index2, index3));
                indices.push_back(Vec3i(index4, index3, index2));
            }
            else
            {
                indices.push_back(Vec3i(index2, index4, index1));
                indices.push_back(Vec3i(index4, index3, index1));
            }
        }
    }

    imstkNew<VecDataArray<float, 2>> uvCoordsPtr(dim[0] * dim[1]);
    VecDataArray<float, 2>&          uvCoords = *uvCoordsPtr.get();
    for (int i = 0; i < dim[1]; ++i)
    {
        for (int j = 0; j < dim[0]; j++)
        {
            uvCoords[i * dim[0] + j] = Vec2f(static_cast<float>(i) / dim[1], static_cast<float>(j) / dim[0]) * uvScale;
        }
    }

    clothMesh->initialize(verticesPtr, indicesPtr);
    clothMesh->setVertexTCoords("uvs", uvCoordsPtr);

    return clothMesh;
}

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
             const Vec2d        size,
             const Vec2i        dim,
             const Vec3d        pos)
{
    imstkNew<PbdObject> clothObj(name);

    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> clothMesh = makeClothGeometry(size, dim, pos, 2.0);

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0e2);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 0.05);
    pbdParams->m_uniformMassValue = size[0] * size[1] / (dim[0] * dim[1]) * 0.01;
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 10;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(clothMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    setFleshTextures(material);
    imstkNew<VisualModel> visualModel(clothMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    clothObj->addVisualModel(visualModel);
    clothObj->setPhysicsGeometry(clothMesh);
    clothObj->setCollidingGeometry(clothMesh);
    clothObj->setDynamicalModel(pbdModel);

    return clothObj;
}

///
/// \brief This example demonstrates the cloth simulation
/// using Position based dynamics
///
int
main()
{
    // Write log to stdout and file
    Logger::startLogger();

    // Setup a scene
    imstkNew<Scene>            scene("PBDCloth");
    std::shared_ptr<PbdObject> clothObj =
        makeClothObj("Cloth", Vec2d(5.0, 5.0), Vec2i(16, 16), Vec3d(0.0, 6.0, 0.0));
    scene->addSceneObject(clothObj);

    auto            planeObj =  std::make_shared<CollidingObject>("Plane");
    imstkNew<Plane> plane(Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));
    plane->setWidth(10.0);
    planeObj->setVisualGeometry(plane);
    planeObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    planeObj->setCollidingGeometry(plane);
    scene->addSceneObject(planeObj);

    // Adjust camera
    scene->getActiveCamera()->setFocalPoint(-0.116722, 1.70485, 0.625839);
    scene->getActiveCamera()->setPosition(2.25549, 8.07292, 14.8692);

    auto clothCollision = std::make_shared<PbdObjectCollision>(clothObj, planeObj, "PointSetToPlaneCD");
    scene->getCollisionGraph()->addInteraction(clothCollision);

    // Make two sphere's for indication
    imstkNew<SceneObject> clickObj("clickObj");
    imstkNew<Sphere>      clickSphere1(Vec3d(0.0, 0.0, 0.0), 0.1);
    imstkNew<Sphere>      clickSphere2(Vec3d(0.0, 0.0, 0.0), 0.1);
    clickObj->addVisualModel(std::make_shared<VisualModel>(clickSphere1));
    clickObj->addVisualModel(std::make_shared<VisualModel>(clickSphere2));
    clickObj->getVisualModel(0)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::None);
    clickObj->getVisualModel(1)->getRenderMaterial()->setShadingModel(RenderMaterial::ShadingModel::None);
    clickObj->getVisualModel(1)->getRenderMaterial()->setColor(Color::Red);
    scene->addSceneObject(clickObj);

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
        driver->setDesiredDt(0.01);

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

        // Picking allows movement along plane orthogonal to the view
        int   triangleSelected       = -1;
        Vec3d triangleSelectionPtUvw = Vec3d::Zero();

        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonPress,
            [&](MouseEvent* e)
        {
            if (e->m_buttonId == 0)
            {
                // Get mouse position (0, 1) with origin at bot left of screen
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                // To NDC coordinates
                const Vec3d rayDir = scene->getActiveCamera()->getEyeRayDir(
                        Vec2d(mousePos[0] * 2.0 - 1.0, mousePos[1] * 2.0 - 1.0));
                const Vec3d rayStart = scene->getActiveCamera()->getPosition();

                auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(clothObj->getCollidingGeometry());
                std::shared_ptr<VecDataArray<double, 3>> verticesPtr = surfMesh->getVertexPositions();
                const VecDataArray<double, 3>& vertices = *verticesPtr;
                std::shared_ptr<VecDataArray<int, 3>> indicesPtr = surfMesh->getTriangleIndices();
                const VecDataArray<int, 3>& indices = *indicesPtr;

                // Comptue the nearest triangle intersection along the picking ray
                double minDist   = IMSTK_DOUBLE_MAX;
                triangleSelected = -1;
                for (int i = 0; i < indices.size(); i++)
                {
                    const Vec3d& a = vertices[indices[i][0]];
                    const Vec3d& b = vertices[indices[i][1]];
                    const Vec3d& c = vertices[indices[i][2]];
                    if (CollisionUtils::testSegmentTriangle(rayStart, rayStart + rayDir * 1000.0, a, b, c, triangleSelectionPtUvw))
                    {
                        Vec3d iPt =
                            a * triangleSelectionPtUvw[0] +
                            b * triangleSelectionPtUvw[1] +
                            c * triangleSelectionPtUvw[2];
                        double dist = (rayStart - iPt).norm();

                        if (dist < minDist)
                        {
                            minDist = dist;
                            triangleSelected = i;
                        }
                    }
                }
            }
            });
        // Unselect/drop the sphere
        connect<MouseEvent>(viewer->getMouseDevice(), &MouseDeviceClient::mouseButtonRelease,
            [&](MouseEvent* e)
        {
            if (e->m_buttonId == 0)
            {
                triangleSelected = -1;
            }
            });

        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
        {
            // Run the model in real time
            clothObj->getPbdModel()->getConfig()->m_dt = sceneManager->getDt();
            });

        auto updatePickingFunc = std::make_shared<TaskNode>([&]()
        {
            if (triangleSelected != -1)
            {
                // Get mouses current position
                const Vec2d mousePos = viewer->getMouseDevice()->getPos();
                const Vec3d rayDir   = scene->getActiveCamera()->getEyeRayDir(
                        Vec2d(mousePos[0] * 2.0 - 1.0, mousePos[1] * 2.0 - 1.0));
                const Vec3d rayStart = scene->getActiveCamera()->getPosition();

                // Compute new intersection point along view plane this is the location to move too
                Vec3d curr_iPt;
                CollisionUtils::testRayToPlane(rayStart, rayDir, plane->getPosition(),
                        plane->getNormal(), curr_iPt);

                clickSphere1->setPosition(curr_iPt);
                clickSphere1->updatePostTransformData();

                auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(clothObj->getCollidingGeometry());
                std::shared_ptr<VecDataArray<double, 3>> verticesPtr = surfMesh->getVertexPositions();
                VecDataArray<double, 3>& vertices = *verticesPtr;
                std::shared_ptr<VecDataArray<int, 3>> indicesPtr = surfMesh->getTriangleIndices();
                const VecDataArray<int, 3>& indices = *indicesPtr;

                Vec3d& a       = vertices[indices[triangleSelected][0]];
                Vec3d& b       = vertices[indices[triangleSelected][0]];
                Vec3d& c       = vertices[indices[triangleSelected][0]];
                Vec3d prev_iPt =
                    a * triangleSelectionPtUvw[0] +
                    b * triangleSelectionPtUvw[1] +
                    c * triangleSelectionPtUvw[2];

                clickSphere2->setPosition(prev_iPt);
                clickSphere2->updatePostTransformData();

                double stiffness = 0.1;
                Vec3d diff       = curr_iPt - prev_iPt;

                a += diff * stiffness;
                b += diff * stiffness;
                c += diff * stiffness;
            }
            }, "PickingUpdate");

        // Insert a step into the model
        // Another, possibly better solution would be to add an internal constraint for it
        connect<Event>(scene, &Scene::configureTaskGraph, [&](Event*)
        {
            std::shared_ptr<TaskGraph> taskGraph = scene->getTaskGraph();
            // Compute picking update position after internal solve, but before velocities are updated
            taskGraph->insertAfter(clothObj->getPbdModel()->getSolveNode(), updatePickingFunc);
            });

        driver->start();
    }

    return 0;
}