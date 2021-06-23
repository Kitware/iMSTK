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
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkDirectionalLight.h"
#include "imstkSpotLight.h"
#include "imstkLogger.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkSurfaceMesh.h"
#include "imstkSurfaceMeshSubdivide.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Creates cloth geometry
/// \param cloth width
/// \param cloth height
/// \param cloth row count
/// \param cloth column count
///
static std::shared_ptr<SurfaceMesh>
makeClothGeometry(const double width,
                  const double height,
                  const int    nRows,
                  const int    nCols)
{
    imstkNew<SurfaceMesh> clothMesh;

    imstkNew<VecDataArray<double, 3>> verticesPtr(nRows * nCols);
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    const double                      dy       = width / (nCols - 1);
    const double                      dx       = height / (nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            vertices[i * nCols + j] = Vec3d(dx * static_cast<double>(i), 1.0, dy * static_cast<double>(j));
        }
    }

    // Add connectivity data
    imstkNew<VecDataArray<int, 3>> indicesPtr;
    VecDataArray<int, 3>&          indices = *indicesPtr.get();
    for (int i = 0; i < nRows - 1; ++i)
    {
        for (int j = 0; j < nCols - 1; j++)
        {
            const int index1 = i * nCols + j;
            const int index2 = index1 + nCols;
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

    clothMesh->initialize(verticesPtr, indicesPtr);

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
             const double       width,
             const double       height,
             const int          nRows,
             const int          nCols)
{
    imstkNew<PbdObject> clothObj(name);

    // Setup the Geometry
    std::shared_ptr<SurfaceMesh> clothMesh = makeClothGeometry(width, height, nRows, nCols);

    // Setup the Parameters
    imstkNew<PBDModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 1.0e2);
    pbdParams->enableConstraint(PbdConstraint::Type::Dihedral, 1.0e1);
    pbdParams->m_fixedNodeIds     = { 0, static_cast<size_t>(nCols) - 1 };
    pbdParams->m_uniformMassValue = width * height / (nRows * nCols);
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.005;
    pbdParams->m_iterations = 5;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(clothMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);

    imstkNew<VisualModel> visualModel(clothMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    clothObj->addVisualModel(visualModel);
    clothObj->setPhysicsGeometry(clothMesh);
    clothObj->setDynamicalModel(pbdModel);

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
    imstkNew<Scene>            scene("PBDCloth");
    std::shared_ptr<PbdObject> clothObj = nullptr;
    {
        clothObj = makeClothObj("Cloth", 10.0, 10.0, 16, 16);
        scene->addSceneObject(clothObj);

        // Light (white)
        imstkNew<DirectionalLight> whiteLight("whiteLight");
        whiteLight->setFocalPoint(Vec3d(5.0, -8.0, -5.0));
        whiteLight->setIntensity(1.0);
        scene->addLight(whiteLight);

        // Light (red)
        imstkNew<SpotLight> colorLight("colorLight");
        colorLight->setPosition(Vec3d(-5.0, -3.0, 5.0));
        colorLight->setFocalPoint(Vec3d(0.0, -5.0, 5.0));
        colorLight->setIntensity(100.);
        colorLight->setColor(Color::Red);
        colorLight->setSpotAngle(30.0);
        scene->addLight(colorLight);

        // Adjust camera
        scene->getActiveCamera()->setFocalPoint(0.0, -5.0, 5.0);
        scene->getActiveCamera()->setPosition(-15.0, -5.0, 25.0);
    }

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

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
                clothMesh->setTriangleIndices(newClothMesh->getTriangleIndices());
                clothMesh->setVertexAttribute("Velocities", newClothMesh->getVertexAttribute("Velocities"));
                clothMesh->postModified();

                // Re-setup the constraints on the object
                clothObj->initialize();
            }
        });

        driver->start();
    }

    return 0;
}
