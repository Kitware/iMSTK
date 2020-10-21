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

#include "imstkBackwardEuler.h"
#include "imstkCamera.h"
#include "imstkFeDeformableObject.h"
#include "imstkFEMDeformableBodyModel.h"
#include "imstkImageData.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLight.h"
#include "imstkMeshIO.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOneToOneMap.h"
#include "imstkPbdConstraint.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSPHModel.h"
#include "imstkSPHObject.h"
#include "imstkSurfaceMesh.h"
#include "imstkTaskGraphVizWriter.h"
#include "imstkTetrahedralMesh.h"
#include "imstkTetraTriangleMap.h"
#include "imstkViewer.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Generate a box-shape fluid object
/// \param radius of SPH particles/used for spacing
/// \param center of the SPH fluid box
/// \param size of the SPH fluid box
///
std::shared_ptr<StdVectorOfVec3d>
generateBoxShapeFluid(const double particleRadius, const Vec3d& boxCenter, const Vec3d& boxSize)
{
    double bounds[6] =
    {
        boxCenter.x() - boxSize.x() * 0.5,
        boxCenter.x() + boxSize.x() * 0.5,
        boxCenter.y() - boxSize.y() * 0.5,
        boxCenter.y() + boxSize.y() * 0.5,
        boxCenter.z() - boxSize.z() * 0.5,
        boxCenter.z() + boxSize.z() * 0.5
    };

    const double spacing = 2.0 * particleRadius;
    const Vec3i  dim     = {
        static_cast<int>(boxSize.x() / spacing),
        static_cast<int>(boxSize.y() / spacing),
        static_cast<int>(boxSize.z() / spacing) };

    imstkNew<StdVectorOfVec3d> particles;
    particles->reserve(dim[0] * dim[1] * dim[2]);

    for (double z = bounds[4]; z < bounds[5]; z += spacing)
    {
        for (double y = bounds[2]; y < bounds[3]; y += spacing)
        {
            for (double x = bounds[0]; x < bounds[1]; x += spacing)
            {
                particles->push_back(Vec3d(x, y, z));
            }
        }
    }

    return particles;
}

///
/// \brief Generate an SPH Box Object
/// \param name of the object
/// \param radius of SPH particles
/// \param shift/position of the fluid box
/// \parma dimensions of the fluid box
///
std::shared_ptr<SPHObject>
makeSPHBoxObject(const std::string& name, const double particleRadius, const Vec3d& boxShift, const Vec3d& boxSize)
{
    // Create the sph object
    imstkNew<SPHObject> fluidObj(name);

    // Setup the Geometry
    std::shared_ptr<StdVectorOfVec3d> particles = generateBoxShapeFluid(particleRadius, boxShift, boxSize);
    LOG(INFO) << "Number of particles: " << particles->size();
    imstkNew<PointSet> fluidGeometry;
    fluidGeometry->initialize(*particles);

    // Setup the Parameters
    imstkNew<SPHModelConfig> sphParams(particleRadius);
    sphParams->m_bNormalizeDensity = true;
    sphParams->m_kernelOverParticleRadiusRatio = 6.0;
    sphParams->m_viscosityCoeff = 0.5;
    sphParams->m_surfaceTensionStiffness = 5.0;

    // Setup the Model
    imstkNew<SPHModel> sphModel;
    sphModel->setModelGeometry(fluidGeometry);
    sphModel->configure(sphParams);
    sphModel->setTimeStepSizeType(TimeSteppingType::RealTime);

    // Setup the VisualModel
    imstkNew<VisualModel>    fluidVisualModel(fluidGeometry.get());
    imstkNew<RenderMaterial> fluidMaterial;
    fluidMaterial->setDisplayMode(RenderMaterial::DisplayMode::Fluid);
    fluidMaterial->setVertexColor(Color::Orange);
    fluidMaterial->setPointSize(static_cast<double>(particleRadius));
    fluidVisualModel->setRenderMaterial(fluidMaterial);

    // Setup the Object
    fluidObj->addVisualModel(fluidVisualModel);
    fluidObj->setCollidingGeometry(fluidGeometry);
    fluidObj->setDynamicalModel(sphModel);
    fluidObj->setPhysicsGeometry(fluidGeometry);

    return fluidObj;
}

///
/// \brief Generate a Deformable object simulated with FEM
/// \param name of the object
/// \param position of the object
///
static std::shared_ptr<FeDeformableObject>
makeFEDragonObject(const std::string& name, const Vec3d& position)
{
    // Create the sph object
    imstkNew<FeDeformableObject> object(name);

    // Setup the Geometry
    auto tetMesh = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    tetMesh->translate(position, Geometry::TransformType::ApplyToData);
    imstkNew<SurfaceMesh> surfMesh;
    tetMesh->extractSurfaceMesh(surfMesh, true);

    // Setup the Parameters
    imstkNew<FEMModelConfig> config;
    config->m_fixedNodeIds = { 50, 126, 177 };

    // Setup the Model
    imstkNew<FEMDeformableBodyModel> dynaModel;
    dynaModel->configure(config);
    dynaModel->setTimeStepSizeType(TimeSteppingType::Fixed);
    dynaModel->setModelGeometry(tetMesh);
    imstkNew<BackwardEuler> timeIntegrator(0.01);
    dynaModel->setTimeIntegrator(timeIntegrator);

    // Setup the VisualModel
    imstkNew<VisualModel>    surfMeshModel(surfMesh.get());
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    surfMeshModel->setRenderMaterial(material);

    // Setup the Object
    object->addVisualModel(surfMeshModel);
    object->setPhysicsGeometry(tetMesh);
    object->setPhysicsToVisualMap(std::make_shared<OneToOneMap>(tetMesh, surfMesh));
    object->setDynamicalModel(dynaModel);

    return object;
}

///
/// \brief Generate a Deformable object simulated with PBD
/// \param name of the object
/// \param position of the object
///
static std::shared_ptr<PbdObject>
makePBDDragonObject(const std::string& name, const Vec3d& position)
{
    // Create the pbd object
    imstkNew<PbdObject> pbdObj(name);

    // Setup the Geometry (read dragon mesh)
    auto highResSurfMesh = MeshIO::read<SurfaceMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.obj");
    auto coarseTetMesh   = MeshIO::read<TetrahedralMesh>(iMSTK_DATA_ROOT "/asianDragon/asianDragon.veg");
    highResSurfMesh->translate(position, Geometry::TransformType::ApplyToData);
    coarseTetMesh->translate(position, Geometry::TransformType::ApplyToData);
    imstkNew<SurfaceMesh> coarseSurfMesh;
    coarseTetMesh->extractSurfaceMesh(coarseSurfMesh, true);

    // Setup the Parameters
    imstkNew<PBDModelConfig> pbdParams;
    pbdParams->m_femParams->m_YoungModulus = 1000.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.3;
    pbdParams->m_fixedNodeIds = { 50, 126, 177 };
    pbdParams->enableFEMConstraint(PbdConstraint::Type::FEMTet, PbdFEMConstraint::MaterialType::StVK);
    pbdParams->m_uniformMassValue = 1.0;
    pbdParams->m_gravity    = Vec3d(0, -9.8, 0);
    pbdParams->m_defaultDt  = 0.01;
    pbdParams->m_iterations = 10;
    pbdParams->collisionParams->m_proximity = 0.3;
    pbdParams->collisionParams->m_stiffness = 0.1;

    // Setup the Model
    imstkNew<PbdModel> model;
    model->setModelGeometry(coarseTetMesh);
    model->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    imstkNew<VisualModel> surfMeshModel(highResSurfMesh);
    surfMeshModel->setRenderMaterial(material);

    // Setup the Object
    pbdObj->addVisualModel(surfMeshModel);
    pbdObj->setCollidingGeometry(coarseSurfMesh);
    pbdObj->setPhysicsGeometry(coarseTetMesh);
    pbdObj->setPhysicsToCollidingMap(std::make_shared<OneToOneMap>(coarseTetMesh, coarseSurfMesh));
    pbdObj->setPhysicsToVisualMap(std::make_shared<TetraTriangleMap>(coarseTetMesh, highResSurfMesh));
    pbdObj->setDynamicalModel(model);

    return pbdObj;
}

///
/// \brief This example demonstrates 3 different models running at the same time
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("SPH-FEM-PBD");

    // Setup the scene
    {
        scene->getConfig()->taskTimingEnabled = true;
        scene->getActiveCamera()->setPosition(0.0, 2.0, 25.0);

        // Deformable Pbd Dragon
        std::shared_ptr<PbdObject> pbdDragon = makePBDDragonObject("PBDDragon", Vec3d(-5.0, 0.0, 0.0));
        scene->addSceneObject(pbdDragon);

        // Deformable FEM dragon
        std::shared_ptr<FeDeformableObject> feDragon = makeFEDragonObject("FEDragon", Vec3d(0.0, 0.0, 0.0));
        scene->addSceneObject(feDragon);

        // SPH fluid box overtop the dragon
        std::shared_ptr<SPHObject> sphFluidBox = makeSPHBoxObject("Box", 0.1, Vec3d(5.0, 7.0, 0.0), Vec3d(3.0, 7.0, 3.0));
        scene->addSceneObject(sphFluidBox);

        // Light
        imstkNew<DirectionalLight> light("light");
        light->setDirection(0.0, -1.0, -1.0);
        light->setIntensity(1);
        scene->addLight(light);
    }

    // Run the simulation
    {
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);
        viewer->setBackgroundColors(Vec3d(0.3285, 0.3285, 0.6525), Vec3d(0.13836, 0.13836, 0.2748), true);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer

        // Add mouse and keyboard controls to the viewer
        {
            imstkNew<MouseSceneControl> mouseControl(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            viewer->addControl(mouseControl);

            imstkNew<KeyboardSceneControl> keyControl(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setViewer(viewer);
            viewer->addControl(keyControl);
        }

        // Start viewer running, scene as paused
        sceneManager->requestStatus(ThreadStatus::Paused);
        viewer->start();
    }

    imstkNew<TaskGraphVizWriter> writer;
    writer->setFileName("sphFemPbdInteractionTaskGraph.svg");
    writer->setHighlightCriticalPath(true);
    writer->setInput(scene->getTaskGraph());
    writer->write();

    return 0;
}
