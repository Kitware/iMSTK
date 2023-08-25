/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#include "ChainDropScene.h"
#include "Verification/CSVUtils.h"

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
///
/// \brief Create pbd chain drop scene. During this scene the force estimate from the constraint
/// at the top of the chain is tracked as a function of time for verification
///
static std::shared_ptr<PbdObject>
makePbdChain(
    const std::string&                   name,
    std::shared_ptr<PbdModel>            model,
    const double                         chainLength,
    const double                         stiffness,
    const ChainDropScene::Configuration& cfg)
{
    auto chainObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<LineMesh> springMesh =
        GeometryUtils::toLineGrid(Vec3d(0.0, 1.0, 0.0), Vec3d(1.0, 0.0, 0.0), chainLength, 20);

    // Setup the Parameters
    auto pbdParams = model->getConfig();
    pbdParams->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, stiffness);

    // Setup the VisualModel
    auto material = std::make_shared<RenderMaterial>();
    material->setBackFaceCulling(false);
    material->setColor(Color::Red);
    material->setLineWidth(2.0);
    material->setPointSize(3.0);
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    auto pointsVis = std::make_shared<VisualModel>();

    pointsVis->setGeometry(springMesh);
    auto material2 = std::make_shared<RenderMaterial>();
    material2->setDisplayMode(RenderMaterial::DisplayMode::Points);
    material2->setPointSize(10.0);
    pointsVis->setRenderMaterial(material2);

    // Setup the Object
    chainObj->setVisualGeometry(springMesh);
    chainObj->getVisualModel(0)->setRenderMaterial(material);

    chainObj->addVisualModel(pointsVis);
    chainObj->setPhysicsGeometry(springMesh);
    chainObj->setDynamicalModel(model);
    chainObj->getPbdBody()->fixedNodeIds     = { 0 };
    chainObj->getPbdBody()->uniformMassValue = cfg.mass;

    return chainObj;
}

ChainDropScene::ChainDropScene()
{
    ChainDropScene::Configuration cfg;
    cfg.iterations  = 1000;
    cfg.chainLength = 20.0;
    cfg.mass      = 1.0;
    cfg.stiffness = 1E8;
    cfg.dt = 0.025;

    setConfiguration(cfg);
}

ChainDropScene::ChainDropScene(const ChainDropScene::Configuration& cfg)
{
    setConfiguration(cfg);
}

void
ChainDropScene::setConfiguration(const Configuration& cfg)
{
    ProgrammableScene::setConfiguration(cfg);
    m_config = cfg;
}

bool
ChainDropScene::setupScene(double sampleTime)
{
    std::shared_ptr<PbdModelConfig> pbdParams = m_pbdModel->getConfig();
    pbdParams->m_gravity    = Vec3d(0.0, -10.0, 0.0);
    pbdParams->m_dt         = m_config.dt;
    pbdParams->m_iterations = m_config.iterations;
    pbdParams->m_linearDampingCoeff  = 0.00;
    pbdParams->m_angularDampingCoeff = 0.0;
    pbdParams->m_doPartitioning      = false;

    std::vector<int> animatedVerts;

    // Setup progammed movement
    auto deviceClient = std::make_shared<ProgrammableClient>();
    deviceClient->setDeltaTime(m_pbdModel->getConfig()->m_dt);

    // Create chain with config parameters
    m_pbdChain = makePbdChain("chain", m_pbdModel, m_config.chainLength, m_config.stiffness, m_config);
    m_scene->addSceneObject(m_pbdChain);

    deviceClient->addWaitCommand(0.0, 4.0);

    m_clients.push_back(deviceClient);

    m_duration = deviceClient->getTotalDuration();
    m_numSteps = (size_t)(m_duration / m_config.dt);

    ProgrammableScene::setupScene(sampleTime);
    return true;
}

bool
ChainDropScene::setupView()
{
    m_scene->getActiveCamera()->setPosition(0.0, -30, 45);
    m_scene->getActiveCamera()->setFocalPoint(0, 0.5, 0);
    m_scene->getActiveCamera()->setViewUp(0.0, 1.0, -0.1);

    return true;
}

bool
ChainDropScene::trackData(double time)
{
    auto&  constraints = m_pbdChain->getPbdModel()->getConstraints()->getConstraints();
    double force       =  -1.0 * constraints[0]->getForce(m_config.dt);

    double frame = time / m_config.dt;

    if (frame < 74.5 || frame > 100)
    {
        return true;
    }

    m_dataTracker.probe("Force", force);
    m_dataTracker.probe("Frame", frame);

    m_dataTracker.streamProbesToFile(time);

    return ProgrammableScene::trackData(time);
}

bool
ChainDropScene::postProcessAnalyticResults()
{
    return true;// compareAndPlotTimeSeriesCSVs(m_outDir + "computed.csv", m_outDir + "computed.csv", cmp, m_outDir + "validation_results/");
}
} // end namespace imstk
