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
#include "imstkCleanMesh.h"
#include "imstkCollisionGraph.h"
#include "imstkDataArray.h"
#include "imstkImageData.h"
#include "imstkImageDistanceTransform.h"
#include "imstkLight.h"
#include "imstkMeshIO.h"
#include "imstkNew.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSignedDistanceField.h"
#include "imstkSimulationManager.h"
#include "imstkSPHModel.h"
#include "imstkSPHObject.h"
#include "imstkSphObjectCollisionPair.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshDistanceTransform.h"
#include "imstkSurfaceMeshImageMask.h"
#include "imstkViewer.h"
#include "imstkVisualModel.h"

using namespace imstk;
using namespace imstk::expiremental;

///
/// \brief Generate a volume of fluid with the specified SurfaceMesh
///
std::shared_ptr<StdVectorOfVec3d>
generateFluidVolume(const double particleRadius, std::shared_ptr<SurfaceMesh> spawnVolumeSurface)
{
    Vec3d minima, maxima;
    spawnVolumeSurface->computeBoundingBox(minima, maxima);

    double particleDiameter = particleRadius * 2.0;
    const Vec3d  size = (maxima - minima) + Vec3d(particleDiameter, particleDiameter, particleDiameter);
    Vec3i  dim = size.cwiseProduct(Vec3d(1.0 / particleDiameter, 1.0 / particleDiameter, 1.0 / particleDiameter)).cast<int>();

    imstkNew<SurfaceMeshImageMask> makeBinaryMask;
    makeBinaryMask->setInputMesh(spawnVolumeSurface);
    makeBinaryMask->setDimensions(dim[0], dim[1], dim[2]);
    makeBinaryMask->update();
    imstkNew<ImageDistanceTransform> distTransformFromMask;
    distTransformFromMask->setInputImage(makeBinaryMask->getOutputImage());
    distTransformFromMask->update();

    DataArray<float>& scalars = *std::dynamic_pointer_cast<DataArray<float>>(distTransformFromMask->getOutputImage()->getScalars());
    const Vec3i& dim1    = makeBinaryMask->getOutputImage()->getDimensions();
    const Vec3d& spacing = makeBinaryMask->getOutputImage()->getSpacing();
    const Vec3d& shift = makeBinaryMask->getOutputImage()->getOrigin() + spacing * 0.5;

    imstkNew<StdVectorOfVec3d> particles;
    particles->reserve(dim1[0] * dim1[1] * dim1[2]);

    int i = 0;
    for (int z = 0; z < dim1[2]; z++)
    {
        for (int y = 0; y < dim1[1]; y++)
        {
            for (int x = 0; x < dim1[0]; x++, i++)
            {
                if (scalars[i] < -particleDiameter * 3.0)
                {
                    particles->push_back(Vec3d(x, y, z).cwiseProduct(spacing) + shift);
                }
            }
        }
    }
    particles->shrink_to_fit();
    return particles;
}

std::shared_ptr<SPHObject>
makeSPHObject(const std::string& name, const double particleRadius)
{
    // Create the sph object
    imstkNew<SPHObject> fluidObj(name);

    // Setup the Geometry
    auto fluidSpawnVolumeSurf = MeshIO::read<SurfaceMesh>("C:/Users/Andx_/Desktop/human model/vesselsCut.stl");
    std::shared_ptr<StdVectorOfVec3d> particles = generateFluidVolume(particleRadius, fluidSpawnVolumeSurf);
    LOG(INFO) << "Number of particles: " << particles->size();
    imstkNew<PointSet> fluidGeometry;
    fluidGeometry->initialize(*particles);

    // Setup the Parameters
    imstkNew<SPHModelConfig> sphParams(particleRadius);
    sphParams->m_bNormalizeDensity = true;
    sphParams->m_kernelOverParticleRadiusRatio = 6.0;
    sphParams->m_viscosityCoeff = 0.8;
    sphParams->m_surfaceTensionStiffness = 5.0;
    sphParams->m_frictionBoundary = 0.1;

    // Setup the Model
    imstkNew<SPHModel> sphModel;
    sphModel->setModelGeometry(fluidGeometry);
    sphModel->configure(sphParams);
    sphModel->setTimeStepSizeType(TimeSteppingType::RealTime);

    // Setup the VisualModel
    imstkNew<VisualModel>    fluidVisualModel(fluidGeometry.get());
    imstkNew<RenderMaterial> fluidMaterial;
    fluidMaterial->setDisplayMode(RenderMaterial::DisplayMode::Fluid);
    fluidMaterial->setPointSize(static_cast<float>(particleRadius) * 3.0f); // For fluids
    //fluidMaterial->setPointSize(static_cast<float>(particleRadius) * 800.0f); // For spheres
    fluidVisualModel->setRenderMaterial(fluidMaterial);

    // Setup the Object
    fluidObj->setDynamicalModel(sphModel);
    fluidObj->addVisualModel(fluidVisualModel);
    fluidObj->setCollidingGeometry(fluidGeometry);
    fluidObj->setPhysicsGeometry(fluidGeometry);

    return fluidObj;
}

static std::shared_ptr<CollidingObject>
makeCollidingObject(const std::string& name, const Vec3d& position)
{
    // Create the pbd object
    imstkNew<CollidingObject> collidingObj(name);

    // Setup the Geometry (read dragon mesh)
    auto fullBodyMesh = MeshIO::read<SurfaceMesh>("C:/Users/Andx_/Desktop/human model/humanWithHead.stl");
    auto collisionMesh = MeshIO::read<SurfaceMesh>("C:/Users/Andx_/Desktop/human model/invertedVessels.stl");

    imstkNew<CleanMesh> cleanMesh;
    cleanMesh->setInputMesh(collisionMesh);
    cleanMesh->update();

    LOG(INFO) << "Computing SDF";
    imstkNew<SurfaceMeshDistanceTransform> computeSdf;
    computeSdf->setInputMesh(cleanMesh->getOutputMesh());
    computeSdf->setDimensions(150, 150, 150);
    computeSdf->update();
    LOG(INFO) << "SDF Complete";

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Surface);
    material->setOpacity(0.2f);
    material->setDiffuseColor(Color(71.0 / 255.0, 61.0 / 255.0, 57.0 / 255.0, 1.0));
    imstkNew<VisualModel> surfMeshModel(fullBodyMesh);
    surfMeshModel->setRenderMaterial(material);
    //surfMeshModel->hide();

    // Setup the Object
    collidingObj->addVisualModel(surfMeshModel);
    collidingObj->setCollidingGeometry(std::make_shared<SignedDistanceField>(computeSdf->getOutputImage()));

    MeshIO::write(computeSdf->getOutputImage(), "C:/Users/Andx_/Desktop/test.nii");

    return collidingObj;
}

///
/// \brief This example demonstrates interaction simulates SPH fluid contained within
/// an SDF vessel
///
int
main()
{
    imstkNew<SimulationManager> simManager;
    auto                        scene = simManager->createNewScene("Vessel");

    // Setup the scene
    {
        scene->getCamera()->setPosition(1.5, 5.0, 1.0);
        scene->getCamera()->setFocalPoint(0.0, 4.5, 0.0);
        //scene->getConfig()->taskTimingEnabled = true;

        // Static Dragon object
        std::shared_ptr<CollidingObject> dragonObj = makeCollidingObject("Vessel", Vec3d(0.0, 0.0, 0.0));
        scene->addSceneObject(dragonObj);

        // SPH fluid box overtop the dragon
        std::shared_ptr<SPHObject> sphObj = makeSPHObject("Fluid", 0.003);
        scene->addSceneObject(sphObj);

        // Interaction
        imstkNew<SphObjectCollisionPair> collisionInteraction(sphObj, dragonObj, CollisionDetection::Type::PointSetToImplicit);
        scene->getCollisionGraph()->addInteraction(collisionInteraction);

        // Light
        imstkNew<DirectionalLight> light("light");
        light->setDirection(0.0, 1.0, -1.0);
        light->setIntensity(1);
        scene->addLight(light);
    }

    simManager->setActiveScene(scene);
    simManager->getViewer()->setBackgroundColors(Vec3d(0.3285, 0.3285, 0.6525), Vec3d(0.13836, 0.13836, 0.2748), true);
    simManager->start(SimulationStatus::Paused);

    return 0;
}
