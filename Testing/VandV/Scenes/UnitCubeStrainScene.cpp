/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. */

#include "UnitCubeStrainScene.h"

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
/// \brief Creates pbd simulated unit cube for strain energy verification
///
static std::shared_ptr<PbdObject>
makePbdUnitCube(
    const std::string&                        name,
    std::vector<int>&                         animatedVerts,
    std::shared_ptr<PbdModel>                 model,
    const UnitCubeStrainScene::Configuration& cfg)
{
    auto cubeObj = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> cubeMesh = GeometryUtils::toTetGrid(Vec3d(0.0, 0.0, 0.0), Vec3d(1.0, 1.0, 1.0), cfg.partitions);

    // std::shared_ptr<SurfaceMesh>     surfMesh = cubeMesh->extractSurfaceMesh();

    // Setup the Object
    cubeObj->setPhysicsGeometry(cubeMesh);
    // cubeObj->addComponent<Collider>()->setGeometry(surfMesh);
    cubeObj->setVisualGeometry(cubeMesh);
    cubeObj->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    cubeObj->setDynamicalModel(model);
    cubeObj->getPbdBody()->uniformMassValue = 1000.0 / cubeMesh->getNumVertices();

    model->getConfig()->m_femParams->m_YoungModulus = cfg.youngsModulus;
    model->getConfig()->m_femParams->m_PoissonRatio = cfg.poissonRatio;
    model->getConfig()->enableFemConstraint(cfg.materialType);
    // model->getConfig()->setBodyDamping(cubeObj->getPbdBody()->bodyHandle, 0.01);

    std::shared_ptr<VecDataArray<double, 3>> vertices = cubeMesh->getVertexPositions();

    // Only animate the boundary vertices
    for (int i = 0; i < cubeMesh->getNumVertices(); i++)
    {
        const Vec3d& pos = (*vertices)[i];
        if (fabs(pos[0] + 0.5) <= 1e-7 || fabs(pos[0] - 0.5) <= 1e-7
            || fabs(pos[1] + 0.5) <= 1e-7 || fabs(pos[1] - 0.5) <= 1e-7
            || fabs(pos[2] + 0.5) <= 1e-7 || fabs(pos[2] - 0.5) <= 1e-7)
        {
            animatedVerts.push_back(i);
            cubeObj->getPbdBody()->fixedNodeIds.push_back(i);
        }
        /*animatedVerts.push_back(i);
        cubeObj->getPbdBody()->fixedNodeIds.push_back(i);*/
    }
    LOG(INFO) << cubeMesh->getNumVertices() << " vertices in Unit Cube Mesh";
    LOG(INFO) << cubeMesh->getNumCells() << " cells in Unit Cube Mesh \n";

    return cubeObj;
}

UnitCubeStrainScene::UnitCubeStrainScene()
{
    UnitCubeStrainScene::Configuration cfg;
    cfg.iterations = 5;
    cfg.strain     = 0.20;
    cfg.compressionDuration = 5;
    cfg.youngsModulus       = 1000.0; // https://doi.org/10.1016/S0301-5629(02)00489-1
    cfg.poissonRatio    = 0.49;       // https://doi.org/10.1118/1.279566
    cfg.partitions      = Vec3i(8, 8, 8);
    cfg.deformationType = ProgrammableClient::DeformationType::Compression;
    // cfg.materialType    = PbdStrainEnergyConstraint::MaterialType::StVK;
    setConfiguration(cfg);
}

UnitCubeStrainScene::UnitCubeStrainScene(const UnitCubeStrainScene::Configuration& cfg)
{
    setConfiguration(cfg);
}

void
UnitCubeStrainScene::setConfiguration(const Configuration& cfg)
{
    ProgrammableScene::setConfiguration(cfg);
    m_config = cfg;
}

bool
UnitCubeStrainScene::setupScene(double sampleTime)
{
    // Setup progammed movement
    auto deviceClient = std::make_shared<ProgrammableClient>();
    deviceClient->setDeltaTime(m_config.dt);

    std::vector<int>           animatedVerts;
    std::shared_ptr<PbdObject> pbdObject = makePbdUnitCube("unitCube", animatedVerts, m_pbdModel, m_config);
    m_scene->addSceneObject(pbdObject);
    m_pbdCube = pbdObject;

    // Amount of strain at the end of the simulation
    std::vector<bool> pin{ true, true, true };
    deviceClient->addDeformation(pbdObject, animatedVerts, m_config.strain, m_config.deformationType, m_config.poissonRatio, pin, 0.0, 5.0);

    m_clients.push_back(deviceClient);

    m_duration = deviceClient->getTotalDuration();
    m_numSteps = (size_t)(m_duration / m_config.dt);

    writeAnalyticStrainEnergyBaseline();
    ProgrammableScene::setupScene(sampleTime);
    return true;
}

bool
UnitCubeStrainScene::setupView()
{
    m_scene->getActiveCamera()->setPosition(0.0378858, 0.677443, 4.02733);
    m_scene->getActiveCamera()->setFocalPoint(0, 0, 0);
    m_scene->getActiveCamera()->setViewUp(0.0, 1.0, -0.1);

    return true;
}

bool
UnitCubeStrainScene::trackData(double time)
{
    double strainEnergy = 0.0;
    auto&  constraints  = m_pbdCube->getPbdModel()->getConstraints()->getConstraints();

    for (auto constraint : constraints)
    {
        strainEnergy += constraint->getConstraintC();
    }
    m_dataTracker.probe("StrainEnergy", strainEnergy);
    m_dataTracker.streamProbesToFile(time);

    return ProgrammableScene::trackData(time);
}

bool
UnitCubeStrainScene::writeAnalyticStrainEnergyBaseline()
{
    double time       = 0.0;
    Mat3d  defGrad    = Mat3d::Identity();
    double strainRate = m_config.strain / m_duration;

    double pratio = 2.0 * m_config.poissonRatio;

    DataTracker tracker;
    tracker.setFilename(m_outDir + "analytic.csv");
    int seIdx = tracker.configureProbe("StrainEnergy");

    // std::cout << "Time , Strain Energy \n";
    for (size_t i = 1; i < getNumSteps(); i++) // start at 1 since tracker starts after first constraint projection step
    {
        // Calculate the current deformation gradient
        switch (m_config.deformationType)
        {
        case ProgrammableClient::DeformationType::Compression:
        {
            double isoCompression = sqrt(1.0 / (1.0 - strainRate * m_config.dt)) - 1.0;
            Mat3d  compression{
                { isoCompression* pratio, 0.0, 0.0 },
                { 0.0, -strainRate * m_config.dt, 0.0 },
                { 0.0, 0.0, isoCompression* pratio } };

            defGrad += compression;
            break;
        }
        case ProgrammableClient::DeformationType::Tension:
        {
            double isoTension = sqrt(1.0 / (1.0 + strainRate * m_config.dt)) - 1.0;

            Mat3d tension{
                { isoTension* pratio, 0.0, 0.0 },
                { 0.0, strainRate* m_config.dt, 0.0 },
                { 0.0, 0.0, isoTension* pratio } };

            defGrad += tension;
            break;
        }
        case ProgrammableClient::DeformationType::SimpleShear:
        {
            double gamma = strainRate * m_config.dt;
            Mat3d  simpleShear{
                { 0.0, gamma, 0.0 },
                { 0.0, 0.0, 0.0 },
                { 0.0, 0.0, 0.0 } };

            defGrad += simpleShear;
            break;
        }
        case ProgrammableClient::DeformationType::PureShear:
        {
            double gamma = strainRate * m_config.dt;
            Mat3d  pureShear{
                { 0.0, gamma, 0.0 },
                { gamma, 0.0, 0.0 },
                { 0.0, 0.0, 0.0 } };

            defGrad += pureShear;
            break;
        }
        }

        // for readability
        double youngsModulus = m_config.youngsModulus;
        double poissons      = m_config.poissonRatio;

        double strainEnergy = 0.0;
        // Use the deformation gradient to calculate the expected strain energy for a unit cube
        switch (m_config.materialType)
        {
        case PbdFemConstraint::MaterialType::StVK:
        {
            Mat3d  greenStrain = 0.5 * (defGrad.transpose() * defGrad - Mat3d::Identity());
            double lambda      = youngsModulus * poissons / ((1.0 + poissons) * (1.0 - 2.0 * poissons));
            double mu = youngsModulus / (2.0 * (1.0 + poissons));

            // double SE = (lambda / 2.0) * greenStrain.trace() * greenStrain.trace() + mu * (greenStrain * greenStrain).trace();

            Mat3d I = Mat3d::Identity();
            Mat3d E = 0.5 * (defGrad.transpose() * defGrad - I);

            // C here is strain energy (Often denoted as W in literature)
            // for the StVK model W = mu[tr(E^{T}E)] + 0.5*lambda*(tr(E))^2
            strainEnergy = mu * ((E.transpose() * E).trace()) + 0.5 * lambda * (E.trace() * E.trace());
            break;
        }
        case PbdFemConstraint::MaterialType::NeoHookean:
        {
            double lambda = youngsModulus * poissons / ((1.0 + poissons) * (1 - 2.0 * poissons));
            double mu     = youngsModulus / (2.0 * (1 + poissons));

            double I1    = (defGrad * defGrad.transpose()).trace();
            double I3    = (defGrad.transpose() * defGrad).determinant();
            auto   logI3 = log(I3);

            // auto F_invT = defGrad.inverse().transpose();

            auto J   = defGrad.determinant();
            auto lnJ = log(J);

            strainEnergy = 0.5 * mu * (I1 - 3.0 - 2 * lnJ) + 0.5 * lambda * lnJ * lnJ;
            break;
        }

        case PbdFemConstraint::MaterialType::Corotation:
        {
            double lambda = youngsModulus * poissons / ((1.0 + poissons) * (1 - 2.0 * poissons));
            double mu     = youngsModulus / (2.0 * (1 + poissons));

            Eigen::JacobiSVD<Mat3d> svd(defGrad, Eigen::ComputeFullU | Eigen::ComputeFullV);
            Mat3d                   R = svd.matrixU() * svd.matrixV().adjoint();
            Vec3d                   Sigma(svd.singularValues());
            Mat3d                   invFT = svd.matrixU();
            invFT.col(0) /= Sigma(0);
            invFT.col(1) /= Sigma(1);
            invFT.col(2) /= Sigma(2);
            invFT *= svd.matrixV().adjoint();
            double J  = Sigma(0) * Sigma(1) * Sigma(2);
            Mat3d  FR = defGrad - R;

            double SE = FR(0, 0) * FR(0, 0) + FR(0, 1) * FR(0, 1) + FR(0, 2) * FR(0, 2)
                        + FR(1, 0) * FR(1, 0) + FR(1, 1) * FR(1, 1) + FR(1, 2) * FR(1, 2)
                        + FR(2, 0) * FR(2, 0) + FR(2, 1) * FR(2, 1) + FR(2, 2) * FR(2, 2);

            strainEnergy = mu * SE + 0.5 * lambda * (J - 1) * (J - 1);
            break;
        }

        case PbdFemConstraint::MaterialType::Linear:
        {
            double lambda = youngsModulus * poissons / ((1.0 + poissons) * (1 - 2.0 * poissons));
            double mu     = youngsModulus / (2.0 * (1 + poissons));
            Mat3d  I      = Mat3d::Identity();
            Mat3d  e      = 0.5 * (defGrad * defGrad.transpose() - I);
            strainEnergy = mu * (e * e).trace() + 0.5 * lambda * e.trace() * e.trace();
            break;
        }
        }

        tracker.probe(seIdx, strainEnergy);
        time += m_config.dt;
        tracker.streamProbesToFile(time);
    }

    return true;
}

bool
UnitCubeStrainScene::postProcessAnalyticResults()
{
    return true;// compareAndPlotTimeSeriesCSVs(m_outDir + "computed.csv", m_outDir + "computed.csv", cmp, m_outDir + "validation_results/");
}
} // namespace imstk
