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
#include "imstkDeformableBodyModel.h"

// vega
#include "generateMassMatrix.h"
#include "generateMeshGraph.h"

namespace imstk
{

DeformableBodyModel::DeformableBodyModel() :
DynamicalModel(DynamicalModelType::elastoDynamics), m_damped(false){}

void
DeformableBodyModel::setForceModelConfiguration(std::shared_ptr<ForceModelConfig> fmConfig)
{
    m_forceModelConfiguration = fmConfig;
}

std::shared_ptr<imstk::ForceModelConfig>
DeformableBodyModel::getForceModelConfiguration() const
{
    return m_forceModelConfiguration;
}

void
DeformableBodyModel::setInternalForceModel(std::shared_ptr<InternalForceModel> fm)
{
    m_internalForceModel = fm;
}

std::shared_ptr<imstk::InternalForceModel>
DeformableBodyModel::getInternalForceModel() const
{
    return m_internalForceModel;
}

void
DeformableBodyModel::setTimeIntegrator(std::shared_ptr<TimeIntegrator> timeIntegrator)
{
    m_timeIntegrator = timeIntegrator;
}

std::shared_ptr<imstk::TimeIntegrator>
DeformableBodyModel::getTimeIntegrator() const
{
    return m_timeIntegrator;
}

void
DeformableBodyModel::setModelGeometry(std::shared_ptr<Geometry> geometry)
{
    m_forceModelGeometry = geometry;
}

std::shared_ptr<imstk::Geometry>
DeformableBodyModel::getModelGeometry()
{
    return m_forceModelGeometry;
}

void
DeformableBodyModel::configure(const std::string& configFileName)
{
    m_forceModelConfiguration = std::make_shared<ForceModelConfig>(configFileName);
}

void
DeformableBodyModel::initialize(std::shared_ptr<VolumetricMesh> physicsMesh)
{
    this->setModelGeometry(physicsMesh);

    // prerequisite of for successfully initializing
    if (!m_forceModelGeometry || !m_forceModelConfiguration)
    {
        LOG(WARNING) << "DeformableBodyModel::initialize: Physics mesh or force model configuration not set yet!";
        return;
    }

    m_vegaPhysicsMesh = physicsMesh->getAttachedVegaMesh();

    initializeForceModel();
    initializeMassMatrix();
    //initializeDampingMatrix();
    initializeTangentStiffness();
    loadInitialStates();
    loadBoundaryConditions();
    initializeGravityForce();
    initializeExplicitExternalForces();

    m_Feff.resize(m_numDOF);
    m_Finternal.resize(m_numDOF);
    m_Finternal.setConstant(0.0);//?
    m_Fcontact.resize(m_numDOF);
    m_tmpStorage.resize(m_numDOF);

    m_qSol.resize(m_numDOF);
    m_qSol.setConstant(0.0);
}

void
DeformableBodyModel::loadInitialStates()
{
    // For now the initial states are set to zero
    m_initialState = std::make_shared<kinematicState>(m_numDOF);
    m_previousState = std::make_shared<kinematicState>(m_numDOF);
    m_currentState = std::make_shared<kinematicState>(m_numDOF);
}

void
DeformableBodyModel::loadBoundaryConditions()
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

        if (file.is_open())
        {
            size_t index;
            while (!file.eof())
            {
                file >> index;
                m_fixedNodeIds.emplace_back(index);
            }
        }
        else
        {
            LOG(WARNING) << "DeformableBodyModel::loadBoundaryConditions: Could not open external file with boundary conditions";
            return;
        }

        file.close();
        return;
    }

}

void
DeformableBodyModel::initializeForceModel()
{
    const float g = m_forceModelConfiguration->getFloatsOptionsMap().at("gravity");
    const bool isGravityPresent = (g > 0) ? true : false;

    m_numDOF = m_vegaPhysicsMesh->getNumVertices() * 3;

    switch (m_forceModelConfiguration->getForceModelType())
    {
    case ForceModelType::StVK:

        this->m_internalForceModel = std::make_shared<StVKForceModel>(m_vegaPhysicsMesh, isGravityPresent, g);
        break;

    case ForceModelType::Linear:

        this->m_internalForceModel = std::make_shared<LinearFEMForceModel>(m_vegaPhysicsMesh, isGravityPresent, g);
        break;

    case ForceModelType::Corotational:

        this->m_internalForceModel = std::make_shared<CorotationalFEMForceModel>(m_vegaPhysicsMesh);
        break;

    case ForceModelType::Invertible:

        this->m_internalForceModel = std::make_shared<IsotropicHyperelasticFEForceModel>(
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
DeformableBodyModel::initializeMassMatrix(const bool saveToDisk /*= false*/)
{
    if (!this->m_forceModelGeometry)
    {
        LOG(WARNING) << "DeformableBodyModel::initializeMassMatrix Force model geometry not set!";
        return;
    }

    vega::SparseMatrix *vegaMatrix;
    vega::GenerateMassMatrix::computeMassMatrix(m_vegaPhysicsMesh.get(), &vegaMatrix, true);//caveat

    this->m_vegaMassMatrix.reset(vegaMatrix);

    this->initializeEigenMatrixFromVegaMatrix(*vegaMatrix, m_M);

    // TODO: Add option to save mass matrix to file
}

void
DeformableBodyModel::initializeDampingMatrix(std::shared_ptr<vega::VolumetricMesh> vegaMesh)
{
    auto dampingLaplacianCoefficient = this->m_forceModelConfiguration->getFloatsOptionsMap().at("dampingLaplacianCoefficient");
    auto dampingMassCoefficient = this->m_forceModelConfiguration->getFloatsOptionsMap().at("dampingMassCoefficient");

    m_damped = (dampingLaplacianCoefficient == 0.0 || dampingMassCoefficient == 0.0) ? false : true;

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

    auto meshGraph = std::make_shared<vega::Graph>(*vega::GenerateMeshGraph::Generate(vegaMesh.get()));

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

    this->m_vegaDampingMatrix.reset(matrix);

    this->initializeEigenMatrixFromVegaMatrix(*matrix, m_C);
}

void
DeformableBodyModel::initializeTangentStiffness()
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

    if (!this->m_vegaMassMatrix)
    {
        LOG(WARNING) << "DeformableBodyModel::initializeTangentStiffness - Vega mass matrix doesn't exist!";
        return;
    }

    matrix->BuildSubMatrixIndices(*this->m_vegaMassMatrix.get());

    if (this->m_vegaDampingMatrix)
    {
        matrix->BuildSubMatrixIndices(*this->m_vegaDampingMatrix.get(), 1);
    }

    this->m_vegaTangentStiffnessMatrix.reset(matrix);

    this->initializeEigenMatrixFromVegaMatrix(*matrix, m_K);

    if (m_damped)
    {
        const auto &dampingStiffnessCoefficient = m_forceModelConfiguration->getFloatsOptionsMap().at("dampingStiffnessCoefficient");
        const auto &dampingMassCoefficient = m_forceModelConfiguration->getFloatsOptionsMap().at("dampingMassCoefficient");

        // Initialize the Raleigh damping matrix
        m_C = dampingMassCoefficient*m_M + dampingStiffnessCoefficient*m_K;
    }

    //?
    this->m_internalForceModel->setTangentStiffness(m_vegaTangentStiffnessMatrix);
}

void
DeformableBodyModel::initializeGravityForce()
{
    m_gravityForce.resize(m_numDOF);
    m_gravityForce.setZero();
    const float gravity = m_forceModelConfiguration->getFloatsOptionsMap().at("gravity");

    m_vegaPhysicsMesh->computeGravity(m_gravityForce.data(), gravity);
}

void
DeformableBodyModel::computeImplicitSystemRHS(kinematicState& stateAtT,
                                              kinematicState& newState)
{
    // Do checks if there are uninitialized matrices

    /*auto &M = this->evalMass(newState);
    auto &K = this->evalDFx(newState);
    auto &f = this->evalRHS(newState);*/

    m_internalForceModel->getTangentStiffnessMatrix(newState.getQ(), m_K);

    auto uPrev = stateAtT.getQ();
    auto vPrev = stateAtT.getQDot();
    auto u = newState.getQ();
    auto v = newState.getQDot();

    const double dT = m_timeIntegrator->getTimestepSize();

    //m_Feff = m_M * (v - vPrev) / dT;
    //m_Feff -= m_K * (u - uPrev - vPrev * dT);

    m_Feff = m_K * (vPrev * -dT * dT);

    if (m_damped)
    {
        m_Feff -= dT*m_C*newState.getQDot();
    }

    m_internalForceModel->getInternalForce(m_Finternal, newState.getQ());
    m_Feff -= m_Finternal;
    m_Feff += m_explicitExternalForce;
    m_Feff += m_gravityForce;

    //state.applyBoundaryConditions(this->rhs);
}

void
DeformableBodyModel::computeSemiImplicitSystemRHS(kinematicState& stateAtT,
kinematicState& newState)
{
    // Do checks if there are uninitialized matrices
    m_internalForceModel->getTangentStiffnessMatrix(newState.getQ(), m_K);

    auto uPrev = stateAtT.getQ();
    auto vPrev = stateAtT.getQDot();
    auto u = newState.getQ();
    auto v = newState.getQDot();

    const double dT = m_timeIntegrator->getTimestepSize();

    m_Feff = m_K * (vPrev * -dT * dT);

    if (m_damped)
    {
        m_Feff -= dT*m_C*newState.getQDot();
    }

    m_internalForceModel->getInternalForce(m_Finternal, newState.getQ());
    m_Feff -= m_Finternal;
    m_Feff += m_explicitExternalForce;
    m_Feff += m_gravityForce;
}

void
DeformableBodyModel::computeImplicitSystemLHS(const kinematicState& stateAtT,
                                              kinematicState& newState)
{
    // Do checks if there are uninitialized matrices


    /*auto &M = this->evalMass(newState);
    auto &K = this->evalDFx(newState);
    auto &C = this->evalDFv(newState);*/

    updateMassMatrix();
    m_internalForceModel->getTangentStiffnessMatrix(newState.getQ(), m_K);
    updateDampingMatrix();

    const double dT = m_timeIntegrator->getTimestepSize();

    m_Keff = m_M;
    if (m_damped)
    {
        m_Keff += dT*m_C;
    }
    m_Keff += (dT*dT) * m_K;

    //previousState.applyBoundaryConditions(this->systemMatrix);
}

void
DeformableBodyModel::initializeExplicitExternalForces()
{
    m_explicitExternalForce.resize(m_numDOF);
    m_explicitExternalForce.setZero();

    // Not supported for now
}

void
DeformableBodyModel::updateDampingMatrix()
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
DeformableBodyModel::applyBoundaryConditions(SparseMatrixd &M, const bool withCompliance) const
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
DeformableBodyModel::applyBoundaryConditions(Vectord &x) const
{
    for (auto & index : m_fixedNodeIds)
    {
        x(index) = 0.0;
    }
}

void
DeformableBodyModel::updateMassMatrix()
{
    // Do nothing for now as topology changes are not supported yet!
}

void
DeformableBodyModel::updatePhysicsGeometry()
{
    auto volMesh = std::static_pointer_cast<VolumetricMesh>(m_forceModelGeometry);
    auto u = m_currentState->getQ();
    volMesh->setVerticesDisplacements(u);
}


void
DeformableBodyModel::updateBodyStates(const Vectord& solution, const stateUpdateType updateType)
{
    auto uPrev = m_previousState->getQ();
    auto u = m_currentState->getQ();
    auto v = m_currentState->getQDot();

    m_tmpStorage = v;

    switch(updateType)
    {
    case stateUpdateType::deltaVelocity:
        m_currentState->setV(v + solution);
        m_previousState->setV(m_tmpStorage);

        m_tmpStorage = u;
        m_currentState->setU(uPrev + m_timeIntegrator->getTimestepSize()*v);
        m_previousState->setU(m_tmpStorage);
        break;

    case stateUpdateType::velocity:
        m_currentState->setV(solution);
        m_previousState->setV(m_tmpStorage);

        m_tmpStorage = u;
        m_currentState->setU(uPrev + m_timeIntegrator->getTimestepSize()*v);
        m_previousState->setU(m_tmpStorage);
        break;

    default:
        LOG(WARNING) << "DeformableBodyModel::updateBodyStates: Unknown state update type";
    }
    this->m_qSol = m_currentState->getQ();
}

NonLinearSystem::VectorFunctionType
DeformableBodyModel::getFunction(const bool semiImplicit)
{
    // Function to evaluate the nonlinear objective function given the current state
    return [&, this](const Vectord& q) -> const Vectord&
    {
        //updateBodyStates(-q, stateUpdateType::deltaVelocity);
        if (semiImplicit)
        {
            computeSemiImplicitSystemRHS(*m_previousState.get(), *m_currentState.get());
        }
        else
        {
            computeImplicitSystemRHS(*m_previousState.get(), *m_currentState.get());
        }
        //applyBoundaryConditions(m_Feff);
        return m_Feff;
    };
}

NonLinearSystem::MatrixFunctionType
DeformableBodyModel::getFunctionGradient()
{
    //const Vectord& q
    // Gradient of the nonlinear objective function given the current state
    return [&, this](const Vectord& q) -> const SparseMatrixd&
    {
        computeImplicitSystemLHS(*m_previousState.get(), *m_currentState.get());
        //applyBoundaryConditions(m_Keff);
        return m_Keff;
    };
}

NonLinearSystem::UpdateFunctionType
DeformableBodyModel::getUpdateFunction()
{
    // Function to evaluate the nonlinear objective function given the current state
    return[&, this](const Vectord& q) -> void
    {
        updateBodyStates(q, stateUpdateType::deltaVelocity);
    };
}

void
DeformableBodyModel::initializeEigenMatrixFromVegaMatrix(const vega::SparseMatrix& vegaMatrix, SparseMatrixd& eigenMatrix)
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
DeformableBodyModel::getContactForce()
{
    return m_Fcontact;
}

} // imstk