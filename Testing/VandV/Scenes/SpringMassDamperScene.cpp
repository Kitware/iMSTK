/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#include "SpringMassDamperScene.h"

#include "imstkGeometryUtilities.h"
#include "imstkPbdModel.h"
#include "imstkPbdModelConfig.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdObjectCollision.h"
#include "imstkObjectControllerGhost.h"
#include "imstkTetrahedralMesh.h"

#include "imstkCamera.h"
#include "imstkRenderMaterial.h"
#include "imstkVisualModel.h"

namespace imstk
{
bool
SpringMassDamperScene::writeAnalyticBaseline()
{
    double time = 0.0;
    for (size_t i = 1; i < getNumSteps(); i++) // start at 1 since tracker starts after first constraint projection step
    {
        double truePosition = 0.0;

        // m_analyticTracker.probe("Position", truePosition);
        time += m_config.dt;

        analyticPosition.push_back(0.0);
        // m_analyticTracker.streamProbesToFile(time);
    }

    return true;
}

///
/// \brief Create pbd spring mass
///
static std::shared_ptr<PbdObject>
makePbdSpringMass(
    const std::string&                          name,
    std::shared_ptr<PbdModel>                   model,
    std::vector<int>&                           animatedVerts,
    const SpringMassDamperScene::Configuration& cfg)
{
    auto springObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<LineMesh> springMesh =
        GeometryUtils::toLineGrid(Vec3d(0.0, 1.0, 0.0), Vec3d(0.0, -1.0, 0.0), 1.0, 2);

    // Setup the Parameters
    auto pbdParams = model->getConfig();
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, cfg.stiffness);

    animatedVerts.push_back(1);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setColor(Color::Red);
    material->setLineWidth(2.0);
    material->setPointSize(6.0);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    auto pointsVis = std::make_shared<VisualModel>();

    pointsVis->setGeometry(springMesh);
    auto material2 = std::make_shared<RenderMaterial>();
    material2->setDisplayMode(RenderMaterial::DisplayMode::Points);
    material2->setPointSize(10.0);
    pointsVis->setRenderMaterial(material2);

    // Setup the Object
    springObj->setVisualGeometry(springMesh);
    springObj->getVisualModel(0)->setRenderMaterial(material);

    springObj->addVisualModel(pointsVis);
    springObj->setPhysicsGeometry(springMesh);
    springObj->setDynamicalModel(model);
    springObj->getPbdBody()->fixedNodeIds     = { 0 };
    springObj->getPbdBody()->uniformMassValue = cfg.mass;

    return springObj;
}

SpringMassDamperScene::SpringMassDamperScene()
{
    SpringMassDamperScene::Configuration cfg;
    cfg.iterations = 1;
    cfg.elongationPercent = 20;
    cfg.mass      = 1.0;
    cfg.stiffness = 10.0;

    setConfiguration(cfg);
}

SpringMassDamperScene::SpringMassDamperScene(const SpringMassDamperScene::Configuration& cfg)
{
    setConfiguration(cfg);
}

void
SpringMassDamperScene::setConfiguration(const Configuration& cfg)
{
    ProgrammableScene::setConfiguration(cfg);
    m_config = cfg;
}

bool
SpringMassDamperScene::setupScene(double sampleTime)
{
    std::shared_ptr<PbdModelConfig> pbdParams = m_pbdModel->getConfig();
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_dt         = m_config.dt;
    pbdParams->m_iterations = m_config.iterations;
    pbdParams->m_linearDampingCoeff  = 0.0;
    pbdParams->m_angularDampingCoeff = 0.0;
    pbdParams->m_doPartitioning      = false;

    m_analyticTracker.setFilename(m_outDir + "analytic.csv");

    std::vector<int> animatedVerts;

    // Setup progammed movement
    auto deviceClient = std::make_shared<ProgrammableClient>();
    deviceClient->setDeltaTime(m_pbdModel->getConfig()->m_dt);

    /*  auto deviceClient2 = std::make_shared<ProgrammableClient>();
      deviceClient2->setDeltaTime(m_pbdModel->getConfig()->m_dt);*/
    std::shared_ptr<PbdObject> pbdSpring = makePbdSpringMass("springMass", m_pbdModel, animatedVerts, m_config);
    m_pbdSpring = pbdSpring;
    m_scene->addSceneObject(m_pbdSpring);

    Vec3d             finalTranslation = Vec3d(0.0, -1.0 * m_config.elongationPercent / 100, 0.0);
    std::vector<bool> pin{ true, true, true };

    deviceClient->addLinearVertexMovement(m_pbdSpring, animatedVerts,
      finalTranslation,
      pin,
      0.0, m_config.dt);

    /*deviceClient->addHoldCommand(m_pbdSpring, 0.5, 0.5, animatedVerts);*/

    double duration = 10.0;
    deviceClient->addWaitCommand(0.0, 10.0);

    m_clients.push_back(deviceClient);

    m_duration = deviceClient->getTotalDuration();
    m_numSteps = (size_t)(m_duration / m_config.dt);

    // writeAnalyticBaseline();
    ProgrammableScene::setupScene(sampleTime);
    return true;
}

bool
SpringMassDamperScene::setupView()
{
    m_scene->getActiveCamera()->setPosition(0.00522859, -0.203912, 3.67425);
    m_scene->getActiveCamera()->setFocalPoint(0, 0.5, 0);
    m_scene->getActiveCamera()->setViewUp(0.0, 1.0, -0.1);

    return true;
}

bool
SpringMassDamperScene::trackData(double time)
{
    auto  physMesh    = std::dynamic_pointer_cast<LineMesh>(m_pbdSpring->getPhysicsGeometry());
    auto& constraints = m_pbdSpring->getPbdModel()->getConstraints()->getConstraints();

    double force = 0.0;
    for (auto constraint : constraints)
    {
        force += constraint->getForce(m_config.dt);
    }
    // LOG(WARNING) << "Force = " << force;
    m_dataTracker.probe("Force(N)", force);
    m_dataTracker.probe("YPosition(m)", physMesh->getVertexPosition(1)[1]);
    m_dataTracker.streamProbesToFile(time);

    double trueForce = m_config.stiffness * physMesh->getVertexPosition(1)[1];

    double truePosition = 0.0;

    double C = sqrt(m_config.stiffness / m_config.mass);
    truePosition = -1.0 * (m_config.elongationPercent / 100.0) * cos(C * time);

    m_analyticTracker.probe("Force(N)", trueForce);
    m_analyticTracker.probe("YPosition(m)", truePosition);

    m_analyticTracker.streamProbesToFile(time);

    return ProgrammableScene::trackData(time);
}

bool
SpringMassDamperScene::postProcessAnalyticResults()
{
    return true;// compareAndPlotTimeSeriesCSVs(m_outDir + "computed.csv", m_outDir + "computed.csv", cmp, m_outDir + "validation_results/");
}
} // end namespace imstk
