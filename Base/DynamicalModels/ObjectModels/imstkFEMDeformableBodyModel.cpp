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

namespace imstk
{

FEMDeformableBodyModel::FEMDeformableBodyModel() :
DynamicalModel(DynamicalModelType::elastoDynamics){}

void
FEMDeformableBodyModel::setForceModelConfiguration(std::shared_ptr<ForceModelConfig> fmConfig)
{
    m_forceModelConfiguration = fmConfig;
}

std::shared_ptr<imstk::ForceModelConfig>
FEMDeformableBodyModel::getForceModelConfiguration() const
{
    return m_forceModelConfiguration;
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

void
FEMDeformableBodyModel::setModelGeometry(std::shared_ptr<Geometry> geometry)
{
    m_forceModelGeometry = geometry;
}

std::shared_ptr<imstk::Geometry>
FEMDeformableBodyModel::getModelGeometry()
{
    return m_forceModelGeometry;
}

void
FEMDeformableBodyModel::configure(const std::string& configFileName)
{
    m_forceModelConfiguration = std::make_shared<ForceModelConfig>(configFileName);
}

void
FEMDeformableBodyModel::initialize(std::shared_ptr<VolumetricMesh> physicsMesh)
{
    this->setModelGeometry(physicsMesh);

    // prerequisite of for successfully initializing
    if (!m_forceModelGeometry || !m_forceModelConfiguration)
    {
        LOG(WARNING) << "DeformableBodyModel::initialize: Physics mesh or force model configuration not set yet!";
        return;
    }

    m_vegaPhysicsMesh = physicsMesh->getAttachedVegaMesh();

    this->initializeForceModel();
    this->initializeMassMatrix();
    this->initializeDampingMatrix();
    this->initializeTangentStiffness();
    this->loadInitialStates();
    this->loadBoundaryConditions();
    this->initializeGravityForce();
    this->initializeExplicitExternalForces();

    m_Feff.resize(m_numDOF);
    m_Finternal.resize(m_numDOF);
    m_Finternal.setConstant(0.0);
    m_Fcontact.resize(m_numDOF);
    m_Fcontact.setConstant(0.0);
    m_qSol.resize(m_numDOF);
    m_qSol.setConstant(0.0);
}

void
FEMDeformableBodyModel::loadInitialStates()
{
    // For now the initial states are set to zero
    m_initialState = std::make_shared<kinematicState>(m_numDOF);
    m_previousState = std::make_shared<kinematicState>(m_numDOF);
    m_currentState = std::make_shared<kinematicState>(m_numDOF);
}

void
FEMDeformableBodyModel::loadBoundaryConditions()
{
    auto fileName = m_forceModelConfiguration->getStringOptionsMap().at("fixedDOFFilename");

    if (fileName.empty())
    {
        LOG(WARNING) << "DeformableBodyModel::loadBoundaryConditions: The external boundary conditions file name is empty";
        return;
    }
    else
    {
        std::ifstream file(fileName.data());

        if (file.peek() == std::ifstream::traits_type::eof())
        {
            LOG(INFO) << "DeformableBodyModel::loadBoundaryConditions: The external boundary conditions file is empty";
            return;
        }

        if (file.is_open())
        {
            size_t index;
            auto maxAllowed = m_vegaPhysicsMesh->getNumVertices();
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
                }
            }

            file.close();
            std::sort(m_fixedNodeIds.begin(), m_fixedNodeIds.end());// for efficiency
        }
        else
        {
            LOG(WARNING) << "DeformableBodyModel::loadBoundaryConditions: Could not open boundary conditions file!";
        }
    }
}

void
FEMDeformableBodyModel::initializeForceModel()
{
    const double g = m_forceModelConfiguration->getFloatsOptionsMap().at("gravity");
    const bool isGravityPresent = (g > 0) ? true : false;

    m_numDOF = m_vegaPhysicsMesh->getNumVertices() * 3;

    switch (m_forceModelConfiguration->getForceModelType())
    {
    case ForceModelType::StVK:

        m_internalForceModel = std::make_shared<StVKForceModel>(m_vegaPhysicsMesh, isGravityPresent, g);
        break;

    case ForceModelType::Linear:

        m_internalForceModel = std::make_shared<LinearFEMForceModel>(m_vegaPhysicsMesh, isGravityPresent, g);
        break;

    case ForceModelType::Corotational:

        m_internalForceModel = std::make_shared<CorotationalFEMForceModel>(m_vegaPhysicsMesh);
        break;

    case ForceModelType::Invertible:

        m_internalForceModel = std::make_shared<IsotropicHyperelasticFEForceModel>(
                                        m_forceModelConfiguration->getHyperelasticMaterialType(),
                                        m_vegaPhysicsMesh,
                                        -MAX_D,
                                        isGravityPresent,
                                        g);
        break;

    default:
        LOG(WARNING) << "DeformableBodyModel::initializeForceModel: Unknown force model type";

    } //switch
}

void
FEMDeformableBodyModel::initializeMassMatrix(const bool saveToDisk /*= false*/)
{
    if (!m_forceModelGeometry)
    {
        LOG(WARNING) << "DeformableBodyModel::initializeMassMatrix Force model geometry not set!";
        return;
    }

    vega::SparseMatrix *vegaMatrix;
    vega::GenerateMassMatrix::computeMassMatrix(m_vegaPhysicsMesh.get(), &vegaMatrix, true);//caveat

    m_vegaMassMatrix.reset(vegaMatrix);

    this->initializeEigenMatrixFromVegaMatrix(*vegaMatrix, m_M);

    // TODO: Add option to save mass matrix to file
}

void
FEMDeformableBodyModel::initializeDampingMatrix()
{
    auto dampingLaplacianCoefficient = m_forceModelConfiguration->getFloatsOptionsMap().at("dampingLaplacianCoefficient");
    auto dampingMassCoefficient = m_forceModelConfiguration->getFloatsOptionsMap().at("dampingMassCoefficient");
    auto dampingStiffnessCoefficient = m_forceModelConfiguration->getFloatsOptionsMap().at("dampingStiffnessCoefficient");

    m_damped = (dampingStiffnessCoefficient == 0.0 && dampingLaplacianCoefficient == 0.0 && dampingMassCoefficient == 0.0) ? false : true;

    if (!m_damped)
    {
        return;
    }

    if (dampingLaplacianCoefficient <= 0.0)
    {
        LOG(WARNING) << "DeformableBodyModel::initializeDampingMatrix Damping coefficient is negative!";
        return;
    }

    auto imstkVolMesh = std::static_pointer_cast<VolumetricMesh>(m_forceModelGeometry);
    //std::shared_ptr<vega::VolumetricMesh> vegaMesh = VegaMeshReader::getVegaVolumeMeshFromVolumeMesh(imstkVolMesh);

    auto meshGraph = std::make_shared<vega::Graph>(*vega::GenerateMeshGraph::Generate(m_vegaPhysicsMesh.get()));

    if (!meshGraph)
    {
        LOG(WARNING) << "DeformableBodyModel::initializeDampingMatrix: Mesh graph not avaliable!";
        return;
    }

    vega::SparseMatrix *matrix;
    meshGraph->GetLaplacian(&matrix, 1);

    if (!matrix)
    {
        LOG(WARNING) << "DeformableBodyModel::initializeDampingMatrix: Mesh Laplacian not avaliable!";
        return;
    }

    matrix->ScalarMultiply(dampingLaplacianCoefficient);

    m_vegaDampingMatrix.reset(matrix);

    this->initializeEigenMatrixFromVegaMatrix(*matrix, m_C);
}

void
FEMDeformableBodyModel::initializeTangentStiffness()
{
    if (!m_internalForceModel)
    {
        LOG(WARNING) << "DeformableBodyModel::initializeTangentStiffness: Tangent stiffness cannot be initialized without force model";
        return;
    }

    vega::SparseMatrix *matrix;
    m_internalForceModel->getTangentStiffnessMatrixTopology(&matrix);

    if (!matrix)
    {
        LOG(WARNING) << "DeformableBodyModel::initializeTangentStiffness - Tangent stiffness matrix topology not avaliable!";
        return;
    }

    if (!m_vegaMassMatrix)
    {
        LOG(WARNING) << "DeformableBodyModel::initializeTangentStiffness - Vega mass matrix doesn't exist!";
        return;
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
        const auto &dampingStiffnessCoefficient = m_forceModelConfiguration->getFloatsOptionsMap().at("dampingStiffnessCoefficient");
        const auto &dampingMassCoefficient = m_forceModelConfiguration->getFloatsOptionsMap().at("dampingMassCoefficient");

        // Initialize the Raleigh damping matrix
        m_C = dampingMassCoefficient*m_M + dampingStiffnessCoefficient*m_K;
    }

    m_internalForceModel->setTangentStiffness(m_vegaTangentStiffnessMatrix);
}

void
FEMDeformableBodyModel::initializeGravityForce()
{
    m_Fgravity.resize(m_numDOF);
    m_Fgravity.setZero();
    const double gravity = m_forceModelConfiguration->getFloatsOptionsMap().at("gravity");

    m_vegaPhysicsMesh->computeGravity(m_Fgravity.data(), gravity);
}

void
FEMDeformableBodyModel::computeImplicitSystemRHS(kinematicState& stateAtT,
                                                 kinematicState& newState,
                                                 const stateUpdateType updateType)
{
    auto &uPrev = stateAtT.getQ();
    auto &vPrev = stateAtT.getQDot();
    auto &u = newState.getQ();
    auto &v = newState.getQDot();

    // Do checks if there are uninitialized matrices
    m_internalForceModel->getTangentStiffnessMatrix(u, m_K);
    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case stateUpdateType::deltaVelocity:

        m_Feff = m_K * -(uPrev - u + v* dT);

        if (m_damped)
        {
            m_Feff -= m_C*v;
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
FEMDeformableBodyModel::computeSemiImplicitSystemRHS(kinematicState& stateAtT,
                                                     kinematicState& newState,
                                                     const stateUpdateType updateType)
{
    auto &uPrev = stateAtT.getQ();
    auto &vPrev = stateAtT.getQDot();
    auto &u = newState.getQ();
    auto &v = newState.getQDot();

    // Do checks if there are uninitialized matrices
    m_internalForceModel->getTangentStiffnessMatrix(u, m_K);
    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case stateUpdateType::deltaVelocity:

        m_Feff = m_K * (vPrev * -dT);

        if (m_damped)
        {
            m_Feff -= m_C*vPrev;
        }

        m_internalForceModel->getInternalForce(u, m_Finternal);
        m_Feff -= m_Finternal;
        m_Feff += m_FexplicitExternal;
        m_Feff += m_Fgravity;
        m_Feff += m_Fcontact;
        m_Feff *= dT;

        break;

    default:
        LOG(WARNING) << "FEMDeformableBodyModel::computeSemiImplicitSystemRHS: Update type not supported";
    }
}

void
FEMDeformableBodyModel::computeImplicitSystemLHS(const kinematicState& stateAtT,
                                                 kinematicState& newState,
                                                 const stateUpdateType updateType)
{
    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case stateUpdateType::deltaVelocity:

        this->updateMassMatrix();
        m_internalForceModel->getTangentStiffnessMatrix(newState.getQ(), m_K);
        this->updateDampingMatrix();

        m_Keff = m_M;
        if (m_damped)
        {
            m_Keff += dT*m_C;
        }
        m_Keff += (dT*dT) * m_K;

        break;

    default:
        LOG(WARNING) << "FEMDeformableBodyModel::computeImplicitSystemLHS: Update type not supported";
    }
}

void
FEMDeformableBodyModel::initializeExplicitExternalForces()
{
    m_FexplicitExternal.resize(m_numDOF);
    m_FexplicitExternal.setZero();
}

void
FEMDeformableBodyModel::updateDampingMatrix()
{
    if (m_damped)
    {
        const auto &dampingStiffnessCoefficient = m_forceModelConfiguration->getFloatsOptionsMap().at("dampingStiffnessCoefficient");
        const auto &dampingMassCoefficient = m_forceModelConfiguration->getFloatsOptionsMap().at("dampingMassCoefficient");

        if (dampingMassCoefficient > 0)
        {
            m_C = dampingMassCoefficient*m_M;

            if (dampingStiffnessCoefficient > 0)
            {
                m_C += m_K*dampingStiffnessCoefficient;
            }
        }
        else if(dampingStiffnessCoefficient > 0)
        {
            m_C = m_K*dampingStiffnessCoefficient;
        }
    }
}

void
FEMDeformableBodyModel::applyBoundaryConditions(SparseMatrixd &M, const bool withCompliance) const
{
    double compliance = withCompliance ? 1.0 : 0.0;

    // Set column and row to zero.
    for (auto & index : m_fixedNodeIds)
    {
        auto idx = static_cast<SparseMatrixd::Index>(index);

        for (int k = 0; k < M.outerSize(); ++k)
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

void
FEMDeformableBodyModel::applyBoundaryConditions(Vectord &x) const
{
    for (auto & index : m_fixedNodeIds)
    {
        x(index) = 0.0;
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
    auto volMesh = std::static_pointer_cast<VolumetricMesh>(m_forceModelGeometry);
    auto &u = m_currentState->getQ();
    volMesh->setVertexDisplacements(u);
}

void
FEMDeformableBodyModel::updateBodyPreviousStates()
{
    m_previousState->setU(m_currentState->getQ());
    m_previousState->setV(m_currentState->getQDot());
}

void
FEMDeformableBodyModel::updateBodyStates(const Vectord& solution, const stateUpdateType updateType)
{
    this->updateBodyPreviousStates();
    this->updateBodyIntermediateStates(solution, updateType);
}

void
FEMDeformableBodyModel::updateBodyIntermediateStates(
                        const Vectord& solution,
                        const stateUpdateType updateType)
{
    auto &uPrev = m_previousState->getQ();
    auto &u = m_currentState->getQ();
    auto &v = m_currentState->getQDot();
    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case stateUpdateType::deltaVelocity:
        m_currentState->setV(v + solution);
        m_currentState->setU(uPrev + dT*v);

        break;

    case stateUpdateType::velocity:
        m_currentState->setV(solution);
        m_currentState->setU(uPrev + dT*v);

        break;

    default:
        LOG(WARNING) << "DeformableBodyModel::updateBodyIntermediateStates: Unknown state update type";
    }
    m_qSol = m_currentState->getQ();
}

NonLinearSystem::VectorFunctionType
FEMDeformableBodyModel::getFunction()
{
    // Function to evaluate the nonlinear objective function given the current state
    return[&, this](const Vectord& q, const bool semiImplicit) -> const Vectord&
    {
        (semiImplicit) ?
        this->computeSemiImplicitSystemRHS(*m_previousState.get(), *m_currentState.get(), m_updateType) :
        this->computeImplicitSystemRHS(*m_previousState.get(), *m_currentState.get(), m_updateType);

        return m_Feff;
    };
}

NonLinearSystem::MatrixFunctionType
FEMDeformableBodyModel::getFunctionGradient()
{
    // Gradient of the nonlinear objective function given the current state
    return [&, this](const Vectord& q) -> const SparseMatrixd&
    {
        this->computeImplicitSystemLHS(*m_previousState.get(), *m_currentState.get(), m_updateType);

        return m_Keff;
    };
}

NonLinearSystem::UpdateFunctionType
FEMDeformableBodyModel::getUpdateFunction()
{
    // Function to evaluate the nonlinear objective function given the current state
    return[&, this](const Vectord& q, const bool fullyImplicit) -> void
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
    return[&, this]() -> void
    {
        this->updateBodyPreviousStates();
    };
}

void
FEMDeformableBodyModel::initializeEigenMatrixFromVegaMatrix(const vega::SparseMatrix& vegaMatrix,
                                                            SparseMatrixd& eigenMatrix)
{
    auto rowLengths = vegaMatrix.GetRowLengths();
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

} // imstk