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
#include "generateMassMatrix.h"

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

bool
DeformableBodyModel::loadBoundaryConditions()
{
    auto fileName = m_forceModelConfiguration->getStringOptionsMap().at("fixedDOFFilename");

    if (fileName.empty())
    {
        LOG(WARNING) << "DeformableBodyModel::loadBoundaryConditions: The external boundary conditions file name is empty";
        return false;
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
            return false;
        }

        file.close();
        return true;
    }
}

void
DeformableBodyModel::initializeForceModel()
{
    auto imstkVolMesh = std::static_pointer_cast<VolumetricMesh>(m_forceModelGeometry);
    std::shared_ptr<vega::VolumetricMesh> vegaMesh = VegaMeshReader::getVegaVolumeMeshFromVolumeMesh(imstkVolMesh);

    switch (m_forceModelConfiguration->getForceModelType())
    {
    case ForceModelType::StVK:

        this->m_internalForceModel = std::make_shared<StVKForceModel>(vegaMesh);
        break;

    case ForceModelType::Linear:

        this->m_internalForceModel = std::make_shared<LinearFEMForceModel>(vegaMesh);
        break;

    case ForceModelType::Corotational:

        this->m_internalForceModel = std::make_shared<CorotationalFEMForceModel>(vegaMesh);
        break;

    case ForceModelType::Invertible:

        this->m_internalForceModel = std::make_shared<IsotropicHyperelasticFEForceModel>(vegaMesh, -MAX_D);
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
        LOG(INFO) << "DeformableBodyModel::initializeMassMatrix Force model geometry not set!";
        return;
    }

    vega::SparseMatrix *vegaMatrix;
    auto imstkVolMesh = std::static_pointer_cast<VolumetricMesh>(m_forceModelGeometry);
    std::shared_ptr<vega::VolumetricMesh> vegaMesh = VegaMeshReader::getVegaVolumeMeshFromVolumeMesh(imstkVolMesh);
    vega::GenerateMassMatrix::computeMassMatrix(vegaMesh.get(), &vegaMatrix, true);

    auto rowLengths = vegaMatrix->GetRowLengths();
    auto nonZeroValues = vegaMatrix->GetEntries();
    auto columnIndices = vegaMatrix->GetColumnIndices();

    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(vegaMatrix->GetNumEntries());
    for (int i = 0, end = vegaMatrix->GetNumRows(); i < end; ++i)
    {
        for (int k = 0, k_end = rowLengths[i]; k < k_end; ++k)
        {
            triplets.emplace_back(i, columnIndices[i][k], nonZeroValues[i][k]);
        }
    }
    m_M.resize(vegaMatrix->GetNumRows(), vegaMatrix->GetNumColumns());
    m_M.setFromTriplets(std::begin(triplets), std::end(triplets));
    m_M.makeCompressed();

    /*this->vegaMassMatrix.reset(vegaMatrix);

    if (saveToDisk)
    {
    char name[] = "ComputedMassMatrix.mass";
    this->vegaMassMatrix->Save(name);
    }*/
}

void
DeformableBodyModel::initializeDampingMatrix()
{
    auto dampingLaplacianCoefficient =
        this->m_forceModelConfiguration->getFloatsOptionsMap().at("dampingLaplacianCoefficient");

    auto dampingMassCoefficient =
        this->m_forceModelConfiguration->getFloatsOptionsMap().at("dampingMassCoefficient");

    if (dampingLaplacianCoefficient == 0.0 || dampingMassCoefficient == 0.0)
    {
        m_damped = false;
        return;
    }
    else
    {
        m_damped = true;
    }

    if (dampingLaplacianCoefficient <= 0.0)
    {
        LOG(INFO) << "DeformableBodyModel::initializeDampingMatrix Damping coefficient is negative!";
        return;
    }

    auto imstkVolMesh = std::static_pointer_cast<VolumetricMesh>(m_forceModelGeometry);
    std::shared_ptr<vega::VolumetricMesh> vegaMesh = VegaMeshReader::getVegaVolumeMeshFromVolumeMesh(imstkVolMesh);

    auto meshGraph = std::make_shared<vega::Graph>(*vega::GenerateMeshGraph::Generate(this->m_forceModelGeometry.get()));

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

    auto rowLengths = matrix->GetRowLengths();
    auto nonZeroValues = matrix->GetEntries();
    auto columnIndices = matrix->GetColumnIndices();

    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(matrix->GetNumEntries());
    for (int i = 0, end = matrix->GetNumRows(); i < end; ++i)
    {
        for (int k = 0, k_end = rowLengths[i]; k < k_end; ++k)
        {
            triplets.emplace_back(i, columnIndices[i][k], nonZeroValues[i][k]);
        }
    }

    m_C.resize(matrix->GetNumRows(), matrix->GetNumColumns());
    m_C.setFromTriplets(std::begin(triplets), std::end(triplets));
    m_C.makeCompressed();

    //this->dampingMatrix.reset(matrix);
}

void
DeformableBodyModel::initializeTangentStiffness()
{
    if (!m_internalForceModel)
    {
        LOG(WARNING) << "Tangent stiffness cannot be initialized without force model";
        return;
    }

    vega::SparseMatrix *matrix;
    m_internalForceModel->GetTangentStiffnessMatrixTopology(&matrix);

    if (!matrix)
    {
        // TODO: log this
        return;
    }

    if (!this->vegaMassMatrix)
    {
        // TODO: log this
        return;
    }

    matrix->BuildSubMatrixIndices(*this->vegaMassMatrix.get());

    if (this->dampingMatrix)
    {
        matrix->BuildSubMatrixIndices(*this->dampingMatrix.get(), 1);
    }

    auto rowLengths = matrix->GetRowLengths();
    auto columnIndices = matrix->GetColumnIndices();

    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(matrix->GetNumEntries());
    for (int i = 0, end = matrix->GetNumRows(); i < end; ++i)
    {
        for (int k = 0, k_end = rowLengths[i]; k < k_end; ++k)
        {
            triplets.emplace_back(i, columnIndices[i][k], 0.001);
        }
    }
    m_K.resize(matrix->GetNumRows(),
        matrix->GetNumColumns());
    m_K.setFromTriplets(std::begin(triplets), std::end(triplets));
    m_K.makeCompressed();

    this->vegaTangentStiffnessMatrix.reset(matrix);

    const auto &dampingStiffnessCoefficient =
        m_forceModelConfiguration->getFloatsOptionsMap().at("dampingStiffnessCoefficient");

    const auto &dampingMassCoefficient =
        m_forceModelConfiguration->getFloatsOptionsMap().at("dampingMassCoefficient");

    // Initialize the Raleigh damping matrix
    m_C = dampingMassCoefficient*m_M + m_K*dampingStiffnessCoefficient;
}

void
DeformableBodyModel::initializeGravity()
{
    m_gravityForce.resize(m_numDOF);
    m_gravityForce.setZero();
    double gravity = m_forceModelConfiguration->getFloatsOptionsMap().at("gravity");

    auto vegaMesh = VegaMeshReader::getVegaVolumeMeshFromVolumeMesh(std::static_pointer_cast<VolumetricMesh>(m_forceModelGeometry));
    vegaMesh->computeGravity(m_gravityForce.data(), gravity);
}

void
DeformableBodyModel::computeImplicitSystemRHS(const kinematicState& state,
                                              const kinematicState& newState)
{
    // Do checks if there are uninitialized matrices

    /*auto &M = this->evalMass(newState);
    auto &K = this->evalDFx(newState);
    auto &f = this->evalRHS(newState);*/

    m_internalForceModel->getTangentStiffnessMatrix(newState.getQ(), m_K);

    double timeStep = m_timeIntegrator->getTimestepSize();

    m_Feff = m_M * (newState.getQDot() - state.getQDot()) / timeStep;
    m_Feff -= m_K * (newState.getQ() - state.getQ() - newState.getQDot() * timeStep);
    m_Feff += m_explicitExternalForce;
    m_Feff += m_gravityForce;

    if (m_damped)
    {
        m_Feff -= timeStep*this->Damping(newState)*newState.getQDot();
    }
    //state.applyBoundaryConditions(this->rhs);
}

void
DeformableBodyModel::computeImplicitSystemLHS(const kinematicState& state,
                                              const kinematicState& newState)
{
    // Do checks if there are uninitialized matrices


    /*auto &M = this->evalMass(newState);
    auto &K = this->evalDFx(newState);
    auto &C = this->evalDFv(newState);*/

    m_internalForceModel->getTangentStiffnessMatrix(newState.getQ(), m_K);

    double timeStep = m_timeIntegrator->getTimestepSize();

    m_Keff = (1.0 / timeStep) * m_M;
    if (m_damped)
    {
        const auto &dampingStiffnessCoefficient =
            m_forceModelConfiguration->getFloatsOptionsMap().at("dampingStiffnessCoefficient");

        const auto &dampingMassCoefficient =
            m_forceModelConfiguration->getFloatsOptionsMap().at("dampingMassCoefficient");

        m_C = dampingMassCoefficient*m_M + m_K*dampingStiffnessCoefficient;
        m_Keff += m_C;
    }
    m_Keff += timeStep * m_K;

    //previousState.applyBoundaryConditions(this->systemMatrix);
}

void
DeformableBodyModel::initializeExplicitExternalForces()
{
    m_explicitExternalForce.resize(m_numDOF);
    m_explicitExternalForce.setZero();
}

NonLinearSystem::VectorFunctionType&
DeformableBodyModel::getFunction(const Vectord& q)
{
    // Function to evaluate the nonlinear objective function.
    return [&, this](const Vectord &) -> const Vectord&
    {
        computeImplicitSystemRHS(state, newState);
        return m_Feff;
    };
}

NonLinearSystem::MatrixFunctionType&
DeformableBodyModel::getFunctionGradient(const Vectord& q)
{
    // Gradient of the nonlinear objective function.
    return [&, this](const Vectord &) -> const SparseMatrixd&
    {
        computeImplicitSystemLHS(state, newState);
        return m_Keff;
    };
}

}
