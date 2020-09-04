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
#include "imstkCube.h"
#include "imstkGeometryUtilities.h"
#include "imstkImageData.h"
#include "imstkImplicitGeometryToImageData.h"
#include "imstkLevelSetDeformableObject.h"
#include "imstkLevelSetModel.h"
#include "imstkLight.h"
#include "imstkLogger.h"
#include "imstkMeshIO.h"
#include "imstkNew.h"
#include "imstkOpenVRDeviceClient.h"
#include "imstkPlane.h"
#include "imstkRbdConstraint.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkRigidObjectController.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshFlyingEdges.h"
#include "imstkSurfaceMeshSubdivide.h"
#include "imstkVisualModel.h"
#include "imstkVolumeRenderMaterial.h"
#include "imstkVTKOpenVRViewer.h"

#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeProperty.h>

using namespace imstk;
using namespace imstk::expiremental;

///
/// \brief Creates cloth object
///
std::shared_ptr<LevelSetDeformableObject>
makeLevelsetObj(const std::string& name)
{
    imstkNew<LevelSetDeformableObject> levelsetObj(name);

    // Setup a plane
    imstkNew<Plane> plane(Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, 1.0, 0.0));

    // Put into an image
    imstkNew<ImplicitGeometryToImageData> toSdf;
    toSdf->setInputGeometry(plane);
    Vec6d bounds;
    bounds[0] = -2.0;
    bounds[1] = 2.0;
    bounds[2] = -2.0;
    bounds[3] = 2.0;
    bounds[4] = -2.0;
    bounds[5] = 2.0;
    toSdf->setBounds(bounds);
    toSdf->setDimensions(Vec3i(150, 150, 150));
    toSdf->update();
    std::shared_ptr<ImageData> initLvlsetImage = toSdf->getOutputImage();
    //imstkNew<SignedDistanceField> sdf(initLvlsetImage);

    // Setup the Geometry (read dragon mesh)
    //std::shared_ptr<ImageData> initLvlsetImage = MeshIO::read<ImageData>("C:/Users/Andx_/Desktop/mandibleReducedSmoothened1_SDF.nii")->cast(IMSTK_DOUBLE);

    // Setup the Parameters
    imstkNew<LevelSetModelConfig> lvlSetConfig;
    lvlSetConfig->m_sparseUpdate = true;
    lvlSetConfig->m_dt = 0.001;

    // Setup the Model
    imstkNew<LevelSetModel> model;
    model->setModelGeometry(initLvlsetImage);
    model->configure(lvlSetConfig);

    // Setup the VisualModel
    imstkNew<VisualModel> visualModel(initLvlsetImage);
    imstkNew<VolumeRenderMaterial> mat;
    {
        vtkNew<vtkColorTransferFunction> color;
        color->AddRGBPoint(0.0, 0.0, 0.0, 1.0);
        mat->getVolumeProperty()->SetColor(color);
        vtkNew<vtkPiecewiseFunction> opacity;
        opacity->AddPoint(0.0, 0.0);
        opacity->AddPoint(-0.001, 1.0);
        mat->getVolumeProperty()->SetScalarOpacity(opacity);
        mat->getVolumeProperty()->SetInterpolationTypeToLinear();
        mat->getVolumeProperty()->ShadeOn();
    }
    visualModel->setRenderMaterial(mat);

    // Setup the Object
    levelsetObj->addVisualModel(visualModel);
    levelsetObj->setPhysicsGeometry(initLvlsetImage);
    levelsetObj->setCollidingGeometry(std::make_shared<SignedDistanceField>(initLvlsetImage));
    levelsetObj->setDynamicalModel(model);

    return levelsetObj;
}

std::shared_ptr<CollidingObject>
makeCollidingObject(const std::string& name)
{
    imstkNew<CollidingObject> collideObj(name);

    // Setup a plane, then cut with a sphere
    imstkNew<Plane> plane(Vec3d(0.0, 0.0, 0.0), Vec3d(0.0, 1.0, 0.0));
    /*imstkNew<Sphere> sphere(Vec3d(0.0, 0.0, 0.0), 4.0);
    imstkNew<CompositeImplicitGeometry> compGeometry;
    compGeometry->addImplicitGeometry(plane, CompositeImplicitGeometry::GeometryBoolType::UNION);
    compGeometry->addImplicitGeometry(sphere, CompositeImplicitGeometry::GeometryBoolType::DIFFERENCE);*/

    // Put into an image
    imstkNew<ImplicitGeometryToImageData> toSdf;
    toSdf->setInputGeometry(plane);
    Vec6d bounds;
    bounds[0] = -5.0;
    bounds[1] = 5.0;
    bounds[2] = -5.0;
    bounds[3] = 5.0;
    bounds[4] = -5.0;
    bounds[5] = 5.0;
    toSdf->setBounds(bounds);
    toSdf->setDimensions(Vec3i(35, 35, 35));
    toSdf->update();
    std::shared_ptr<ImageData> initLvlsetImage = toSdf->getOutputImage();

    imstkNew<SurfaceMeshFlyingEdges> isoExtract;
    isoExtract->setInputImage(initLvlsetImage);
    isoExtract->update();

    // Setup the VisualModel
    imstkNew<VisualModel> visualModel(isoExtract->getOutputMesh());

    // Setup the Object
    collideObj->addVisualModel(visualModel);
    collideObj->setCollidingGeometry(std::make_shared<SignedDistanceField>(initLvlsetImage));

    return collideObj;
}

std::shared_ptr<RigidObject2>
makeRigidObj(const std::string& name)
{
    imstkNew<RigidBodyModel2>       rbdModel;
    rbdModel->getConfig()->m_dt = 0.0005;
    rbdModel->getConfig()->m_maxNumIterations = 10;

    // Create the first rbd, plane floor
    imstkNew<RigidObject2> rigidObj(name);
    {
        // Setup a cube
        imstkNew<Cube> cubeGeom;
        cubeGeom->setWidth(0.05);

        // Setup a surface for the cube, we use this for collision
        std::shared_ptr<SurfaceMesh> cubeSurfMesh = GeometryUtils::toCubeSurfaceMesh(cubeGeom);
        // The points are used for collision so we want it to be somewhat fine
        imstkNew<SurfaceMeshSubdivide> subdiv;
        subdiv->setInputMesh(cubeSurfMesh);
        subdiv->setNumberOfSubdivisions(3);
        subdiv->update();
        std::shared_ptr<SurfaceMesh> cubeSurfMeshSubdiv = subdiv->getOutputMesh();

        // Create the visual model
        imstkNew<VisualModel> visualModel(cubeSurfMeshSubdiv);
        imstkNew<RenderMaterial> mat;
        mat->setDisplayMode(RenderMaterial::DisplayMode::Points);
        mat->setPointSize(15.0);
        visualModel->setRenderMaterial(mat);

        // Create the object
        rigidObj->addVisualModel(visualModel);
        rigidObj->setPhysicsGeometry(cubeSurfMeshSubdiv);
        rigidObj->setCollidingGeometry(cubeSurfMeshSubdiv);
        rigidObj->setVisualGeometry(cubeSurfMeshSubdiv);
        // Make the cube transform apply to the surface mesh
        //rigidObj->setPhysicsToCollidingMap(cubeSurfMesh);
        rigidObj->setDynamicalModel(rbdModel);
        rigidObj->getRigidBody()->m_mass = 100.0;
        rigidObj->getRigidBody()->setInertiaFromPointSet(cubeSurfMeshSubdiv, 0.01);
        rigidObj->getRigidBody()->m_initPos = Vec3d(0.0, 10.0, 0.0);
        //rigidObj->getRigidBody()->m_initPos = Vec3d(0.702677, 0.798054, -1.161267);
        //rigidObj->getRigidBody()->m_initOrientation = Rotd(0.8, Vec3d(1.0, 0.0, 0.0)) * Rotd(0.4, Vec3d(0.0, 1.0, 0.0));
    }
    //{
    //    std::shared_ptr<SurfaceMesh> toolMesh1 = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/Surgical Instruments/Scalpel Blade10/Scalpel Blade10.obj");
    //    imstkNew<SurfaceMeshSubdivide> subdivide;
    //    subdivide->setInputMesh(toolMesh1);
    //    subdivide->setNumberOfSubdivisions(2);
    //    subdivide->update();
    //    std::shared_ptr<SurfaceMesh> toolMesh = subdivide->getOutputMesh();
    //    toolMesh->translate(0.0, 0.0, 1.0, Geometry::TransformType::ApplyToData);
    //    toolMesh->rotate(Vec3d(0.0, 1.0, 0.0), 3.14, Geometry::TransformType::ApplyToData);
    //    toolMesh->rotate(Vec3d(1.0, 0.0, 0.0), -1.57, Geometry::TransformType::ApplyToData);
    //    toolMesh->scale(0.07, Geometry::TransformType::ApplyToData);

    //    imstkNew<VisualModel>    visualModel(toolMesh);
    //    imstkNew<RenderMaterial> material;
    //    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    //    material->setShadingModel(RenderMaterial::ShadingModel::PBR);
    //    material->setMetalness(0.9f);
    //    material->setRoughness(0.2f);
    //    visualModel->setRenderMaterial(material);

    //   // Create the object
    //   rigidObj->addVisualModel(visualModel);
    //   rigidObj->setPhysicsGeometry(toolMesh);
    //   rigidObj->setCollidingGeometry(toolMesh);
    //   rigidObj->setVisualGeometry(toolMesh);
    //   rigidObj->setDynamicalModel(rbdModel);
    //   rigidObj->getRigidBody()->m_mass = 100.0;
    //   rigidObj->getRigidBody()->setInertiaFromPointSet(toolMesh, 0.01);
    //   rigidObj->getRigidBody()->m_initPos = Vec3d(0.0, 4.0, 0.0);
    //   //rigidObj->getRigidBody()->m_initOrientation = Rotd(0.8, Vec3d(1.0, 0.0, 0.0)) * Rotd(0.4, Vec3d(0.0, 1.0, 0.0));
    //}
    return rigidObj;
}

std::shared_ptr<VisualObject>
makeVisualObject(const std::string& name, const std::string& fileName, const Vec3d pos, const double scale = 1.0)
{
    // Create the first rbd, plane floor
    imstkNew<VisualObject> visualObj(name);
    {
        std::shared_ptr<SurfaceMesh> surfMesh = MeshIO::read<SurfaceMesh>(fileName);
        surfMesh->translate(pos, Geometry::TransformType::ApplyToData);
        surfMesh->scale(scale, Geometry::TransformType::ApplyToData);
        visualObj->addVisualModel(std::make_shared<VisualModel>(surfMesh));
    }
    return visualObj;
}

///
/// \brief This example demonstrates evolution of a levelset
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("LevelsetDeformable");
    scene->getConfig()->writeTaskGraph = true;

   /* std::shared_ptr<LevelSetDeformableObject> lvlSetObj = makeLevelsetObj("LevelSetObj");
    scene->addSceneObject(lvlSetObj);*/

    std::shared_ptr<RigidObject2> rbdObj = makeRigidObj("RigidObj");
    scene->addSceneObject(rbdObj);

    //scene->addSceneObject(makeVisualObject("modelTest", "C:/Users/Andx_/Desktop/torso.stl", Vec3d(0.0, 10.0, 0.0), 0.001));

   /* imstkNew<RigidObjectLevelSetCollisionPair> interaction(rbdObj, lvlSetObj);
    scene->getCollisionGraph()->addInteraction(interaction);*/


    // Light (white)
    imstkNew<DirectionalLight> whiteLight("whiteLight");
    whiteLight->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    whiteLight->setIntensity(1.0);
    scene->addLight(whiteLight);

    // Adjust camera
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setPosition(0.0, 10.0, -10.0);

    {
        // Add a module to run the viewer
        imstkNew<VTKOpenVRViewer> viewer("VRViewer");
        viewer->setActiveScene(scene);

        // Add a module to run the scene
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // Start/stop scene with the view

        // Add a VR controller for the sceneobject
        imstkNew<RigidObjectController> controller(rbdObj, viewer->getVRDeviceClient(OPENVR_RIGHT_CONTROLLER));
        scene->addController(controller);

        // Start running
        viewer->requestStatus(ThreadStatus::Running);
        viewer->start();
    }

    return 0;
}
