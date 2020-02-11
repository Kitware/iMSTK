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
#include "imstkLineMesh.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdSolver.h"
#include "imstkSimulationManager.h"

using namespace imstk;

///
/// \brief This example demonstrates string simulation
/// using Position based dynamics with varying bend stiffnesses
///
int
main()
{
    auto simManager   = std::make_shared<SimulationManager>();
    auto scene = simManager->createNewScene("PBDString");

    // Setup N separate string simulations with varying bend stiffnesses
    const unsigned int numStrings    = 8;
    const unsigned int numVerts      = 30;
    const double       stringSpacing = 2.0;          // How far each string is apart
    const double       stringLength  = 10.0;         // Total length of string
    const Color        startColor    = Color::Red;   // Color of first string
    const Color        endColor      = Color::Green; // Color of last string

    struct PbdSim
    {
        std::shared_ptr<LineMesh> geometry;
        std::shared_ptr<PbdObject> object;
        std::shared_ptr<PbdModel> model;
        std::shared_ptr<PBDModelConfig> params;
        std::shared_ptr<VisualModel> visualModel;
        std::shared_ptr<PbdSolver> solver;
    };
    std::vector<PbdSim> sims(numStrings);

    const double size          = stringSpacing * (numStrings - 1);
    const double vertexSpacing = stringLength / numVerts;
    for (unsigned int i = 0; i < numStrings; i++)
    {
        // Setup the line mesh
        sims[i].geometry = std::make_shared<LineMesh>();
        StdVectorOfVec3d vertList;
        vertList.resize(numVerts);
        for (size_t j = 0; j < numVerts; j++)
        {
            vertList[j] = Vec3d(
                static_cast<double>(i) * stringSpacing - size * 0.5,
                stringLength * 0.5 - static_cast<double>(j) * vertexSpacing, 0.0);
        }
        sims[i].geometry->setInitialVertexPositions(vertList);
        sims[i].geometry->setVertexPositions(vertList);

        // Add connectivity data
        std::vector<LineMesh::LineArray> segments;
        for (size_t j = 0; j < numVerts - 1; j++)
        {
            LineMesh::LineArray seg = { j, j + 1 };
            segments.push_back(seg);
        }

        sims[i].geometry->setLinesVertices(segments);

        sims[i].object = std::make_shared<PbdObject>("String " + std::to_string(i));
        sims[i].model  = std::make_shared<PbdModel>();
        sims[i].model->setModelGeometry(sims[i].geometry);

        // Configure the parameters with bend stiffnesses varying from 0.001 to ~0.1
        sims[i].params = std::make_shared<PBDModelConfig>();
        sims[i].params->enableConstraint(PbdConstraint::Type::Distance, 0.001);
        sims[i].params->enableConstraint(PbdConstraint::Type::Bend, static_cast<double>(i) * 0.1 / numStrings + 0.001);
        sims[i].params->m_fixedNodeIds     = { 0 }; // Fix the first node in each string
        sims[i].params->m_uniformMassValue = 5.0;
        sims[i].params->m_gravity          = Vec3d(0, -9.8, 0);
        sims[i].params->m_dt               = 0.0005;
        sims[i].params->m_maxIter          = 5;

        // Set the parameters
        sims[i].model->configure(sims[i].params);
        sims[i].object->setDynamicalModel(sims[i].model);
        sims[i].object->setPhysicsGeometry(sims[i].geometry);

        sims[i].visualModel = std::make_shared<VisualModel>(sims[i].geometry);
        std::shared_ptr<RenderMaterial> material = std::make_shared<RenderMaterial>();
        material->setDisplayMode(RenderMaterial::DisplayMode::WIREFRAME);
        material->setDebugColor(Color::lerpRgb(startColor, endColor, static_cast<double>(i) / (numStrings - 1)));
        material->setLineWidth(2.0f);
        sims[i].visualModel->setRenderMaterial(material);
        sims[i].object->addVisualModel(sims[i].visualModel);

        // Solver
        sims[i].solver = std::make_shared<PbdSolver>();
        sims[i].solver->setPbdObject(sims[i].object);
        scene->addNonlinearSolver(sims[i].solver);

        // Add in scene
        scene->addSceneObject(sims[i].object);
    }

    // Adjust the camera
    scene->getCamera()->setFocalPoint(0.0, 0.0, 0.0);
    scene->getCamera()->setPosition(0.0, 0.0, 15.0);

    // Move the points every frame
    double       t          = 0.0;
    const double dt         = 0.0005;
    const double radius     = 1.5;
    auto         movePoints =
        [&sims, &t, dt, radius](Module* module)
        {
            for (unsigned int i = 0; i < sims.size(); i++)
            {
                Vec3d pos = sims[i].model->getCurrentState()->getVertexPosition(0);
                // Move in circle, derivatives of parametric eq of circle
                sims[i].model->getCurrentState()->setVertexPosition(0, imstk::Vec3d(
                pos.x() + -std::sin(t) * radius * dt,
                pos.y(),
                pos.z() + std::cos(t) * radius * dt));
            }
            t += dt;
        };
    simManager->getSceneManager(scene)->setPostUpdateCallback(movePoints);

    // Start
    simManager->setActiveScene(scene);
    simManager->startSimulation(SimulationStatus::RUNNING);

    return 0;
}
