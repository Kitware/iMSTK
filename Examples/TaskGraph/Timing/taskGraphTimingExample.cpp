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

#include "imstkAPIUtilities.h"
#include "imstkCamera.h"
#include "imstkLineMesh.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"
#include "imstkSimulationManager.h"
#include "imstkTaskGraphVizWriter.h"

using namespace imstk;

///
/// \brief Create pbd string geometry
///
static std::shared_ptr<LineMesh>
makeStringGeometry(const Vec3d& pos, const size_t numVerts, const double stringLength)
{
    // Create the geometry
    std::shared_ptr<LineMesh> stringGeometry = std::make_shared<LineMesh>();

    StdVectorOfVec3d vertList;
    vertList.resize(numVerts);
    const double vertexSpacing = stringLength / numVerts;
    for (size_t j = 0; j < numVerts; j++)
    {
        vertList[j] = pos - Vec3d(0.0, static_cast<double>(j) * vertexSpacing, 0.0);
    }
    stringGeometry->setInitialVertexPositions(vertList);
    stringGeometry->setVertexPositions(vertList);

    // Add connectivity data
    std::vector<LineMesh::LineArray> segments;
    for (size_t j = 0; j < numVerts - 1; j++)
    {
        LineMesh::LineArray seg = { j, j + 1 };
        segments.push_back(seg);
    }

    stringGeometry->setLinesVertices(segments);
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
    std::shared_ptr<PbdObject> stringObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<LineMesh> stringMesh = makeStringGeometry(pos, numVerts, stringLength);

    // Setup the Parameters
    auto pbdParams = std::make_shared<PBDModelConfig>();
    pbdParams->enableConstraint(PbdConstraint::Type::Distance, 1e7);
    pbdParams->enableConstraint(PbdConstraint::Type::Bend, bendStiffness);
    pbdParams->m_fixedNodeIds     = { 0 };
    pbdParams->m_uniformMassValue = 5.0;
    pbdParams->m_gravity    = Vec3d(0, -9.8, 0);
    pbdParams->m_defaultDt  = 0.0005;
    pbdParams->m_iterations = 5;

    // Setup the Model
    std::shared_ptr<PbdModel> pbdModel = std::make_shared<PbdModel>();
    pbdModel->setModelGeometry(stringMesh);
    pbdModel->configure(pbdParams);

    // Setup the VisualModel
    std::shared_ptr<RenderMaterial> material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setColor(color);
    material->setLineWidth(2.0f);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    std::shared_ptr<VisualModel> visualModel = std::make_shared<VisualModel>(stringMesh);
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
    auto simManager = std::make_shared<SimulationManager>();
    auto scene      = simManager->createNewScene("PBDString");
    scene->getConfig()->taskTimingEnabled = true;

    // Setup N separate strings with varying bend stiffnesses
    std::vector<std::shared_ptr<PbdObject>> pbdStringObjs =
        makePbdStrings(numStrings, numVerts, stringSpacing, stringLength, startColor, endColor);
    for (std::shared_ptr<PbdObject> obj : pbdStringObjs)
    {
        scene->addSceneObject(obj);
    }

    // Adjust the camera
    scene->getCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getCamera()->setPosition(0.0, 0.0, 15.0);

    // Move the points every frame
    double t = 0.0;

    auto movePoints =
        [&pbdStringObjs, &t](Module* module)
        {
            for (unsigned int i = 0; i < pbdStringObjs.size(); i++)
            {
                std::shared_ptr<PbdModel> model = pbdStringObjs[i]->getPbdModel();
                const Vec3d               pos   = model->getCurrentState()->getVertexPosition(0);
                // Move in circle, derivatives of parametric eq of circle
                const Vec3d newPos = Vec3d(
                pos.x() + -std::sin(t) * radius * dt,
                pos.y(),
                pos.z() + std::cos(t) * radius * dt);
                model->getCurrentState()->setVertexPosition(0, newPos);
            }
            t += dt;
        };
    simManager->getSceneManager(scene)->setPostUpdateCallback(movePoints);

    // Start
    simManager->setActiveScene(scene);
    simManager->start();

    // Write the graph, highlighting the critical path and putting the completion time in the name
    TaskGraphVizWriter writer;
    writer.setInput(scene->getTaskGraph());
    writer.setFileName("taskGraphBenchmarkExample.svg");
    writer.setHighlightCriticalPath(true);
    writer.setWriteNodeComputeTimesColor(true);
    writer.setWriteNodeComputeTimesText(true);
    writer.write();

    return 0;
}
