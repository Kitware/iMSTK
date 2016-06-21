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

#include "imstkDeformableBodyModel.h"

// vega
#include "generateMassMatrix.h"
#include "generateMeshGraph.h"

namespace imstk
{

DeformableBodyModel::DeformableBodyModel() : DynamicalModel(DynamicalModel::Type::elastoDynamics), m_damped(false){}

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
DeformableBodyModel::initialize()
{
    // prerequisite of for successfully initializing
    if (!m_forceModelGeometry || !m_forceModelConfiguration)
    {
        LOG(WARNING) << "DeformableBodyModel::initialize: Physics mesh or force model configuration not set yet!";
        return;
    }

    m_vegaPhysicsMesh = VegaMeshReader::getVegaVolumeMeshFromVolumeMesh(std::static_pointer_cast<VolumetricMesh>(m_forceModelGeometry));

    initializeForceModel();//c
    initializeMassMatrix();//c
    initializeDampingMatrix();//c
    initializeTangentStiffness();//c
    loadInitialStates();
    loadBoundaryConditions();
    initializeGravityForce();
    initializeExplicitExternalForces();

    m_Feff.resize(m_numDOF);
    m_Finternal.resize(m_numDOF);
}

void
DeformableBodyModel::loadInitialStates()
{
    // Initialize the states
    m_initialState->initialize(m_numDOF);
    m_previousState->initialize(m_numDOF);
    m_currentState->initialize(m_numDOF);
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
    const float g = m_forceModelConfiguration->getFloatsOptionsMap.at("gravity");
    const bool isGravityPresent = (g > 0) ? true : false;

    switch (m_forceModelConfiguration->getForceModelType())
    {
    case ForceModelType::StVK:

        this->m_internalForceModel = std::make_shared<StVKForceModel>(m_vegaPhysicsMesh, isGravityPresent, g);
        break;

    case ForceModelType::Linear:

        this->m_internalForceModel = std::make_shared<LinearFEMForceModel>(m_vegaPhysicsMesh, isGravityPresent, g);
        break;

    case ForceModelType::Corotational:

        this->m_internalForceModel = std::make_shared<CorotationalFEMForceModel>(m_vegaPhysicsMesh, isGravityPresent, g);
        break;

    case ForceModelType::Invertible:

        this->m_internalForceModel = std::make_shared<IsotropicHyperelasticFEForceModel>(m_vegaPhysicsMesh, -MAX_D, isGravityPresent, g);
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

    /*
    if (saveToDisk)
    {
    char name[] = "ComputedMassMatrix.mass";
    this->vegaMassMatrix->Save(name);
    }
    */
}

void
DeformableBodyModel::initializeDampingMatrix()
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
    std::shared_ptr<vega::VolumetricMesh> vegaMesh = VegaMeshReader::getVegaVolumeMeshFromVolumeMesh(imstkVolMesh);

    auto meshGraph = std::make_shared<vega::Graph>(*vega::GenerateMeshGraph::Generate(vegaMesh.get()));

    if (!meshGraph)
    {
        // TODO: log this
        return;
    }

    vega::SparseMatrix *matrix;
    meshGraph->GetLaplacian(&matrix, 1);

    if (!matrix)
    {
        // TODO: log this
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
        // TODO: log this
        return;
    }

    if (!this->m_vegaMassMatrix)
    {
        // TODO: log this
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
DeformableBodyModel::computeImplicitSystemRHS(const kinematicState& stateAtT,
                                              const kinematicState& newState)
{
    // Do checks if there are uninitialized matrices

    /*auto &M = this->evalMass(newState);
    auto &K = this->evalDFx(newState);
    auto &f = this->evalRHS(newState);*/

    m_internalForceModel->getTangentStiffnessMatrix(newState.getQ(), m_K);

    const double dT = m_timeIntegrator->getTimestepSize();

    m_Feff = m_M * (newState.getQDot() - stateAtT.getQDot()) / dT;
    m_Feff -= m_K * (newState.getQ() - stateAtT.getQ() - stateAtT.getQDot() * dT);

    if (m_damped)
    {
        m_Feff -= dT*m_C*newState.getQDot();
    }

    m_internalForceModel->getInternalForce(m_Finternal, newState.getQ());
    m_Feff += m_Finternal;

    m_Feff += m_explicitExternalForce;
    m_Feff += m_gravityForce;

    //state.applyBoundaryConditions(this->rhs);
}

void
DeformableBodyModel::computeImplicitSystemLHS(const kinematicState& stateAtT,
                                              const kinematicState& newState)
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
DeformableBodyModel::updateMassMatrix()
{
    // Do nothing for now as topology changes are not supported yet!
}

void
DeformableBodyModel::updatePhysicsGeometry(const kinematicState& state)
{
    auto volMesh = std::static_pointer_cast<VolumetricMesh>(m_forceModelGeometry);
    volMesh->setVerticesDisplacements(m_currentState->getQ());
}

void
DeformableBodyModel::updateBodyStates(const Vectord& delataV)
{
    auto uPrev = m_previousState->getQ();
    auto u = m_currentState->getQ();
    auto v = m_currentState->getQDot();

    v += delataV;
    u = uPrev + m_timeIntegrator->getTimestepSize()*v;
}

NonLinearSystem::VectorFunctionType&
DeformableBodyModel::getFunction(const Vectord& q)
{
    // Function to evaluate the nonlinear objective function given the current state
    return [&, this](const Vectord&) -> const Vectord&
    {
        computeImplicitSystemRHS(state, newState);
        return m_Feff;
    };
}

NonLinearSystem::MatrixFunctionType&
DeformableBodyModel::getFunctionGradient(const Vectord& q)
{
    // Gradient of the nonlinear objective function given the current state
    return [&, this](const Vectord&) -> const SparseMatrixd&
    {
        computeImplicitSystemLHS(state, newState);
        return m_Keff;
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

}
