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
#include "imstkDirectionalLight.h"
#include "imstkImageDistanceTransform.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSignedDistanceField.h"
#include "imstkSimulationManager.h"
#include "imstkSPHModel.h"
#include "imstkSPHObject.h"
#include "imstkSphObjectCollision.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshDistanceTransform.h"
#include "imstkSurfaceMeshImageMask.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Generate a volume of fluid with the specified SurfaceMesh
///
static std::shared_ptr<VecDataArray<double, 3>>
generateFluidVolume(const double particleRadius, std::shared_ptr<SurfaceMesh> spawnSurfaceVolume)
{
    Vec3d minima, maxima;
    spawnSurfaceVolume->computeBoundingBox(minima, maxima);

    double      particleDiameter = particleRadius * 2.0;
    const Vec3d size = (maxima - minima) + Vec3d(particleDiameter, particleDiameter, particleDiameter);
    Vec3i       dim  = size.cwiseProduct(Vec3d(1.0 / particleDiameter, 1.0 / particleDiameter, 1.0 / particleDiameter)).cast<int>();

    // Compute the binary mask
    imstkNew<SurfaceMeshImageMask> makeBinaryMask;
    makeBinaryMask->setInputMesh(spawnSurfaceVolume);
    makeBinaryMask->setDimensions(dim[0], dim[1], dim[2]);
    makeBinaryMask->update();

    // Compute the DT (won't perfectly conform to surface as we used a binary mask)
    imstkNew<ImageDistanceTransform> distTransformFromMask;
    distTransformFromMask->setInputImage(makeBinaryMask->getOutputImage());
    distTransformFromMask->update();

    std::shared_ptr<DataArray<float>> scalarsPtr = std::dynamic_pointer_cast<DataArray<float>>(distTransformFromMask->getOutputImage()->getScalars());
    const DataArray<float>&           scalars    = *scalarsPtr;
    const Vec3i&                      dim1       = makeBinaryMask->getOutputImage()->getDimensions();
    const Vec3d&                      spacing    = makeBinaryMask->getOutputImage()->getSpacing();
    const Vec3d&                      shift      = makeBinaryMask->getOutputImage()->getOrigin() + spacing * 0.5;
    const double                      threshold  = particleDiameter * 1.0; // How far from the boundary to accept particles

    imstkNew<VecDataArray<double, 3>> particles;
    particles->reserve(dim1[0] * dim1[1] * dim1[2]);

    int i = 0;
    for (int z = 0; z < dim1[2]; z++)
    {
        for (int y = 0; y < dim1[1]; y++)
        {
            for (int x = 0; x < dim1[0]; x++, i++)
            {
                if (x > 1 && y > 1 && z > 1 && scalars[i] < -threshold)
                {
                    particles->push_back(Vec3d(x, y, z).cwiseProduct(spacing) + shift);
                }
            }
        }
    }
    particles->squeeze();
    return particles;
}

static std::shared_ptr<SphObject>
makeSPHObject(const std::string& name, const double particleRadius, const double particleSpacing)
{
    // Create the sph object
    imstkNew<SphObject> fluidObj(name);

    // Setup the Geometry
    auto spawnMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/legs/femoralArteryCut.stl");
    // By spacing them slightly closer we can induce larger compression at the start
    std::shared_ptr<VecDataArray<double, 3>> particles = generateFluidVolume(particleSpacing, spawnMesh);
    LOG(INFO) << "Number of particles: " << particles->size();
    imstkNew<PointSet> fluidGeometry;
    fluidGeometry->initialize(particles);

    // Setup the Parameters
    imstkNew<SphModelConfig> sphParams(particleRadius);
    sphParams->m_bNormalizeDensity = true;
    sphParams->m_kernelOverParticleRadiusRatio = 6.0;
    sphParams->m_surfaceTensionStiffness       = 5.0;
    sphParams->m_frictionBoundary = 0.1;

    // Setup the Model
    imstkNew<SphModel> sphModel;
    sphModel->setModelGeometry(fluidGeometry);
    sphModel->configure(sphParams);
    sphModel->setTimeStepSizeType(TimeSteppingType::RealTime);

    // Setup the VisualModel
    imstkNew<VisualModel> fluidVisualModel;
    fluidVisualModel->setGeometry(fluidGeometry);
    imstkNew<RenderMaterial> fluidMaterial;
    fluidMaterial->setDisplayMode(RenderMaterial::DisplayMode::Fluid);
    fluidMaterial->setPointSize(particleRadius * 2.0f); // For fluids
    //fluidMaterial->setDisplayMode(RenderMaterial::DisplayMode::Points);
    //fluidMaterial->setPointSize(particleRadius * 1200.0f); // For points (todo: remove view dependence)
    fluidVisualModel->setRenderMaterial(fluidMaterial);

    // Setup the Object
    fluidObj->setDynamicalModel(sphModel);
    fluidObj->addVisualModel(fluidVisualModel);
    fluidObj->setCollidingGeometry(fluidGeometry);
    fluidObj->setPhysicsGeometry(fluidGeometry);

    return fluidObj;
}

static std::shared_ptr<CollidingObject>
makeLegs(const std::string& name)
{
    // Create the pbd object
    imstkNew<CollidingObject> legsObj(name);

    // Setup the Geometry (read dragon mesh)
    auto legsMesh      = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/legs/legsCutaway.stl");
    auto bonesMesh     = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/legs/legsBones.stl");
    auto femoralMesh   = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/legs/femoralArtery.stl");
    auto collisionMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/legs/femoralArteryCut.stl");

    // Setup the Legs VisualModel
    imstkNew<VisualModel> legsMeshModel;
    legsMeshModel->setGeometry(legsMesh);
    imstkNew<RenderMaterial> legsMaterial;
    legsMaterial->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    legsMaterial->setOpacity(0.85f);
    legsMaterial->setDiffuseColor(Color(0.8, 0.688, 0.396));
    legsMeshModel->setRenderMaterial(legsMaterial);

    // Setup the Bones VisualModel
    imstkNew<VisualModel> bonesMeshModel;
    bonesMeshModel->setGeometry(bonesMesh);
    imstkNew<RenderMaterial> bonesMaterial;
    bonesMaterial->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    bonesMaterial->setDiffuseColor(Color(0.538, 0.538, 0.538));
    bonesMeshModel->setRenderMaterial(bonesMaterial);

    // Setup the Femoral VisualModel
    imstkNew<VisualModel> femoralMeshModel;
    femoralMeshModel->setGeometry(femoralMesh);
    imstkNew<RenderMaterial> femoralMaterial;
    femoralMaterial->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    femoralMaterial->setOpacity(0.2f);
    femoralMaterial->setDiffuseColor(Color(0.8, 0.119, 0.180));
    femoralMeshModel->setRenderMaterial(femoralMaterial);

    // Setup the Object
    legsObj->addVisualModel(legsMeshModel);
    legsObj->addVisualModel(bonesMeshModel);
    legsObj->addVisualModel(femoralMeshModel);

    LOG(INFO) << "Computing SDF";
    imstkNew<SurfaceMeshDistanceTransform> computeSdf;
    computeSdf->setInputMesh(collisionMesh);
    computeSdf->setDimensions(100, 100, 100);
    Vec3d min, max;
    collisionMesh->computeBoundingBox(min, max);
    const Vec3d size = max - min;
    Vec6d       bounds;
    bounds[0] = min[0] - size[0] * 0.25;
    bounds[1] = max[0] + size[0] * 0.25;
    bounds[2] = min[1] - size[1] * 0.25;
    bounds[3] = max[1] + size[1] * 0.25;
    bounds[4] = min[2] - size[2] * 0.25;
    bounds[5] = max[2] + size[2] * 0.25;
    computeSdf->setBounds(bounds);
    computeSdf->update();
    LOG(INFO) << "SDF Complete";
    legsObj->setCollidingGeometry(std::make_shared<SignedDistanceField>(computeSdf->getOutputImage()));

    return legsObj;
}

///
/// \brief This example demonstrates interaction simulates SPH fluid contained within
/// an SDF vessel
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("Vessel");

    // Setup the scene
    {
        // Static Dragon object
        std::shared_ptr<CollidingObject> legsObj = makeLegs("Legs");
        scene->addSceneObject(legsObj);

        // Position the camera
        //const Vec6d& bounds = std::dynamic_pointer_cast<SignedDistanceField>(legsObj->getCollidingGeometry())->getBounds();
        //const Vec3d  center = (Vec3d(bounds[0], bounds[2], bounds[4]) + Vec3d(bounds[1], bounds[3], bounds[5])) * 0.5;
        scene->getActiveCamera()->setPosition(3.25, 1.6, 3.38);
        scene->getActiveCamera()->setFocalPoint(-2.05, 1.89, -1.32);
        scene->getActiveCamera()->setViewUp(-0.66, 0.01, 0.75);

        // SPH fluid box overtop the dragon
        std::shared_ptr<SphObject> sphObj = makeSPHObject("Fluid", 0.004, 0.0035);
        scene->addSceneObject(sphObj);

        // Interaction
        scene->addInteraction(
            std::make_shared<SphObjectCollision>(sphObj, legsObj));

        // Light
        imstkNew<DirectionalLight> light;
        light->setDirection(0.0, 1.0, -1.0);
        light->setIntensity(1.0);
        scene->addLight("light0", light);
    }

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);
        viewer->setBackgroundColors(Color(0.3285, 0.3285, 0.6525), Color(0.13836, 0.13836, 0.2748), true);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->setExecutionType(Module::ExecutionType::PARALLEL);
        sceneManager->pause();

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);

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
