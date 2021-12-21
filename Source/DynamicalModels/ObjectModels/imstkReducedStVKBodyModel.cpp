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

// std lib
#include <fstream>
#include <ios>
#include <iomanip>

// imstk
#include "imstkMath.h"
#include "imstkNewtonSolver.h"
#include "imstkReducedStVKBodyModel.h"
#include "imstkTimeIntegrator.h"
#include "imstkVegaMeshIO.h"
#include "imstkVolumetricMesh.h"
#include "imstkTaskGraph.h"
#include "imstkNewtonSolver.h"

// vega
#include "StVKReducedInternalForces.h"
#include "matrixIO.h"
#include "modalMatrix.h"
#include "reducedStVKForceModel.h"

#pragma warning(push)
#pragma warning(disable : 4458)
#include "configFile.h"
#pragma warning(pop)

namespace imstk
{
ReducedStVK::ReducedStVK() : DynamicalModel(DynamicalModelType::ElastoDynamics)
{
    // m_fixedNodeIds.reserve(1000);

    m_validGeometryTypes = { "TetrahedralMesh", "HexahedralMesh" };

    m_solveNode = m_taskGraph->addFunction("FEMModel_Solve", [&]() { getSolver()->solve(); });
}

ReducedStVK::~ReducedStVK()
{
    // Get vega to destruct first (before the shared pointer to the vega mesh is cleaned up)
    m_internalForceModel = nullptr;
}

void
ReducedStVK::configure(const std::string& configFileName)
{
    std::cerr << "not implemented yet" << std::endl;
    // Configure the ReducedStVKConfig
    m_config = std::make_shared<ReducedStVKConfig>();

    // char femMethod[256];
    // char invertibleMaterial[256];
    // char fixedDOFFilename[256];

    vega::ConfigFile vegaConfigFileOptions;

    // configure the options
    vegaConfigFileOptions.addOptionOptional("dampingMassCoefficient",
                                            &m_config->m_dampingMassCoefficient,
                                            m_config->m_dampingMassCoefficient);
    vegaConfigFileOptions.addOptionOptional("dampingStiffnessCoefficient",
                                            &m_config->m_dampingStiffnessCoefficient,
                                            m_config->m_dampingStiffnessCoefficient);
    vegaConfigFileOptions.addOptionOptional("dampingLaplacianCoefficient",
                                            &m_config->m_dampingLaplacianCoefficient,
                                            m_config->m_dampingLaplacianCoefficient);
    vegaConfigFileOptions.addOptionOptional("numberOfThreads",
                                            &m_config->m_numberOfThreads,
                                            m_config->m_numberOfThreads);
    vegaConfigFileOptions.addOptionOptional("gravity", &m_config->m_gravity, m_config->m_gravity);

    // Parse the configuration file
    CHECK(vegaConfigFileOptions.parseOptions(configFileName.data()) == 0)
        << "ForceModelConfig::parseConfig - Unable to load the configuration file";

    // get the root directory of the boundary file name
    // std::string  rootDir;
    // const size_t last_slash_idx = configFileName.rfind('/');
    // if (std::string::npos != last_slash_idx)
    // {
    //     rootDir = configFileName.substr(0, last_slash_idx);
    // }

    // file names (remove from here?)
    // m_config->m_fixedDOFFilename = rootDir + std::string("/") + std::string(fixedDOFFilename);
}

void
ReducedStVK::configure(std::shared_ptr<ReducedStVKConfig> config)
{
    m_config = config;
}

void
ReducedStVK::setForceModelConfiguration(std::shared_ptr<ReducedStVKConfig> fmConfig)
{
    m_config = fmConfig;
}

std::shared_ptr<imstk::ReducedStVKConfig>
ReducedStVK::getForceModelConfiguration() const
{
    return m_config;
}

void
// ReducedStVK::setInternalForceModel(std::shared_ptr<InternalForceModel> fm)
ReducedStVK::setInternalForceModel(std::shared_ptr<vega::StVKReducedInternalForces> fm)
{
    m_internalForceModel = fm;
}

std::shared_ptr<imstk::InternalForceModel>
ReducedStVK::getInternalForceModel() const
{
    // return m_internalForceModel;
    LOG(FATAL) << "current implementation can not return an imstk::InternalForceModel.";
    // todo: to implement this function, we have to
    // 1) template InternalForceModel with SystemMatrix
    // 2) define ReducedStVKForceModel by inherit from InternalForceModel
    // 3) replace vega::StVKReducedInternalForces with ReducedStVKForceModel
    return nullptr;
}

void
ReducedStVK::setTimeIntegrator(std::shared_ptr<TimeIntegrator> timeIntegrator)
{
    m_timeIntegrator = timeIntegrator;
}

std::shared_ptr<imstk::TimeIntegrator>
ReducedStVK::getTimeIntegrator() const
{
    return m_timeIntegrator;
}

bool
ReducedStVK::initialize()
{
    // prerequisite of for successfully initializing
    CHECK(m_geometry != nullptr && m_config != nullptr)
        << "Physics mesh or force model configuration not set yet!";

    // Setup default solver if model wasn't assigned one
    if (m_solver == nullptr)
    {
        // create a nonlinear system
        auto nlSystem = std::make_shared<NonLinearSystem<Matrixd>>(getFunction(), getFunctionGradient());

        nlSystem->setUnknownVector(getUnknownVec());
        nlSystem->setUpdateFunction(getUpdateFunction());
        nlSystem->setUpdatePreviousStatesFunction(getUpdatePrevStateFunction());

        // create a non-linear solver and add to the scene
        auto nlSolver = std::make_shared<NewtonSolver<Matrixd>>();
        nlSolver->setToSemiImplicit();
        // nlSolver->setLinearSolver(linSolver);
        nlSolver->setMaxIterations(1);
        nlSolver->setSystem(nlSystem);
        setSolver(nlSolver);
    }

    // This will specify \p m_numDOF and \p m_numDOFReduced
    this->readModalMatrix(m_config->m_modesFileName);
    this->loadInitialStates();

    auto physicsMesh = std::dynamic_pointer_cast<imstk::VolumetricMesh>(this->getModelGeometry());
    m_vegaPhysicsMesh = VegaMeshIO::convertVolumetricMeshToVegaMesh(physicsMesh);
    CHECK(m_numDOF == m_vegaPhysicsMesh->getNumVertices() * 3);

    if (!this->initializeForceModel() || !this->initializeMassMatrix()
        || !this->initializeTangentStiffness() || !this->initializeDampingMatrix()
        || !this->initializeGravityForce() || !this->initializeExplicitExternalForces())
    {
        return false;
    }

    // reduced-space
    m_Feff.resize(m_numDOFReduced);
    m_Finternal.resize(m_numDOFReduced);
    m_Finternal.setConstant(0.0);
    m_qSolReduced.resize(m_numDOFReduced);
    m_qSolReduced.setConstant(0.0);
    m_FcontactReduced.resize(m_numDOFReduced);
    m_FgravityReduced.resize(m_numDOFReduced);
    m_FexplicitExternalReduced.resize(m_numDOFReduced);

    // full-space variable
    m_Fcontact.resize(m_numDOF);
    m_Fcontact.setConstant(0.0);
    m_qSol.resize(m_numDOF);
    m_qSol.setConstant(0.0);

    return true;
}

void
ReducedStVK::readModalMatrix(const std::string& fname)
{
    std::vector<float> Ufloat;
    int                m, n;
    vega::ReadMatrixFromDisk_(fname.c_str(), m, n, Ufloat);
    m_numDOF = m;
    m_numDOFReduced = n;
    std::vector<double> Udouble(Ufloat.size());

    for (size_t i = 0; i < Ufloat.size(); ++i)
    {
        Udouble[i] = Ufloat[i];
    }

    m_modalMatrix =
        std::make_shared<vega::ModalMatrix>(m_numDOF / 3, m_numDOFReduced, Udouble.data());
    return;
}

void
ReducedStVK::loadInitialStates()
{
    if (m_numDOF == 0 || m_numDOFReduced == 0)
    {
        LOG(WARNING) << "Num. of degree of freedom is zero!";
    }

    // For now the initial states are set to zero
    m_initialState  = std::make_shared<kinematicState>(m_numDOF);
    m_previousState = std::make_shared<kinematicState>(m_numDOF);
    m_currentState  = std::make_shared<kinematicState>(m_numDOF);

    m_initialStateReduced  = std::make_shared<kinematicState>(m_numDOFReduced);
    m_previousStateReduced = std::make_shared<kinematicState>(m_numDOFReduced);
    m_currentStateReduced  = std::make_shared<kinematicState>(m_numDOFReduced);
}

bool
ReducedStVK::initializeForceModel()
{
    const double g = m_config->m_gravity;
    const bool   isGravityPresent = (g > 0) ? true : false;

    // m_numDOFReduced = m_config->r;
    m_internalForceModel = std::make_shared<vega::StVKReducedInternalForces>(
            m_config->m_cubicPolynomialFilename.c_str(),
            m_numDOFReduced);
    m_forceModel = std::make_shared<vega::ReducedStVKForceModel>(m_internalForceModel.get());

    return true;
}

bool
ReducedStVK::initializeMassMatrix()
{
    CHECK(m_geometry != nullptr)
        << "Force model geometry not set!";
    this->m_massMatrix.resize(m_numDOFReduced * m_numDOFReduced, 0.0);

    // set M to identity
    for (size_t i = 0; i < m_numDOFReduced; ++i)
    {
        this->m_massMatrix[i * m_numDOFReduced + i] = 1.0;
    }
    this->m_M.resize(m_numDOFReduced, m_numDOFReduced);
    this->initializeEigenMatrixFromStdVector(m_massMatrix, m_M);

    return true;
}

bool
ReducedStVK::initializeDampingMatrix()
{
    // \todo: only mass dampings are considered here.
    this->m_dampingMatrix.resize(m_numDOFReduced * m_numDOFReduced, 0.0);
    auto cM = m_config->m_dampingMassCoefficient;
    auto cK = m_config->m_dampingStiffnessCoefficient;
    for (size_t i = 0; i < m_dampingMatrix.size(); ++i)
    {
        m_dampingMatrix[i] = m_massMatrix[i] * cM + m_stiffnessMatrix[i] * cK;
    }
    this->m_C.resize(m_numDOFReduced, m_numDOFReduced);
    this->initializeEigenMatrixFromStdVector(m_massMatrix, m_C);
    m_damped = true;

    return true;
}

bool
ReducedStVK::initializeTangentStiffness()
{
    CHECK(m_forceModel != nullptr)
        << "Tangent stiffness cannot be initialized without force model";

    this->m_stiffnessMatrix.resize(m_numDOFReduced * m_numDOFReduced, 0.0);
    m_forceModel->GetTangentStiffnessMatrix(m_initialStateReduced->getQ().data(),
                                            m_stiffnessMatrix.data());
    this->m_K.resize(m_numDOFReduced, m_numDOFReduced);
    this->initializeEigenMatrixFromStdVector(m_stiffnessMatrix, m_K);
    return true;
}

bool
ReducedStVK::initializeGravityForce()
{
    m_Fgravity.resize(m_numDOF);
    const double gravity = m_config->m_gravity;

    m_vegaPhysicsMesh->computeGravity(m_Fgravity.data(), gravity);
    m_FgravityReduced.resize(m_numDOFReduced);
    this->project(m_Fgravity, m_FgravityReduced);

    return true;
}

void
ReducedStVK::computeImplicitSystemRHS(kinematicState&       stateAtT,
                                      kinematicState&       newState,
                                      const StateUpdateType updateType)
{
    const auto& uPrev = stateAtT.getQ();
    const auto& vPrev = stateAtT.getQDot();
    auto&       u     = newState.getQ();
    const auto& v     = newState.getQDot();

    // Do checks if there are uninitialized matrices
    m_forceModel->GetTangentStiffnessMatrix(u.data(), m_stiffnessMatrix.data());
    this->initializeEigenMatrixFromStdVector(m_stiffnessMatrix, m_K);

    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case StateUpdateType::DeltaVelocity:

        m_Feff = m_K * -(uPrev - u + v * dT);

        if (m_damped)
        {
            m_Feff -= m_C * v;
        }

        m_forceModel->GetInternalForce(u.data(), m_Finternal.data());
        m_Feff -= m_Finternal;
        this->project(m_FexplicitExternal, m_FexplicitExternalReduced);
        m_Feff += m_FexplicitExternalReduced;
        // the reduced gravity has alredy been initialized in initializeGravityForce
        // this->project(m_Fgravity, m_FgravityReduced);
        m_Feff += m_FgravityReduced;
        this->project(m_Fcontact, m_FcontactReduced);
        m_Feff += m_FcontactReduced;
        m_Feff *= dT;
        m_Feff += m_M * (vPrev - v);

        break;
    default:
        LOG(WARNING) << "Update type not supported";
    }
}

void
ReducedStVK::computeSemiImplicitSystemRHS(kinematicState&       stateAtT,
                                          kinematicState&       newState,
                                          const StateUpdateType updateType)
{
    // auto& uPrev = stateAtT.getQ();
    const auto& vPrev = stateAtT.getQDot();
    auto&       u     = newState.getQ();
    // auto& v     = newState.getQDot();

    // Do checks if there are uninitialized matrices
    m_forceModel->GetTangentStiffnessMatrix(u.data(), m_stiffnessMatrix.data());
    this->initializeEigenMatrixFromStdVector(m_stiffnessMatrix, m_K);

    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case StateUpdateType::DeltaVelocity:

        m_Feff = m_K * (vPrev * -dT);

        if (m_damped)
        {
            m_Feff -= m_C * vPrev;
        }

        m_forceModel->GetInternalForce(u.data(), m_Finternal.data());
        m_Feff -= m_Finternal;
        this->project(m_FexplicitExternal, m_FexplicitExternalReduced);
        m_Feff += m_FexplicitExternalReduced;
        this->project(m_Fgravity, m_FgravityReduced);
        m_Feff += m_FgravityReduced;
        this->project(m_Fcontact, m_FcontactReduced);
        m_Feff += m_FcontactReduced;
        m_Feff *= dT;

        break;

    default:
        LOG(FATAL) << "Update type not supported";
    }
}

void
ReducedStVK::computeImplicitSystemLHS(const kinematicState& stateAtT,
                                      kinematicState&       newState,
                                      const StateUpdateType updateType)
{
    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case StateUpdateType::DeltaVelocity:

        stateAtT;      // supress warning (state is not used in this update type hence can be
                       // ignored)

        this->updateMassMatrix();
        m_forceModel->GetTangentStiffnessMatrix(newState.getQ().data(),
                                                    m_stiffnessMatrix.data());
        this->initializeEigenMatrixFromStdVector(m_stiffnessMatrix, m_K);
        this->updateDampingMatrix();

        m_Keff = m_M;
        if (m_damped)
        {
            m_Keff += dT * m_C;
        }
        m_Keff += (dT * dT) * m_K;

        break;

    default:
        LOG(FATAL) << "Update type not supported";
    }
}

bool
ReducedStVK::initializeExplicitExternalForces()
{
    m_FexplicitExternal.resize(m_numDOF);
    m_FexplicitExternal.setZero();
    m_FexplicitExternalReduced.resize(m_numDOFReduced);
    m_FexplicitExternalReduced.setZero();

    return true;
}

void
ReducedStVK::updateDampingMatrix()
{
    if (!m_damped)
    {
        return;
    }

    const auto& dampingStiffnessCoefficient = m_config->m_dampingStiffnessCoefficient;
    const auto& dampingMassCoefficient      = m_config->m_dampingMassCoefficient;

    if (dampingMassCoefficient > 0)
    {
        m_C = dampingMassCoefficient * m_M;

        if (dampingStiffnessCoefficient > 0)
        {
            m_C += m_K * dampingStiffnessCoefficient;
        }
    }
    else if (dampingStiffnessCoefficient > 0)
    {
        m_C = m_K * dampingStiffnessCoefficient;
    }
}

void
ReducedStVK::applyBoundaryConditions(Matrixd& M, const bool withCompliance) const
{
    // I believe there is nothing to do here since the full space has already been constrained
    // before the reduction.
    return;
}

void
ReducedStVK::applyBoundaryConditions(Vectord& x) const
{
    LOG(WARNING) << "Boundary conditions are not not allowed to change";
}

void
ReducedStVK::updateMassMatrix()
{
    // Do nothing for now as topology changes are not supported yet!
}

void
ReducedStVK::updatePhysicsGeometry()
{
    auto  volMesh  = std::static_pointer_cast<VolumetricMesh>(m_geometry);
    auto& uReduced = m_currentStateReduced->getQ();
    auto& uFull    = m_currentState->getQ();
    this->prolongate(uReduced, uFull);
    volMesh->setVertexDisplacements(uFull);
}

void
ReducedStVK::updateBodyPreviousStates()
{
    // full-space
    m_previousStateReduced->setU(m_currentStateReduced->getQ());
    m_previousStateReduced->setV(m_currentStateReduced->getQDot());
    prolongate(*m_previousStateReduced, *m_previousState);
}

void
ReducedStVK::updateBodyStates(const Vectord& solution, const StateUpdateType updateType)
{
    this->updateBodyPreviousStates();
    this->updateBodyIntermediateStates(solution, updateType);
}

void
ReducedStVK::updateBodyIntermediateStates(const Vectord& solution, const StateUpdateType updateType)
{
    const auto& uPrev = m_previousStateReduced->getQ();
    // auto&        u     = m_currentState->getQ();
    const auto&  v  = m_currentStateReduced->getQDot();
    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case StateUpdateType::DeltaVelocity:
        m_currentStateReduced->setV(v + solution);
        m_currentStateReduced->setU(uPrev + dT * v);

        break;

    case StateUpdateType::Velocity:
        m_currentStateReduced->setV(solution);
        m_currentStateReduced->setU(uPrev + dT * v);

        break;

    default:
        LOG(FATAL) << "Unknown state update type";
    }
    prolongate(*m_currentStateReduced, *m_currentState);
    m_qSolReduced = m_currentStateReduced->getQ();
}

NonLinearSystem<Matrixd>::VectorFunctionType
ReducedStVK::getFunction()
{
#pragma warning(push)
#pragma warning(disable : 4100)

    // Function to evaluate the nonlinear objective function given the current state
    return [&, this](const Vectord& q, const bool semiImplicit) -> const Vectord& {
               (semiImplicit) ? this->computeSemiImplicitSystemRHS(*m_previousStateReduced,
                                                            *m_currentStateReduced,
                                                            m_updateType)
               : this->computeImplicitSystemRHS(*m_previousStateReduced,
                                                        *m_currentStateReduced,
                                                        m_updateType);
               return m_Feff;
           };

#pragma warning(pop)
}

NonLinearSystem<Matrixd>::MatrixFunctionType
ReducedStVK::getFunctionGradient()
{
#pragma warning(push)
#pragma warning(disable : 4100)
    // Gradient of the nonlinear objective function given the current state
    return [&, this](const Vectord& q) -> const Matrixd& {
               this->computeImplicitSystemLHS(*m_previousStateReduced,
                                       *m_currentStateReduced,
                                       m_updateType);
               return m_Keff;
           };

#pragma warning(pop)
}

NonLinearSystem<Matrixd>::UpdateFunctionType
ReducedStVK::getUpdateFunction()
{
    // Function to evaluate the nonlinear objective function given the current state
    return [&, this](const Vectord& q, const bool fullyImplicit) -> void {
               (fullyImplicit) ? this->updateBodyIntermediateStates(q, m_updateType)
               : this->updateBodyStates(q, m_updateType);
           };
}

NonLinearSystem<Matrixd>::UpdatePrevStateFunctionType
ReducedStVK::getUpdatePrevStateFunction()
{
    // Function to evaluate the nonlinear objective function given the current state
    return [&, this]() -> void { this->updateBodyPreviousStates(); };
}

void
ReducedStVK::initializeEigenMatrixFromStdVector(const std::vector<double>& A, Matrixd& eigenMatrix)
{
    CHECK(eigenMatrix.rows() == eigenMatrix.cols());
    CHECK(A.size() == eigenMatrix.rows() * eigenMatrix.cols());

    // A is column-major
    for (size_t j = 0; j < eigenMatrix.cols(); ++j)
    {
        size_t offset = j * eigenMatrix.rows();
        for (size_t i = 0; i < eigenMatrix.rows(); ++i)
        {
            eigenMatrix(i, j) = A[i + offset];
        }
    }
}

Vectord&
ReducedStVK::getContactForce()
{
    return m_Fcontact;
}

void
ReducedStVK::setFixedSizeTimeStepping()
{
    m_timeStepSizeType = TimeSteppingType::Fixed;
    m_timeIntegrator->setTimestepSizeToDefault();
}

void
ReducedStVK::setTimeStep(const double timeStep)
{
    m_timeIntegrator->setTimestepSize(timeStep);
}

double
ReducedStVK::getTimeStep() const
{
    return m_timeIntegrator->getTimestepSize();
};

void
ReducedStVK::prolongate(const Vectord& uReduced, Vectord& u) const
{
    //\todo: check sizes
    m_modalMatrix->AssembleVector(const_cast<double*>(uReduced.data()), u.data());
}

void
ReducedStVK::prolongate(kinematicState& uReduced, kinematicState& u) const
{
    this->prolongate(uReduced.getQ(), u.getQ());
    this->prolongate(uReduced.getQDot(), u.getQDot());
}

void
ReducedStVK::project(const Vectord& u, Vectord& uReduced) const
{
    m_modalMatrix->ProjectVector(const_cast<double*>(u.data()), uReduced.data());
}

void
ReducedStVK::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Setup graph connectivity
    m_taskGraph->addEdge(source, m_solveNode);
    m_taskGraph->addEdge(m_solveNode, sink);
}
}  // namespace imstk
