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
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Create pbd string geometry
///
static std::shared_ptr<LineMesh>
makeStringGeometry(const Vec3d& pos, const int numVerts, const double stringLength)
{
    // Create the geometry
    imstkNew<LineMesh> stringGeometry;

    imstkNew<VecDataArray<double, 3>> verticesPtr(numVerts);
    VecDataArray<double, 3>&          vertices      = *verticesPtr.get();
    const double                      vertexSpacing = stringLength / numVerts;
    for (int i = 0; i < numVerts; i++)
    {
        vertices[i] = pos - Vec3d(0.0, static_cast<double>(i) * vertexSpacing, 0.0);
    }

    // Add connectivity data
    imstkNew<VecDataArray<int, 2>> segmentsPtr;
    VecDataArray<int, 2>&          segments = *segmentsPtr.get();
    for (int i = 0; i < numVerts - 1; i++)
    {
        segments.push_back(Vec2i(i, i + 1));
    }

    stringGeometry->initialize(verticesPtr, segmentsPtr);
    return stringGeometry;
}

///
/// \brief Create pbd string object
///
static std::shared_ptr<PbdObject>
makePbdString(
    const std::string& name,
    const Vec3d&       pos,
    const int          numVerts,
    const double       stringLength,
    const double       bendStiffness,
    const Color&       color)
{
    imstkNew<PbdObject> stringObj(name);

    // Setup the Geometry
    std::shared_ptr<LineMesh> stringMesh = makeStringGeometry(pos, numVerts, stringLength);

    // Setup the Parameters
    imstkNew<PBDModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 1.0e7);
    pbdParams->enableConstraint(PbdConstraint::Type::Bend, bendStiffness);
    pbdParams->m_fixedNodeIds     = { 0 };
    pbdParams->m_uniformMassValue = 5.0;
    pbdParams->m_gravity    = Vec3d(0.0, -9.8, 0.0);
    pbdParams->m_dt         = 0.0005;
    pbdParams->m_iterations = 5;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->setModelGeometry(stringMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setColor(color);
    material->setLineWidth(2.0f);
    material->setPointSize(6.0f);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    imstkNew<VisualModel> visualModel(stringMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    stringObj->addVisualModel(visualModel);
    stringObj->setPhysicsGeometry(stringMesh);
    stringObj->setDynamicalModel(pbdModel);

    return stringObj;
}

static std::vector<std::shared_ptr<PbdObject>>
makePbdStrings(const size_t numStrings,
               const int    numVerts,
               const double stringSpacing,
               const double stringLength,
               const Color& startColor,
               const Color& endColor)
{
    std::vector<std::shared_ptr<PbdObject>> pbdStringObjs(numStrings);

    const double size = stringSpacing * (numStrings - 1);

    for (unsigned int i = 0; i < numStrings; i++)
    {
        const Vec3d  tipPos = Vec3d(static_cast<double>(i) * stringSpacing - size * 0.5, stringLength * 0.5, 0.0);
        const double t      = static_cast<double>(i) / (numStrings - 1);

        pbdStringObjs[i] = makePbdString(
            "String " + std::to_string(i),
            tipPos,
            numVerts,
            stringLength,
            (static_cast<double>(i) * 0.1 / numStrings + 0.001) * 1e6,
            Color::lerpRgb(startColor, endColor, t));
    }

    return pbdStringObjs;
}

const double dt            = 0.0005;
const double radius        = 1.5;
const size_t numStrings    = 8;                    // Number of strings
const int    numVerts      = 30;                   // Number of vertices on each string
const double stringSpacing = 2.0;                  // How far each string is apart
const double stringLength  = 10.0;                 // Total length of string
const Color  startColor    = Color(1.0, 0.0, 0.0); // Color of first string
const Color  endColor      = Color(0.0, 1.0, 0.0); // Color of last string

///
/// \brief This example demonstrates string simulation
/// using Position based dynamics with varying bend stiffnesses
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("PBDString");

    // Setup N separate strings with varying bend stiffnesses
    std::vector<std::shared_ptr<PbdObject>> pbdStringObjs =
        makePbdStrings(numStrings, numVerts, stringSpacing, stringLength, startColor, endColor);
    for (auto obj : pbdStringObjs)
    {
        scene->addSceneObject(obj);
    }

    // Adjust the camera
    scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getActiveCamera()->setPosition(0.0, 0.0, 15.0);

    // Move the points every frame
    double t = 0.0;
    auto   movePoints =
        [&pbdStringObjs, &t](Event*)
        {
            for (size_t i = 0; i < pbdStringObjs.size(); i++)
            {
                std::shared_ptr<PbdModel>                model     = pbdStringObjs[i]->getPbdModel();
                std::shared_ptr<VecDataArray<double, 3>> positions = model->getCurrentState()->getPositions();
                (*positions)[0] += Vec3d(
                -std::sin(t) * radius * dt,
                0.0,
                std::cos(t) * radius * dt);
            }
            t += dt;
        };

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        sceneManager->pause(); // Start simulation paused

        connect<Event>(sceneManager, &SceneManager::postUpdate, movePoints);

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