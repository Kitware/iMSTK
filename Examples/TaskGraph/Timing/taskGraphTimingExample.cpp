/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCamera.h"
#include "imstkGeometryUtilities.h"
#include "imstkKeyboardDeviceClient.h"
#include "imstkKeyboardSceneControl.h"
#include "imstkLineMesh.h"
#include "imstkLogger.h"
#include "imstkMouseDeviceClient.h"
#include "imstkMouseSceneControl.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkRenderMaterial.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkTaskGraphVizWriter.h"
#include "imstkVisualModel.h"
#include "imstkVTKViewer.h"

using namespace imstk;

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
    std::shared_ptr<LineMesh> stringMesh =
        GeometryUtils::toLineGrid(pos, Vec3d(0.0, -1.0, 0.0), stringLength, numVerts);

    // Setup the Parameters
    imstkNew<PbdModelConfig> pbdParams;
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 1e7);
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Bend, bendStiffness);
    pbdParams->m_gravity    = Vec3d(0, -9.8, 0);
    pbdParams->m_dt         = 0.0005;
    pbdParams->m_iterations = 5;

    // Setup the Model
    imstkNew<PbdModel> pbdModel;
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    imstkNew<RenderMaterial> material;
    material->setBackFaceCulling(false);
    material->setColor(color);
    material->setLineWidth(2.0f);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    imstkNew<VisualModel> visualModel;
    visualModel->setGeometry(stringMesh);
    visualModel->setRenderMaterial(material);

    // Setup the Object
    stringObj->addVisualModel(visualModel);
    stringObj->setPhysicsGeometry(stringMesh);
    stringObj->setDynamicalModel(pbdModel);
    stringObj->getPbdBody()->fixedNodeIds     = { 0 };
    stringObj->getPbdBody()->uniformMassValue = 5.0;

    return stringObj;
}

static std::vector<std::shared_ptr<PbdObject>>
makePbdStrings(const int    numStrings,
               const int    numVerts,
               const double stringSpacing,
               const double stringLength,
               const Color& startColor,
               const Color& endColor)
{
    std::vector<std::shared_ptr<PbdObject>> pbdStringObjs(numStrings);

    const double size = stringSpacing * (numStrings - 1);

    for (int i = 0; i < numStrings; i++)
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

const double radius        = 1.5;
const int    numStrings    = 8;                    // Number of strings
const int    numVerts      = 30;                   // Number of vertices on each string
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
    scene->getActiveCamera()->setPosition(0.0, 0.0, 15.0);

    // Setup N separate strings with varying bend stiffnesses
    std::vector<std::shared_ptr<PbdObject>> pbdStringObjs =
        makePbdStrings(numStrings, numVerts, stringSpacing, stringLength, startColor, endColor);
    // Add the string scene objects to the scene
    for (std::shared_ptr<PbdObject> obj : pbdStringObjs)
    {
        scene->addSceneObject(obj);
    }

    // Run the simulation
    {
        // Setup a viewer to render
        imstkNew<VTKViewer> viewer;
        viewer->setActiveScene(scene);

        // Setup a scene manager to advance the scene
        imstkNew<SceneManager> sceneManager;
        sceneManager->setActiveScene(scene);
        sceneManager->pause();
        double t = 0.0;
        connect<Event>(sceneManager, &SceneManager::postUpdate, [&](Event*)
            {
                const double dt = sceneManager->getDt();
                for (size_t i = 0; i < pbdStringObjs.size(); i++)
                {
                    std::shared_ptr<PbdModel> model = pbdStringObjs[i]->getPbdModel();
                    model->getConfig()->m_dt = dt;
                    std::shared_ptr<VecDataArray<double, 3>> positions = pbdStringObjs[i]->getPbdBody()->vertices;
                    (*positions)[0] += Vec3d(
                        -std::sin(t) * radius * dt,
                        0.0,
                        std::cos(t) * radius * dt);
                }
                t += dt;
            });

        imstkNew<SimulationManager> driver;
        driver->addModule(viewer);
        driver->addModule(sceneManager);
        driver->setDesiredDt(0.005);

        // Add mouse and keyboard controls to the viewer
        {
            auto mouseControl = std::make_shared<MouseSceneControl>();
            mouseControl->setDevice(viewer->getMouseDevice());
            mouseControl->setSceneManager(sceneManager);
            scene->addControl(mouseControl);

            auto keyControl = std::make_shared<KeyboardSceneControl>();
            keyControl->setDevice(viewer->getKeyboardDevice());
            keyControl->setSceneManager(sceneManager);
            keyControl->setModuleDriver(driver);
            scene->addControl(keyControl);
        }

        driver->start();
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
