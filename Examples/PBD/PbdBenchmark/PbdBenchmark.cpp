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
/// \param physical dimension of tissue
/// \param dimensions of tetrahedral grid used for tissue
/// \param center of grid
///
static std::shared_ptr<TetrahedralMesh>
makeTetGrid(const Vec3d& size, const Vec3i& dim, const Vec3d& center)
{
    imstkNew<TetrahedralMesh> prismMesh;

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

    prismMesh->initialize(verticesPtr, indicesPtr);
    prismMesh->setVertexTCoords("uvs", uvCoordsPtr);

    return prismMesh;
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

    // Use volume+distance constraints, worse results. More performant (can use larger mesh)
    // Some stiffness parameters are bounded
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 1.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0);

    pbdParams->m_doPartitioning   = true;
    pbdParams->m_uniformMassValue = 0.05;
    pbdParams->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = 3;
    pbdParams->m_viscousDampingCoeff = 0.03;

    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                // if (x == 0 || /*z == 0 ||*/ x == dim[0] - 1 /*|| z == dim[2] - 1*/)
                if (y == dim[1]-1)
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

    // Setup the material
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    // Add a visual model to render the surface of the tet mesh
    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(prismMesh);
    visualModel->setRenderMaterial(material);
    prismObj->addVisualModel(visualModel);


    // Setup the Object
    prismObj->setPhysicsGeometry(prismMesh);
    prismObj->setDynamicalModel(pbdModel);

    return prismObj;
}

//
/// \brief Creates PBD-FEM object of a volume mesh
/// \param name
/// \param physical dimension of block
/// \param dimensions of tetrahedral grid used for block
/// \param center of block
///
static std::shared_ptr<PbdObject>
makePbdFemObjVolume(const std::string& name,
           const Vec3d& size, 
           const Vec3i& dim, 
           const Vec3d& center,
           const int numIter)
{
    imstkNew<PbdObject> prismObj(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(size, dim, center);
    

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;

    // Use FEMTet constraints
    pbdParams->m_femParams->m_YoungModulus = 5.0;
    pbdParams->m_femParams->m_PoissonRatio = 0.4;
    pbdParams->enableFemConstraint(PbdFemConstraint::MaterialType::StVK);


    pbdParams->m_doPartitioning   = true;
    pbdParams->m_uniformMassValue = 0.05;
    pbdParams->m_gravity    = Vec3d(0.0, -1.0, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = numIter;
    pbdParams->m_viscousDampingCoeff = 0.03;

    // Fix the borders
    for (int z = 0; z < dim[2]; z++)
    {
        for (int y = 0; y < dim[1]; y++)
        {
            for (int x = 0; x < dim[0]; x++)
            {
                // if (x == 0 || /*z == 0 ||*/ x == dim[0] - 1 /*|| z == dim[2] - 1*/)
                if (y == dim[1]-1)
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

    // Setup the material
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    // Add a visual model to render the surface of the tet mesh
    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(prismMesh);
    visualModel->setRenderMaterial(material);
    prismObj->addVisualModel(visualModel);


    // Setup the Object
    prismObj->setPhysicsGeometry(prismMesh);
    prismObj->setDynamicalModel(pbdModel);

    return prismObj;
}


static std::shared_ptr<PbdObject>
makePbdObjSurface(const std::string& name,
           const Vec3d& size, 
           const Vec3i& dim, 
           const Vec3d& center,
           const int numIter)
{
    imstkNew<PbdObject> prismObj(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = makeTetGrid(size, dim, center);
    std::shared_ptr<SurfaceMesh>     surfMesh   = prismMesh->extractSurfaceMesh();

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;

    // Use volume+distance constraints, worse results. More performant (can use larger mesh)
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1.0);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1.0);

    pbdParams->m_doPartitioning   = true;
    pbdParams->m_uniformMassValue = 0.05;
    pbdParams->m_gravity    = Vec3d(0.0, -8.0, 0.0);
    pbdParams->m_dt         = 0.05;
    pbdParams->m_iterations = numIter;
    pbdParams->m_viscousDampingCoeff = 0.03;

    // Fix the borders
    for (int vert_id = 0; vert_id < surfMesh->getNumVertices(); vert_id++)
    {   
        auto position = surfMesh->getVertexPosition(vert_id);

        if (position(1) == 2.0){
            pbdParams->m_fixedNodeIds.push_back(vert_id);
        }

    }
    

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(surfMesh);
    pbdModel->configure(pbdParams);

    // Setup the material
    imstkNew<RenderMaterial> material;
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    // Add a visual model to render the surface of the tet mesh
    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(surfMesh);
    visualModel->setRenderMaterial(material);
    prismObj->addVisualModel(visualModel);


    // Setup the Object
    prismObj->setPhysicsGeometry(surfMesh);
    prismObj->setDynamicalModel(pbdModel);

    return prismObj;
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
    imstkNew<Scene> scene("PBDBenchmarkTest");
    scene->getActiveCamera()->setPosition(0.12, 4.51, 16.51);
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setViewUp(0.0, 0.96, -0.28);

    // Setup a tissue
    std::shared_ptr<PbdObject> PbdObj = makePbdFemObjVolume("Tissue",
        Vec3d(4.0, 4.0, 4.0), Vec3i(6, 6, 6), Vec3d(0.0, 0.0, 0.0), 5);
    scene->addSceneObject(PbdObj);

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