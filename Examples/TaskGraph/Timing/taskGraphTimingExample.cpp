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
#include "imstkTaskGraphVizWriter.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

///
/// \brief Create pbd string geometry
///
static std::shared_ptr<LineMesh>
makeStringGeometry(const Vec3d& pos, const size_t numVerts, const double stringLength)
{
    const double vertexSpacing = stringLength / numVerts;

    // Create the geometry
    imstkNew<LineMesh> stringGeometry;

    imstkNew<VecDataArray<double, 3>> verticesPtr;
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    vertices.resize(static_cast<int>(numVerts));
    for (size_t j = 0; j < numVerts; j++)
    {
        vertices[j] = pos - Vec3d(0.0, static_cast<double>(j) * vertexSpacing, 0.0);
    }
    stringGeometry->setInitialVertexPositions(verticesPtr);
    stringGeometry->setVertexPositions(std::make_shared<VecDataArray<double, 3>>(*verticesPtr.get()));

    // Add connectivity data
    imstkNew<VecDataArray<int, 2>> segmentsPtr;
    VecDataArray<int, 2>&          segments = *segmentsPtr.get();
    for (size_t j = 0; j < numVerts - 1; j++)
    {
        segments.push_back(Vec2i(j, j + 1));
    }

    stringGeometry->setLinesIndices(segmentsPtr);
    return stringGeometry;
}

///
/// \brief Create pbd string object
///
static std::shared_ptr<PbdObject>
makePbdString(
    const std::string& name,
    const Vec3d&       pos,
    const size_t       numVerts,
    const double       stringLength,
    const double       bendStiffness,
    const Color&       color)
{
    imstkNew<PbdObject> stringObj(name);

    // Setup the Geometry
    std::shared_ptr<LineMesh> stringMesh = makeStringGeometry(pos, numVerts, stringLength);

    // Setup the Parameters
    imstkNew<PBDModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 1e7);
    pbdParams->enableConstraint(PbdConstraint::Type::Bend, bendStiffness);
    pbdParams->m_fixedNodeIds     = { 0 };
    pbdParams->m_uniformMassValue = 5.0;
    pbdParams->m_gravity    = Vec3d(0, -9.8, 0);
    pbdParams->m_defaultDt  = 0.0005;
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
               const size_t numVerts,
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
const size_t numVerts      = 30;                   // Number of vertices on each string
const double stringSpacing = 2.0;                  // How far each string is apart
const double stringLength  = 10.0;                 // Total length of string
const Color  startColor    = Color(1.0, 0.0, 0.0); // Color of first string
const Color  endColor      = Color(0.0, 1.0, 0.0); // Color of last string

///
/// \brief This examples uses the timing features of the task graph. This allows one
/// to see the elapsed time of every step of iMSTK as well as export the computational
/// graph and show information such as the critical path
///
int
main()
{
    // Setup logger (write to file and stdout)
    Logger::startLogger();

    imstkNew<Scene> scene("PBDString");
    scene->getConfig()->taskTimingEnabled = true;

    // Setup N separate strings with varying bend stiffnesses
    std::vector<std::shared_ptr<PbdObject>> pbdStringObjs =
        makePbdStrings(numStrings, numVerts, stringSpacing, stringLength, startColor, endColor);
    // Add the string scene objects to the scene
    for (std::shared_ptr<PbdObject> obj : pbdStringObjs)
    {
        scene->addSceneObject(obj);
    }

    // Adjust the camera
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
        // Setup a viewer to render in its own thread
        imstkNew<VTKViewer> viewer("Viewer");
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene in its own thread
        imstkNew<SceneManager> sceneManager("Scene Manager");
        sceneManager->setActiveScene(scene);
        viewer->addChildThread(sceneManager); // SceneManager will start/stop with viewer
        connect<Event>(sceneManager, EventType::PostUpdate, movePoints);

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

    // Write the graph, highlighting the critical path and putting the completion time in the name
    imstkNew<TaskGraphVizWriter> writer;
    writer->setInput(scene->getTaskGraph());
    writer->setFileName("taskGraphBenchmarkExample.svg");
    writer->setHighlightCriticalPath(true);
    writer->setWriteNodeComputeTimesColor(true);
    writer->setWriteNodeComputeTimesText(true);
    writer->write();

    return 0;
}
