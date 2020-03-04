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

#include <fstream>

//imstk
#include "imstkFEMDeformableBodyModel.h"

// vega
#include "generateMassMatrix.h"
#include "generateMeshGraph.h"
#include "configFile.h"

namespace imstk
{
FEMDeformableBodyModel::FEMDeformableBodyModel() :
    DynamicalModel(DynamicalModelType::elastoDynamics)
{
    m_fixedNodeIds.reserve(1000);
}

FEMDeformableBodyModel::~FEMDeformableBodyModel()
{
    // Get vega to destruct first (before the shared pointer to the vega mesh is cleaned up)
    m_internalForceModel = nullptr;
}

void
FEMDeformableBodyModel::configure(const std::string& configFileName)
{
    // Configure the FEMModelConfig
    m_FEModelConfig = std::make_shared<FEMModelConfig>();

    char femMethod[256];
    char invertibleMaterial[256];
    char fixedDOFFilename[256];

    vega::ConfigFile vegaConfigFileOptions;

    // configure the options
    vegaConfigFileOptions.addOptionOptional("femMethod", femMethod, "StVK");
    vegaConfigFileOptions.addOptionOptional("invertibleMaterial", invertibleMaterial, "StVK");
    vegaConfigFileOptions.addOptionOptional("fixedDOFFilename", fixedDOFFilename, "");
    vegaConfigFileOptions.addOptionOptional("dampingMassCoefficient", &m_FEModelConfig->m_dampingMassCoefficient, m_FEModelConfig->m_dampingMassCoefficient);
    vegaConfigFileOptions.addOptionOptional("dampingStiffnessCoefficient", &m_FEModelConfig->m_dampingStiffnessCoefficient, m_FEModelConfig->m_dampingStiffnessCoefficient);
    vegaConfigFileOptions.addOptionOptional("dampingLaplacianCoefficient", &m_FEModelConfig->m_dampingLaplacianCoefficient, m_FEModelConfig->m_dampingLaplacianCoefficient);
    vegaConfigFileOptions.addOptionOptional("deformationCompliance", &m_FEModelConfig->m_deformationCompliance, m_FEModelConfig->m_deformationCompliance);
    vegaConfigFileOptions.addOptionOptional("compressionResistance", &m_FEModelConfig->m_compressionResistance, m_FEModelConfig->m_compressionResistance);
    vegaConfigFileOptions.addOptionOptional("inversionThreshold", &m_FEModelConfig->m_inversionThreshold, m_FEModelConfig->m_inversionThreshold);
    vegaConfigFileOptions.addOptionOptional("numberOfThreads", &m_FEModelConfig->m_numberOfThreads, m_FEModelConfig->m_numberOfThreads);
    vegaConfigFileOptions.addOptionOptional("gravity", &m_FEModelConfig->m_gravity, m_FEModelConfig->m_gravity);

    // Parse the configuration file
    if (vegaConfigFileOptions.parseOptions(configFileName.data()) != 0)
    {
        LOG(FATAL) << "ForceModelConfig::parseConfig - Unable to load the configuration file";
    }

    // get the root directory of the boundary file name
    std::string  rootDir;
    const size_t last_slash_idx = configFileName.rfind('/');
    if (std::string::npos != last_slash_idx)
    {
        rootDir = configFileName.substr(0, last_slash_idx);
    }

    // Set fem method
    if (std::strcmp(femMethod, "StVK") == 0)
    {
        m_FEModelConfig->m_femMethod = FEMMethodType::StVK;
    }
    else if (std::strcmp(femMethod, "CLFEM") == 0)
    {
        m_FEModelConfig->m_femMethod = FEMMethodType::Corotational;
    }
    else if (std::strcmp(femMethod, "Linear") == 0)
    {
        m_FEModelConfig->m_femMethod = FEMMethodType::Linear;
    }
    else if (std::strcmp(femMethod, "InvertibleFEM") == 0)
    {
        m_FEModelConfig->m_femMethod = FEMMethodType::Invertible;
    }
    else
    {
        LOG(WARNING) << "FE method not assigned; will default to StVK";
        m_FEModelConfig->m_femMethod = FEMMethodType::StVK;
    }

    // Set up hyperelastic material type
    if (std::strcmp(invertibleMaterial, "StVK") == 0)
    {
        m_FEModelConfig->m_hyperElasticMaterialType = HyperElasticMaterialType::StVK;
    }
    else if (std::strcmp(invertibleMaterial, "NeoHookean") == 0)
    {
        m_FEModelConfig->m_hyperElasticMaterialType = HyperElasticMaterialType::NeoHookean;
    }
    else if (std::strcmp(invertibleMaterial, "MooneyRivlin") == 0)
    {
        m_FEModelConfig->m_hyperElasticMaterialType = HyperElasticMaterialType::MooneyRivlin;
    }
    else
    {
        LOG(INFO) << "Hyperelastic material type not assigned; will default to StVK";
        m_FEModelConfig->m_hyperElasticMaterialType = HyperElasticMaterialType::StVK;
    }

    // file names (remove from here?)
    m_FEModelConfig->m_fixedDOFFilename = rootDir + std::string("/") + std::string(fixedDOFFilename);
}

void
FEMDeformableBodyModel::configure(std::shared_ptr<FEMModelConfig> config)
{
    m_FEModelConfig = config;
}

void
FEMDeformableBodyModel::setForceModelConfiguration(std::shared_ptr<FEMModelConfig> fmConfig)
{
    m_FEModelConfig = fmConfig;
}

std::shared_ptr<imstk::FEMModelConfig>
FEMDeformableBodyModel::getForceModelConfiguration() const
{
    return m_FEModelConfig;
}

void
FEMDeformableBodyModel::setInternalForceModel(std::shared_ptr<InternalForceModel> fm)
{
    m_internalForceModel = fm;
}

std::shared_ptr<imstk::InternalForceModel>
FEMDeformableBodyModel::getInternalForceModel() const
{
    return m_internalForceModel;
}

void
FEMDeformableBodyModel::setTimeIntegrator(std::shared_ptr<TimeIntegrator> timeIntegrator)
{
    m_timeIntegrator = timeIntegrator;
}

std::shared_ptr<imstk::TimeIntegrator>
FEMDeformableBodyModel::getTimeIntegrator() const
{
    return m_timeIntegrator;
}

bool
FEMDeformableBodyModel::initialize()
{
    // prerequisite of for successfully initializing
    if (!m_geometry || !m_FEModelConfig)
    {
        LOG(FATAL) << "DeformableBodyModel::initialize: Physics mesh or force model configuration not set yet!";
        return false;
    }

    auto physicsMesh = std::dynamic_pointer_cast<imstk::VolumetricMesh>(this->getModelGeometry());
    m_vegaPhysicsMesh = VegaMeshIO::convertVolumetricMeshToVegaMesh(physicsMesh);
    //m_vegaPhysicsMesh = physicsMesh->getAttachedVegaMesh();
    if (!this->initializeForceModel()
        || !this->initializeMassMatrix()
        || !this->initializeDampingMatrix()
        || !this->initializeTangentStiffness()
        || !this->loadBoundaryConditions()
        || !this->initializeGravityForce()
        || !this->initializeExplicitExternalForces())
    {
        return false;
    }

    this->loadInitialStates();

    m_Feff.resize(m_numDOF);
    m_Finternal.resize(m_numDOF);
    m_Finternal.setConstant(0.0);
    m_Fcontact.resize(m_numDOF);
    m_Fcontact.setConstant(0.0);
    m_qSol.resize(m_numDOF);
    m_qSol.setConstant(0.0);

    return true;
}

void
FEMDeformableBodyModel::loadInitialStates()
{
    if (m_numDOF == 0)
    {
        LOG(WARNING) << "FEMDeformableBodyModel::loadInitialStates() - Num. of degree of freedom is zero!";
    }

    // For now the initial states are set to zero
    m_initialState  = std::make_shared<kinematicState>(m_numDOF);
    m_previousState = std::make_shared<kinematicState>(m_numDOF);
    m_currentState  = std::make_shared<kinematicState>(m_numDOF);
}

bool
FEMDeformableBodyModel::loadBoundaryConditions()
{
    auto fileName = m_FEModelConfig->m_fixedDOFFilename;

    if (fileName.empty())
    {
        // if the list of fixed nodes are set, use them
        for (auto p:m_FEModelConfig->m_fixedNodeIds)
        {
            m_fixedNodeIds.emplace_back(p);
        }
    }
    else
    {
        std::ifstream file(fileName.data());

        if (file.peek() == std::ifstream::traits_type::eof())
        {
            LOG(INFO) << "DeformableBodyModel::loadBoundaryConditions: The external boundary conditions file is empty";
        }

        if (file.is_open())
        {
            size_t index;
            auto   maxAllowed = m_vegaPhysicsMesh->getNumVertices();
            while (!file.eof())
            {
                file >> index;
                if (index < maxAllowed)
                {
                    m_fixedNodeIds.emplace_back(index);
                }
                else
                {
                    LOG(WARNING) << "FEMDeformableBodyModel::loadBoundaryConditions(): " <<
                        "The boundary condition node id provided is greater than number of nodes and hence excluded!!";
                    return false;
                }
            }

            file.close();
            std::sort(m_fixedNodeIds.begin(), m_fixedNodeIds.end());// for efficiency
        }
        else
        {
            LOG(FATAL) << "DeformableBodyModel::loadBoundaryConditions: Could not open boundary conditions file!";
            return false;
        }
    }
    return true;
}

bool
FEMDeformableBodyModel::initializeForceModel()
{
    const double g = m_FEModelConfig->m_gravity;
    const bool   isGravityPresent = (g > 0) ? true : false;

    m_numDOF = m_vegaPhysicsMesh->getNumVertices() * 3;

    switch (m_FEModelConfig->m_femMethod)
    {
    case FEMMethodType::StVK:

        m_internalForceModel = std::make_shared<StVKForceModel>(m_vegaPhysicsMesh, isGravityPresent, g);
        break;

    case FEMMethodType::Linear:

        m_internalForceModel = std::make_shared<LinearFEMForceModel>(m_vegaPhysicsMesh, isGravityPresent, g);
        break;

    case FEMMethodType::Corotational:

        m_internalForceModel = std::make_shared<CorotationalFEMForceModel>(m_vegaPhysicsMesh);
        break;

    case FEMMethodType::Invertible:

        m_internalForceModel = std::make_shared<IsotropicHyperelasticFEForceModel>(
                                        m_FEModelConfig->m_hyperElasticMaterialType,
                                        m_vegaPhysicsMesh,
                                        -MAX_D,
                                        isGravityPresent,
                                        g);
        break;

    default:
        LOG(FATAL) << "DeformableBodyModel::initializeForceModel: Unknown force model type";
        return false;
    }   //switch

    return true;
}

bool
FEMDeformableBodyModel::initializeMassMatrix()
{
    if (!m_geometry)
    {
        LOG(FATAL) << "DeformableBodyModel::initializeMassMatrix Force model geometry not set!";
        return false;
    }

    vega::SparseMatrix* vegaMatrix;
    vega::GenerateMassMatrix::computeMassMatrix(m_vegaPhysicsMesh.get(), &vegaMatrix, true);//caveat

    m_vegaMassMatrix.reset(vegaMatrix);

    this->initializeEigenMatrixFromVegaMatrix(*vegaMatrix, m_M);

    /// \todo Add option to save mass matrix to file

    return true;
}

bool
FEMDeformableBodyModel::initializeDampingMatrix()
{
    auto dampingLaplacianCoefficient = m_FEModelConfig->m_dampingLaplacianCoefficient;
    auto dampingMassCoefficient      = m_FEModelConfig->m_dampingMassCoefficient;
    auto dampingStiffnessCoefficient = m_FEModelConfig->m_dampingStiffnessCoefficient;

    if (dampingStiffnessCoefficient == 0.0 && dampingLaplacianCoefficient == 0.0 && dampingMassCoefficient == 0.0)
    {
        LOG(WARNING) << "FEMDeformableBodyModel::initializeDampingMatrix() - All the damping parameters are zero!";
        return true;
    }

    if (dampingLaplacianCoefficient < 0.0)
    {
        LOG(WARNING) << "DeformableBodyModel::initializeDampingMatrix Damping coefficient is negative!";
        return false;
    }

    auto imstkVolMesh = std::static_pointer_cast<VolumetricMesh>(m_geometry);
    //std::shared_ptr<vega::VolumetricMesh> vegaMesh = VegaMeshReader::getVegaVolumeMeshFromVolumeMesh(imstkVolMesh);

    auto meshGraph = std::make_shared<vega::Graph>(*vega::GenerateMeshGraph::Generate(m_vegaPhysicsMesh.get()));

    if (!meshGraph)
    {
        LOG(WARNING) << "DeformableBodyModel::initializeDampingMatrix: Mesh graph not avaliable!";
        return false;
    }

    vega::SparseMatrix* matrix;
    meshGraph->GetLaplacian(&matrix, 1);

    if (!matrix)
    {
        LOG(WARNING) << "DeformableBodyModel::initializeDampingMatrix: Mesh Laplacian not avaliable!";
        return false;
    }

    matrix->ScalarMultiply(dampingLaplacianCoefficient);

    m_vegaDampingMatrix.reset(matrix);

    this->initializeEigenMatrixFromVegaMatrix(*matrix, m_C);

    m_damped = true;

    return true;
}

bool
FEMDeformableBodyModel::initializeTangentStiffness()
{
    if (!m_internalForceModel)
    {
        LOG(FATAL) << "DeformableBodyModel::initializeTangentStiffness: Tangent stiffness cannot be initialized without force model";
        return false;
    }

    vega::SparseMatrix* matrix;
    m_internalForceModel->getTangentStiffnessMatrixTopology(&matrix);

    if (!matrix)
    {
        LOG(FATAL) << "DeformableBodyModel::initializeTangentStiffness - Tangent stiffness matrix topology not avaliable!";
        return false;
    }

    if (!m_vegaMassMatrix)
    {
        LOG(FATAL) << "DeformableBodyModel::initializeTangentStiffness - Vega mass matrix doesn't exist!";
        return false;
    }

    matrix->BuildSubMatrixIndices(*m_vegaMassMatrix.get());

    if (m_vegaDampingMatrix)
    {
        matrix->BuildSubMatrixIndices(*m_vegaDampingMatrix.get(), 1);
    }

    m_vegaTangentStiffnessMatrix.reset(matrix);

    this->initializeEigenMatrixFromVegaMatrix(*matrix, m_K);

    if (m_damped)
    {
        const auto& dampingStiffnessCoefficient = m_FEModelConfig->m_dampingStiffnessCoefficient;
        const auto& dampingMassCoefficient      = m_FEModelConfig->m_dampingMassCoefficient;

        // Initialize the Raleigh damping matrix
        m_C = dampingMassCoefficient * m_M + dampingStiffnessCoefficient * m_K;
    }

    m_internalForceModel->setTangentStiffness(m_vegaTangentStiffnessMatrix);

    return true;
}

bool
FEMDeformableBodyModel::initializeGravityForce()
{
    m_Fgravity.resize(m_numDOF);
    m_Fgravity.setZero();
    const double gravity = m_FEModelConfig->m_gravity;

    m_vegaPhysicsMesh->computeGravity(m_Fgravity.data(), gravity);

    return true;
}

void
FEMDeformableBodyModel::computeImplicitSystemRHS(kinematicState&       stateAtT,
                                                 kinematicState&       newState,
                                                 const StateUpdateType updateType)
{
    auto& uPrev = stateAtT.getQ();
    auto& vPrev = stateAtT.getQDot();
    auto& u     = newState.getQ();
    auto& v     = newState.getQDot();

    // Do checks if there are uninitialized matrices
    m_internalForceModel->getTangentStiffnessMatrix(u, m_K);
    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case StateUpdateType::deltaVelocity:

        m_Feff = m_K * -(uPrev - u + v * dT);

        if (m_damped)
        {
            m_Feff -= m_C * v;
        }

        m_internalForceModel->getInternalForce(u, m_Finternal);
        m_Feff -= m_Finternal;
        m_Feff += m_FexplicitExternal;
        m_Feff += m_Fgravity;
        m_Feff += m_Fcontact;
        m_Feff *= dT;
        m_Feff += m_M * (vPrev - v);

        break;
    default:
        LOG(WARNING) << "FEMDeformableBodyModel::computeImplicitSystemRHS: Update type not supported";
    }
}

void
FEMDeformableBodyModel::computeSemiImplicitSystemRHS(kinematicState&       stateAtT,
                                                     kinematicState&       newState,
                                                     const StateUpdateType updateType)
{
    auto& uPrev = stateAtT.getQ();
    auto& vPrev = stateAtT.getQDot();
    auto& u     = newState.getQ();
    auto& v     = newState.getQDot();

    // Do checks if there are uninitialized matrices
    m_internalForceModel->getTangentStiffnessMatrix(u, m_K);
    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case StateUpdateType::deltaVelocity:

        m_Feff = m_K * (vPrev * -dT);

        if (m_damped)
        {
            m_Feff -= m_C * vPrev;
        }

        m_internalForceModel->getInternalForce(u, m_Finternal);
        m_Feff -= m_Finternal;
        m_Feff += m_FexplicitExternal;
        m_Feff += m_Fgravity;
        m_Feff += m_Fcontact;
        m_Feff *= dT;

        break;

    default:
        LOG(FATAL) << "FEMDeformableBodyModel::computeSemiImplicitSystemRHS: Update type not supported";
    }
}

void
FEMDeformableBodyModel::computeImplicitSystemLHS(const kinematicState& stateAtT,
                                                 kinematicState&       newState,
                                                 const StateUpdateType updateType)
{
    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case StateUpdateType::deltaVelocity:

        this->updateMassMatrix();
        m_internalForceModel->getTangentStiffnessMatrix(newState.getQ(), m_K);
        this->updateDampingMatrix();

        m_Keff = m_M;
        if (m_damped)
        {
            m_Keff += dT * m_C;
        }
        m_Keff += (dT * dT) * m_K;

        break;

    default:
        LOG(FATAL) << "FEMDeformableBodyModel::computeImplicitSystemLHS: Update type not supported";
    }
}

bool
FEMDeformableBodyModel::initializeExplicitExternalForces()
{
    m_FexplicitExternal.resize(m_numDOF);
    m_FexplicitExternal.setZero();

    return true;
}

void
FEMDeformableBodyModel::updateDampingMatrix()
{
    if (m_damped)
    {
        const auto& dampingStiffnessCoefficient = m_FEModelConfig->m_dampingStiffnessCoefficient;
        const auto& dampingMassCoefficient      = m_FEModelConfig->m_dampingMassCoefficient;

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
}

void
FEMDeformableBodyModel::applyBoundaryConditions(SparseMatrixd& M, const bool withCompliance) const
{
    double compliance = withCompliance ? 1.0 : 0.0;

    // Set column and row to zero.
    for (auto& index : m_fixedNodeIds)
    {
        auto nodeIdx = static_cast<SparseMatrixd::Index>(index) * 3;

        for (auto idx = nodeIdx; idx < nodeIdx + 3; idx++)
        {
            for (long long k = 0; k < M.outerSize(); ++k)
            {
                for (SparseMatrixd::InnerIterator i(M, k); i; ++i)
                {
                    if (i.row() == idx || i.col() == idx)
                    {
                        i.valueRef() = 0.0;
                    }

                    if (i.row() == idx && i.col() == idx)
                    {
                        i.valueRef() = compliance;
                    }
                }
            }
        }
    }
}

void
FEMDeformableBodyModel::applyBoundaryConditions(Vectord& x) const
{
    for (auto& index : m_fixedNodeIds)
    {
        const auto _3Index = 3 * index;
        x(_3Index) = x(_3Index + 1) = x(_3Index + 2) = 0.0;
    }
}

void
FEMDeformableBodyModel::updateMassMatrix()
{
    // Do nothing for now as topology changes are not supported yet!
}

void
FEMDeformableBodyModel::updatePhysicsGeometry()
{
    auto  volMesh = std::static_pointer_cast<VolumetricMesh>(m_geometry);
    auto& u       = m_currentState->getQ();
    volMesh->setVertexDisplacements(u);
}

void
FEMDeformableBodyModel::updateBodyPreviousStates()
{
    m_previousState->setU(m_currentState->getQ());
    m_previousState->setV(m_currentState->getQDot());
}

void
FEMDeformableBodyModel::updateBodyStates(const Vectord& solution, const StateUpdateType updateType)
{
    this->updateBodyPreviousStates();
    this->updateBodyIntermediateStates(solution, updateType);
}

void
FEMDeformableBodyModel::updateBodyIntermediateStates(
    const Vectord&        solution,
    const StateUpdateType updateType)
{
    auto&        uPrev = m_previousState->getQ();
    auto&        u     = m_currentState->getQ();
    auto&        v     = m_currentState->getQDot();
    const double dT    = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case StateUpdateType::deltaVelocity:
        m_currentState->setV(v + solution);
        m_currentState->setU(uPrev + dT * v);

        break;

    case StateUpdateType::velocity:
        m_currentState->setV(solution);
        m_currentState->setU(uPrev + dT * v);

        break;

    default:
        LOG(FATAL) << "DeformableBodyModel::updateBodyIntermediateStates: Unknown state update type";
    }
    m_qSol = m_currentState->getQ();
}

NonLinearSystem::VectorFunctionType
FEMDeformableBodyModel::getFunction()
{
    // Function to evaluate the nonlinear objective function given the current state
    return [&, this](const Vectord &q, const bool semiImplicit)->const Vectord &
           {
               (semiImplicit) ?
               this->computeSemiImplicitSystemRHS(*m_previousState.get(), *m_currentState.get(), m_updateType) :
               this->computeImplicitSystemRHS(*m_previousState.get(), *m_currentState.get(), m_updateType);
               if (this->m_implementFixedBC)
               {
                   applyBoundaryConditions(m_Feff);
               }
               return m_Feff;
           };
}

NonLinearSystem::MatrixFunctionType
FEMDeformableBodyModel::getFunctionGradient()
{
    // Gradient of the nonlinear objective function given the current state
    return [&, this](const Vectord &q)->const SparseMatrixd &
           {
               this->computeImplicitSystemLHS(*m_previousState.get(), *m_currentState.get(), m_updateType);

               if (this->m_implementFixedBC)
               {
                   applyBoundaryConditions(m_Keff);
               }
               return m_Keff;
           };
}

NonLinearSystem::UpdateFunctionType
FEMDeformableBodyModel::getUpdateFunction()
{
    // Function to evaluate the nonlinear objective function given the current state
    return [&, this](const Vectord &q, const bool fullyImplicit)->void
           {
               (fullyImplicit) ?
               this->updateBodyIntermediateStates(q, m_updateType) :
               this->updateBodyStates(q, m_updateType);
           };
}

NonLinearSystem::UpdatePrevStateFunctionType
FEMDeformableBodyModel::getUpdatePrevStateFunction()
{
    // Function to evaluate the nonlinear objective function given the current state
    return [&, this]()->void
           {
               this->updateBodyPreviousStates();
           };
}

void
FEMDeformableBodyModel::initializeEigenMatrixFromVegaMatrix(const vega::SparseMatrix& vegaMatrix,
                                                            SparseMatrixd&            eigenMatrix)
{
    auto rowLengths    = vegaMatrix.GetRowLengths();
    auto nonZeroValues = vegaMatrix.GetEntries();
    auto columnIndices = vegaMatrix.GetColumnIndices();

    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(vegaMatrix.GetNumEntries());
    for (int i = 0, end = vegaMatrix.GetNumRows(); i < end; ++i)
    {
        for (int k = 0, k_end = rowLengths[i]; k < k_end; ++k)
        {
            triplets.emplace_back(i, columnIndices[i][k], nonZeroValues[i][k]);
        }
    }
    eigenMatrix.resize(vegaMatrix.GetNumRows(), vegaMatrix.GetNumColumns());
    eigenMatrix.setFromTriplets(std::begin(triplets), std::end(triplets));
    eigenMatrix.makeCompressed();
}

Vectord&
FEMDeformableBodyModel::getContactForce()
{
    return m_Fcontact;
}

void
FEMDeformableBodyModel::setFixedSizeTimeStepping()
{
    m_timeStepSizeType = TimeSteppingType::fixed;
    m_timeIntegrator->setTimestepSizeToDefault();
}
} // imstk