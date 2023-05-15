/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#include "BendingBeamScene.h"
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

#include <algorithm>

namespace imstk
{
void
BendingBeamScene::Configuration::toString(std::ostream& str) const
{
    ProgrammableScene::Configuration::toString(str);
    str << "\tpartitions: " << partitions[0] << "x" << partitions[1] << "x" << partitions[2] << "\n";
    str << "\tdisplacement: " << displacement << "\n";
    str << "\tyoungsModulus: " << youngsModulus << "\n";
    str << "\tpoissonRatio: " << poissonRatio << "\n";
    // switch (materialType)
    // {
    // case PbdStrainEnergyConstraint::MaterialType::Linear:
    //     str << "\tmaterialType: Linear\n";
    //     break;
    // case PbdStrainEnergyConstraint::MaterialType::Corotation:
    //     str << "\tmaterialType: Corotation\n";
    //     break;
    // case PbdStrainEnergyConstraint::MaterialType::StVK:
    //     str << "\tmaterialType: StVK\n";
    //     break;
    // case PbdStrainEnergyConstraint::MaterialType::NeoHookean:
    //     str << "\tmaterialType: NeoHookean\n";
    //     break;
    // }
}

///
/// \brief Calculates the expected deflection of the vertices along the central axis of a beam
/// using Bernoulli-Euler beam theory.
///
bool
BendingBeamScene::writeAnalyticBendingBeam()
{
    auto physMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_pbdBeam->getPhysicsGeometry());

    std::vector<int> centralAxisIds;
    double           lengthX = 0.0;

    for (int vertId = 0; vertId < physMesh->getNumVertices(); vertId++)
    {
        auto initPos = physMesh->getInitialVertexPosition(vertId);
        lengthX = std::max(lengthX, initPos[0]);

        if (fabs(initPos[1]) <= 1e-7)    // && fabs(initPos[2]) <= 1e-7
        {
            centralAxisIds.push_back(vertId);
        }
    }

    if (centralAxisIds.size() == 0)
    {
        LOG(FATAL) << "No vertices found on central axis in analyticBeamDeflection";
    }

    double norm = 0.0;

    for (int cVert = 0; cVert < centralAxisIds.size(); cVert++)
    {
        double x = physMesh->getVertexPosition(centralAxisIds[cVert])[0];
        double y = physMesh->getVertexPosition(centralAxisIds[cVert])[1];;

        double correctY = ((x * x) / (2.0 * lengthX * lengthX * lengthX)) * (x - 3.0 * lengthX);

        norm += (y - correctY) * (y - correctY);
        std::cout << "Correct Y = " << correctY << " and pbd gives : " << y << "\n";
    }

    norm /= static_cast<double>(centralAxisIds.size());
    std::cout << "Norm = " << sqrt(norm);

    return true;
}

///
/// \brief Creates pbd simulated cantilevered beam for verifiction
///
std::shared_ptr<PbdObject>
makeBeam(
    const std::string&        name,
    std::vector<int>&         animatedVerts,
    std::vector<int>&         heldVerts,
    std::shared_ptr<PbdModel> model,
    const Vec3d&              gridSize,
    const Vec3i&              gridPartitions)
{
    auto beamObj = std::make_shared<PbdObject>(name);

    // Shift center such that beam starts at origion
    Vec3d center = gridSize[0] / 2.0 * Vec3d(1.0, 0.0, 0.0);

    if (gridPartitions[1] % 2 == 0)
    {
        LOG(FATAL) << "Grid partitions in Y direction must be odd so nodes fall on central axix in \n" <<
            "bending beam verification scene, num nodes in y = " << gridPartitions[1];
    }

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> prismMesh = GeometryUtils::toTetGrid(center, gridSize, gridPartitions);
    std::shared_ptr<SurfaceMesh>     surfMesh  = prismMesh->extractSurfaceMesh(); // for collision

    // Setup the Object
    beamObj->setPhysicsGeometry(prismMesh);
    beamObj->setVisualGeometry(prismMesh);
    beamObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    beamObj->setDynamicalModel(model);

    // Note: Calculate mass per node from material
    beamObj->getPbdBody()->uniformMassValue = 0.06 / prismMesh->getNumVertices();
    // beamObj->setPhysicsToCollidingMap(std::make_shared<PointwiseMap>(prismMesh, surfMesh));

    // model->getConfig()->m_secParams->m_YoungModulus = 108000.0;
    // model->getConfig()->m_secParams->m_PoissonRatio = 0.49;
    // model->getConfig()->enableStrainEnergyConstraint(PbdStrainEnergyConstraint::MaterialType::NeoHookean);
    //model->getConfig()->setBodyDamping(beamObj->getPbdBody()->bodyHandle, 0.01);

    // Fix the borders
    std::shared_ptr<VecDataArray<double, 3>> vertices = prismMesh->getVertexPositions();
    for (int i = 0; i < prismMesh->getNumVertices(); i++)
    {
        const Vec3d& pos = (*vertices)[i];
        if (fabs(pos[0]) <= 1e-7)
        {
            beamObj->getPbdBody()->fixedNodeIds.push_back(i);
        }
    }

    for (int i = 0; i < prismMesh->getNumVertices(); i++)
    {
        const Vec3d& pos = (*vertices)[i];
        if (fabs(pos[0] - gridSize[0]) <= 1e-7) //&& fabs(pos[1])<= 1E-7
        {
            animatedVerts.push_back(i);
            //heldVerts.push_back(i);
        }

        if (fabs(pos[0] - gridSize[0] && fabs(pos[1]) <= 1E-7) <= 1e-7) //&& fabs(pos[1])<= 1E-7
        {
            //animatedVerts.push_back(i);
            heldVerts.push_back(i);
        }
    }

    LOG(INFO) << prismMesh->getNumVertices() << " vertices in Bending Beam Mesh";
    LOG(INFO) << prismMesh->getNumCells() << " cells in Bending Beam Mesh";

    return beamObj;
}

BendingBeamScene::BendingBeamScene() : ProgrammableScene()
{
    BendingBeamScene::Configuration cfg;
    cfg.iterations    = 3;
    cfg.displacement  = 1.0;
    cfg.youngsModulus = 1000.0;       // https://doi.org/10.1016/S0301-5629(02)00489-1
    cfg.poissonRatio  = 0.49;         // https://doi.org/10.1118/1.279566
    cfg.partitions    = Vec3i(15, 5, 4);
    // cfg.materialType  = PbdStrainEnergyConstraint::MaterialType::StVK;
    setConfiguration(cfg);
}

BendingBeamScene::BendingBeamScene(const BendingBeamScene::Configuration& cfg)
{
    setConfiguration(cfg);
}

void
BendingBeamScene::setConfiguration(const Configuration& cfg)
{
    ProgrammableScene::setConfiguration(cfg);
    m_config = cfg;

    m_analyticTracker.setFilename(m_outDir + "analytic.csv");
}

bool
BendingBeamScene::setupScene(double sampleTime)
{
    std::shared_ptr<PbdModelConfig> pbdParams = m_pbdModel->getConfig();
    pbdParams->m_gravity    = Vec3d(0.0, 0.0, 0.0);
    pbdParams->m_iterations = m_config.iterations;
    pbdParams->m_linearDampingCoeff  = 0.001;
    pbdParams->m_angularDampingCoeff = 0.0;
    pbdParams->m_doPartitioning      = false;

    std::shared_ptr<PbdObject> pbdObject;
    std::vector<int>           animatedVerts;

    // Setup progammed movement
    auto deviceClient = std::make_shared<ProgrammableClient>();
    deviceClient->setDeltaTime(m_pbdModel->getConfig()->m_dt);

    Vec3d size = Vec3d(10.0, 0.5, 1.0);

    std::vector<int> heldVerts;
    pbdObject = makeBeam("beam", animatedVerts, heldVerts, m_pbdModel, size, m_config.partitions);
    m_scene->addSceneObject(pbdObject);
    m_pbdBeam = pbdObject;

    // set tracked node ids
    auto physMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_pbdBeam->getPhysicsGeometry());

    double lengthX = 0.0;
    for (int vertId = 0; vertId < physMesh->getNumVertices(); vertId++)
    {
        auto initPos = physMesh->getInitialVertexPosition(vertId);

        if (fabs(initPos[1]) <= 1e-7)    // && fabs(initPos[2]) <= 1e-7
        {
            m_trackedNodeIds.push_back(vertId);
        }
    }

    Vec3d             finalTranslation = Vec3d(0.0, -1.0, 0.0);
    std::vector<bool> pin{ false, true, true };

    deviceClient->addLinearVertexMovement(pbdObject, animatedVerts,
    finalTranslation,
    pin,
    0.0, 3.0);

    deviceClient->addHoldCommand(pbdObject, 3.0, 10, heldVerts);

    m_clients.push_back(deviceClient);
    m_duration = deviceClient->getTotalDuration();

    ProgrammableScene::setupScene(sampleTime);
    return true;
}

bool
BendingBeamScene::setupView()
{
    m_scene->getActiveCamera()->setPosition(4.81857, 1.57499, 14.6467);
    m_scene->getActiveCamera()->setFocalPoint(5, 0, 0);
    m_scene->getActiveCamera()->setViewUp(0.0, 1.0, -0.1);

    return true;
}

bool
BendingBeamScene::trackData(double time)
{
    //if (time >= m_duration - (2.0 * m_config.dt))
    {
        auto physMesh = std::dynamic_pointer_cast<TetrahedralMesh>(m_pbdBeam->getPhysicsGeometry());

        double lengthX = 0.0;
        for (int i = 0; i < m_trackedNodeIds.size(); i++)
        {
            auto pos = physMesh->getVertexPosition(m_trackedNodeIds[i]);
            lengthX = std::max(lengthX, pos[0]);
        }
        for (int i = 0; i < m_trackedNodeIds.size(); i++)
        {
            auto pos = physMesh->getVertexPosition(m_trackedNodeIds[i]);
            m_dataTracker.probe("X" + std::to_string(i) + "Position(m)", pos[0]);
            m_dataTracker.probe("Y" + std::to_string(i) + "Position(m)", pos[1]);

            double x = pos[0];
            double y = pos[1];
            double correctY = ((x * x) / (2.0 * lengthX * lengthX * lengthX)) * (x - 3.0 * lengthX);
            m_analyticTracker.probe("X" + std::to_string(i) + "Position(m)", pos[0]);
            m_analyticTracker.probe("Y" + std::to_string(i) + "Position(m)", correctY);
        }
        m_dataTracker.streamProbesToFile(time);
        m_analyticTracker.streamProbesToFile(time);
    }
    return ProgrammableScene::trackData(time);
}

bool
BendingBeamScene::postProcessAnalyticResults()
{
    bool bRet = true;
    bRet &= compareAndPlotTimeSeriesCSVs(m_outDir + "analytic.csv", m_outDir + "analytic.csv", m_analyticVerification, m_outDir + "analytic_verificarion_results/");
    bRet &= compareAndPlotTimeSeriesCSVs(m_outDir + "computed.csv", m_outDir + "computed.csv", m_computedVerification, m_outDir + "computed_verificarion_results/");
    bRet &= compareCSVs(m_outDir + "analytic.csv", m_outDir + "computed.csv", m_validation);

    PointPlotConfig cfg;
    cfg.dt       = m_config.dt;
    cfg.baseName = "Positions@";
    cfg.xPrefix  = "X";
    cfg.xPostfix = "Position";
    cfg.yPrefix  = "Y";
    cfg.yPostfix = "Position";
    for (size_t t = 1; t < m_duration; t++)
    {
        cfg.times.push_back(t);
    }
    bRet &= plot2DPointCSVs(m_outDir + "analytic.csv", m_outDir + "computed.csv", cfg, m_outDir + "validation_results/");

    for (size_t t = 0; t < cfg.times.size(); t++)
    {
        LOG(INFO) << "At time " << cfg.times[t] << ", xRMS=" << cfg.xRootMeanSquared[t] << ", yRMS=" << cfg.yRootMeanSquared[t];
    }
}
} // end namespace imstk
