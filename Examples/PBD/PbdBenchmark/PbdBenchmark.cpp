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
#include "imstkImageData.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkMeshIO.h"
#include "imstkMeshToMeshBruteForceCD.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkOneToOneMap.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRbdConstraint.h"
#include "imstkRenderMaterial.h"
#include "imstkRigidBodyModel2.h"
#include "imstkRigidObject2.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"


using namespace imstk;

///
/// \brief Creates a tetraheral grid
/// \param physical dimension of domain
/// \param dimensions of tetrahedral grid
/// \param center of grid
///

static std::shared_ptr<TetrahedralMesh>
makeTetGrid(const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    // Create mesh for the benchmark
    imstkNew<TetrahedralMesh> benchmarkMesh;

    // Using a 3D mesh
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

    // Ensure correct windings (positive volume)
    for (int i = 0; i < indices.size(); i++)
    {
        if (tetVolume(vertices[indices[i][0]], vertices[indices[i][1]], vertices[indices[i][2]], vertices[indices[i][3]]) < 0.0)
        {
            std::swap(indices[i][0], indices[i][2]);
        }
    }

    benchmarkMesh->initialize(verticesPtr, indicesPtr);
    benchmarkMesh->setVertexTCoords("uvs", uvCoordsPtr);

    return benchmarkMesh;
}


///
/// \brief Creates tissue object
/// \param name
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of tissue block
///
static std::shared_ptr<PbdObject>
makePbdObj(const std::string& name,
              const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    imstkNew<PbdObject> prismObj(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(size, dim, center);
    

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;

    // Use FEMTet constraints (for now)
    // pbdParams->m_femParams->m_YoungModulus = 1.0;
    // pbdParams->m_femParams->m_PoissonRatio = 0.4;
    // pbdParams->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);

    // Use distance plus dihedral angle
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 10.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 10.0);


    pbdParams->m_doPartitioning   = true;
    pbdParams->m_uniformMassValue = 0.05;
    pbdParams->m_gravity    = Vec3d(0.0, -9.81 , 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = 10;
    pbdParams->m_viscousDampingCoeff = 0.0;

    // Fix the y=0 plane
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                if (y == 0)
                {
                    pbdParams->m_fixedNodeIds.push_back(x + dim[0] * (y + dim[1] * z));
                }
            }
        }
    }

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(prismMesh);
    pbdModel->configure(pbdParams);


    // Render the wireframe, take 2
    imstkNew<RenderMaterial> wire;
    wire->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    wire->setBackFaceCulling(false);
   

    // wire->ShadingModel(RenderMaterial::ShadingModel::None);

    // Add a visual model
    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(prismMesh);
    visualModel->setRenderMaterial(wire);
    prismObj->addVisualModel(visualModel);



    // Setup the Object
    prismObj->setPhysicsGeometry(prismMesh);
    prismObj->setDynamicalModel(pbdModel);

    return prismObj;
}



///
/// \brief This benchmark test the performance of a baisc PBD entity
///
int
main()
{


    /* Goals
        1.  Set up a simple cube volumetric tetrahedral mesh
        2.  Use the Tet PBD solver with gravity loading conditions with 
            fixed bottom BCs
        3.  Variations on a theme, e.g, switch solvers.
        4.  Repeat for surface meshes. 

        Order of Operations
        1. Start logger
        2. Create scene
        3. set active camera
        4. Create prism opject
            4.1 Make Mesh
            4.2 Make wireframe
        5. Add light
        6. Run simulation

        Questions?
        1. Can I force constant time steps?
            Fixed timesteps by default. 

        2. Time integrations schemes? 
            Forward Euler for PBD
            Fancy one for SPH
            FEM has a few to look into

        3. Is there a way to have the sim run to an exact time?
            Dont actually need a driver. All I need is to call:
                scenemanager initialize
                for(inti=0; i<*numsteps*; i++)
                    scene.advance(dt)

        4. Analytic solutions? Any tips?
            None yet. 

        5. Solver (volume+distance) seems cool with tet inversion. How does that work?
            Depends on the solver. Check out modified neo-hookian

        6. Restitution value in PBD object collision. Friction and restitution on contact
            Inverse dashpot mode. Should be bouncy but not bouncy. Probably my fault. 


    */

    std::cout<<"Hey look the code does this at least"<<std::endl;
    
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    // Setup the scene
    imstkNew<Scene> scene("PbdBenchmark");
    scene->getActiveCamera()->setPosition(0.12, 4.51, 16.51);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);


    // Create a mesh
    std::shared_ptr<PbdObject> prismObj = makePbdObj("Prism",
        Vec3d(8.0, 3.0, 8.0), Vec3i(8, 3, 8), Vec3d(0.0, 1.0, 0.0));
    scene->addSceneObject(prismObj);


    // Light
    imstkNew<DirectionalLight> light;
    light->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
    light->setIntensity(1);
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
        driver->setDesiredDt(0.001);

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