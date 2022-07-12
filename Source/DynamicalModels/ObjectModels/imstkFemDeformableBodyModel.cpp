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

#include "imstkFemDeformableBodyModel.h"
#include "imstkConjugateGradient.h"
#include "imstkCorotationalFemForceModel.h"
#include "imstkIsotropicHyperelasticFeForceModel.h"
#include "imstkLinearFemForceModel.h"
#include "imstkLogger.h"
#include "imstkNewtonSolver.h"
#include "imstkPointSet.h"
#include "imstkTaskGraph.h"
#include "imstkTimeIntegrator.h"
#include "imstkTypes.h"
#include "imstkVecDataArray.h"
#include "imstkVegaMeshIO.h"
#include "imstkVolumetricMesh.h"

DISABLE_WARNING_PUSH
    DISABLE_WARNING_HIDES_CLASS_MEMBER
#include "imstkStVKForceModel.h"

#include <generateMassMatrix.h>
#include <generateMeshGraph.h>
#include <configFile.h>

DISABLE_WARNING_POP
#include <fstream>

namespace imstk
{
FemDeformableBodyModel::FemDeformableBodyModel() :
    DynamicalModel(DynamicalModelType::ElastoDynamics)
{
    m_fixedNodeIds.reserve(1000);

    m_validGeometryTypes = { "TetrahedralMesh", "HexahedralMesh" };

    m_solveNode = m_taskGraph->addFunction("FEMModel_Solve", [&]() { getSolver()->solve(); });
}

FemDeformableBodyModel::~FemDeformableBodyModel()
{
    // Get vega to destruct first (before the shared pointer to the vega mesh is cleaned up)
    m_internalForceModel = nullptr;
}

void
FemDeformableBodyModel::configure(const std::string& configFileName)
{
    // Configure the FEMModelConfig
    m_FEModelConfig = std::make_shared<FemModelConfig>();

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
    vegaConfigFileOptions.addOptionOptional("gravity", &m_FEModelConfig->m_gravity, m_FEModelConfig->m_gravity);

    // Parse the configuration file
    CHECK(vegaConfigFileOptions.parseOptions(configFileName.data()) == 0)
        << "ForceModelConfig::parseConfig - Unable to load the configuration file";

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
        m_FEModelConfig->m_femMethod = FeMethodType::StVK;
    }
    else if (std::strcmp(femMethod, "CLFEM") == 0)
    {
        m_FEModelConfig->m_femMethod = FeMethodType::Corotational;
    }
    else if (std::strcmp(femMethod, "Linear") == 0)
    {
        m_FEModelConfig->m_femMethod = FeMethodType::Linear;
    }
    else if (std::strcmp(femMethod, "InvertibleFEM") == 0)
    {
        m_FEModelConfig->m_femMethod = FeMethodType::Invertible;
    }
    else
    {
        LOG(WARNING) << "FE method not assigned; will default to StVK";
        m_FEModelConfig->m_femMethod = FeMethodType::StVK;
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
FemDeformableBodyModel::configure(std::shared_ptr<FemModelConfig> config)
{
    m_FEModelConfig = config;
}

bool
FemDeformableBodyModel::initialize()
{
    // prerequisite of for successfully initializing
    CHECK(m_geometry != nullptr && m_FEModelConfig != nullptr) << "DeformableBodyModel::initialize: Physics mesh or force model configuration not set yet!";
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(m_geometry);
    // If there isn't already a displacements array for the geometry
    if (!pointSet->hasVertexAttribute("displacements"))
    {
        pointSet->setVertexAttribute("displacements", std::make_shared<VecDataArray<double, 3>>(pointSet->getNumVertices()));
    }

    // Setup default solver if model doesn't yet have one
    if (m_solver == nullptr)
    {
        // Create a nonlinear system
        // auto nlSystem = std::make_shared<NonLinearSystem<SparseMatrixd>>(getFunction(), getFunctionGradient());
        auto nlSystem = std::make_shared<NonLinearSystem<SparseMatrixd>>(getFunction(), getFunctionGradient(), getFunctionAndGradient());

        nlSystem->setUnknownVector(getUnknownVec());
        nlSystem->setUpdateFunction(getUpdateFunction());
        nlSystem->setUpdatePreviousStatesFunction(getUpdatePrevStateFunction());

        // Create a linear solver
        auto linSolver = std::make_shared<ConjugateGradient>();

        if (linSolver->getType() == imstk::LinearSolver<imstk::SparseMatrixd>::Type::GaussSeidel
            && isFixedBCImplemented())
        {
            LOG(WARNING) << "The GS solver may not be viable!";
        }

        // Create a non-linear solver and add to the scene
        auto nlSolver = std::make_shared<NewtonSolver<SparseMatrixd>>();
        nlSolver->setToSemiImplicit();
        nlSolver->setLinearSolver(linSolver);
        nlSolver->setSystem(nlSystem);
        setSolver(nlSolver);
    }

    auto physicsMesh = std::dynamic_pointer_cast<PointSet>(this->getModelGeometry());
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

    m_Feff.resize(m_numDof);
    m_Finternal.resize(m_numDof);
    m_Finternal.setConstant(0.0);
    m_Fcontact.resize(m_numDof);
    m_Fcontact.setConstant(0.0);
    m_qSol.resize(m_numDof);
    m_qSol.setConstant(0.0);

    return true;
}

void
FemDeformableBodyModel::loadInitialStates()
{
    if (m_numDof == 0)
    {
        LOG(WARNING) << "Number of degree of freedom is zero!";
    }

    // For now the initial states are set to zero
    m_initialState  = std::make_shared<kinematicState>(m_numDof);
    m_previousState = std::make_shared<kinematicState>(m_numDof);
    m_currentState  = std::make_shared<kinematicState>(m_numDof);
}

bool
FemDeformableBodyModel::loadBoundaryConditions()
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
            LOG(INFO) << "The external boundary conditions file is empty";
        }

        if (file.is_open())
        {
            size_t index;
            size_t maxAllowed = m_vegaPhysicsMesh->getNumVertices();
            while (!file.eof())
            {
                file >> index;
                if (index < maxAllowed)
                {
                    m_fixedNodeIds.emplace_back(index);
                }
                else
                {
                    LOG(WARNING) << "The boundary condition node id provided is greater than number of nodes and hence excluded!!";
                    return false;
                }
            }

            file.close();
            std::sort(m_fixedNodeIds.begin(), m_fixedNodeIds.end());  // for efficiency
        }
        else
        {
            LOG(FATAL) << "Could not open boundary conditions file!";
            return false;
        }
    }
    return true;
}

bool
FemDeformableBodyModel::initializeForceModel()
{
    const double g = m_FEModelConfig->m_gravity;
    // Since vega 4.0 doesn't add gravity correcntly in all cases, we do it ourselves; see \ref initializeGravityForce
    // const bool   isGravityPresent = (g > 0) ? true : false;
    const bool isGravityPresent = false;

    m_numDof = (size_t)m_vegaPhysicsMesh->getNumVertices() * 3;

    switch (m_FEModelConfig->m_femMethod)
    {
    case FeMethodType::StVK:

        m_internalForceModel = std::make_shared<StvkForceModel>(m_vegaPhysicsMesh, isGravityPresent, g);
        break;

    case FeMethodType::Linear:

        m_internalForceModel = std::make_shared<LinearFemForceModel>(m_vegaPhysicsMesh, isGravityPresent, g);
        break;

    case FeMethodType::Corotational:

        m_internalForceModel = std::make_shared<CorotationalFemForceModel>(m_vegaPhysicsMesh);
        break;

    case FeMethodType::Invertible:

        m_internalForceModel = std::make_shared<IsotropicHyperelasticFeForceModel>(
                                        m_FEModelConfig->m_hyperElasticMaterialType,
                                        m_vegaPhysicsMesh,
                                        -MAX_D,
                                        isGravityPresent,
                                        g);
        break;

    default:
        LOG(FATAL) << "Unknown force model type";
        return false;
    }   //switch

    return true;
}

bool
FemDeformableBodyModel::initializeMassMatrix()
{
    CHECK(m_geometry != nullptr) << "Force model geometry not set!";

    vega::SparseMatrix* vegaMatrix;
    vega::GenerateMassMatrix::computeMassMatrix(m_vegaPhysicsMesh.get(), &vegaMatrix, true);    //caveat

    m_vegaMassMatrix.reset(vegaMatrix);

    this->initializeEigenMatrixFromVegaMatrix(*vegaMatrix, m_M);

    /// \todo Add option to save mass matrix to file

    return true;
}

bool
FemDeformableBodyModel::initializeDampingMatrix()
{
    auto dampingLaplacianCoefficient = m_FEModelConfig->m_dampingLaplacianCoefficient;
    auto dampingMassCoefficient      = m_FEModelConfig->m_dampingMassCoefficient;
    auto dampingStiffnessCoefficient = m_FEModelConfig->m_dampingStiffnessCoefficient;

    if (dampingStiffnessCoefficient == 0.0 && dampingLaplacianCoefficient == 0.0 && dampingMassCoefficient == 0.0)
    {
        LOG(WARNING) << "All the damping parameters are zero!";
        return true;
    }

    if (dampingLaplacianCoefficient < 0.0)
    {
        LOG(WARNING) << "Damping coefficient is negative!";
        return false;
    }

    //auto imstkVolMesh = std::static_pointer_cast<VolumetricMesh>(m_geometry);
    //std::shared_ptr<vega::VolumetricMesh> vegaMesh = VegaMeshReader::getVegaVolumeMeshFromVolumeMesh(imstkVolMesh);

    auto meshGraph = std::make_shared<vega::Graph>(*vega::GenerateMeshGraph::Generate(m_vegaPhysicsMesh.get()));

    if (!meshGraph)
    {
        LOG(WARNING) << "Mesh graph not avaliable!";
        return false;
    }

    vega::SparseMatrix* matrix;
    meshGraph->GetLaplacian(&matrix, 1);

    if (!matrix)
    {
        LOG(WARNING) << "Mesh Laplacian not avaliable!";
        return false;
    }

    matrix->ScalarMultiply(dampingLaplacianCoefficient);

    m_vegaDampingMatrix.reset(matrix);

    this->initializeEigenMatrixFromVegaMatrix(*matrix, m_C);

    m_damped = true;

    return true;
}

bool
FemDeformableBodyModel::initializeTangentStiffness()
{
    CHECK(m_internalForceModel != nullptr)
        << "Tangent stiffness cannot be initialized without force model";

    vega::SparseMatrix* matrix = nullptr;
    m_internalForceModel->getTangentStiffnessMatrixTopology(&matrix);

    CHECK(matrix != nullptr) << "Tangent stiffness matrix topology not avaliable!";
    CHECK(m_vegaMassMatrix != nullptr) << "Vega mass matrix doesn't exist!";

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
FemDeformableBodyModel::initializeGravityForce()
{
    m_Fgravity.resize(m_numDof);
    m_Fgravity.setZero();
    const double gravity = m_FEModelConfig->m_gravity;

    m_vegaPhysicsMesh->computeGravity(m_Fgravity.data(), gravity);

    return true;
}

void
FemDeformableBodyModel::computeImplicitSystemRHS(kinematicState&       stateAtT,
                                                 kinematicState&       newState,
                                                 const StateUpdateType updateType)
{
    const auto& uPrev = stateAtT.getQ();
    const auto& vPrev = stateAtT.getQDot();
    auto&       u     = newState.getQ();
    const auto& v     = newState.getQDot();

    // Do checks if there are uninitialized matrices
    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case StateUpdateType::DeltaVelocity:

        m_internalForceModel->getTangentStiffnessMatrix(u, m_K);
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
        LOG(WARNING) << "Update type not supported";
    }
}

void
FemDeformableBodyModel::computeSemiImplicitSystemRHS(kinematicState&       stateAtT,
                                                     kinematicState&       newState,
                                                     const StateUpdateType updateType)
{
    //auto& uPrev = stateAtT.getQ();
    const auto& vPrev = stateAtT.getQDot();
    auto&       u     = newState.getQ();
    //auto& v     = newState.getQDot();

    // Do checks if there are uninitialized matrices
    m_internalForceModel->getTangentStiffnessMatrix(u, m_K);
    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case StateUpdateType::DeltaVelocity:

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
        LOG(FATAL) << "Update type not supported";
    }
}

void
FemDeformableBodyModel::computeImplicitSystemLHS(const kinematicState& imstkNotUsed(stateAtT),
                                                 kinematicState&       newState,
                                                 const StateUpdateType updateType)
{
    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case StateUpdateType::DeltaVelocity:
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
        LOG(FATAL) << "Update type not supported";
    }
}

void
FemDeformableBodyModel::computeSemiImplicitSystemRHSAndLHS(kinematicState&       stateAtT,
                                                           kinematicState&       newState,
                                                           const StateUpdateType updateType)
{
    const auto&  vPrev = stateAtT.getQDot();
    const double dT    = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case StateUpdateType::DeltaVelocity:
        // LHS
        this->updateMassMatrix();
        m_internalForceModel->getForceAndMatrix(newState.getQ(), m_Finternal, m_K);
        this->updateDampingMatrix();

        m_Keff = m_M;
        if (m_damped)
        {
            m_Keff += dT * m_C;
        }
        m_Keff += (dT * dT) * m_K;

        // RHS
        m_Feff = m_K * (vPrev * -dT);

        if (m_damped)
        {
            m_Feff -= m_C * vPrev;
        }

        m_Feff -= m_Finternal;
        m_Feff += m_FexplicitExternal;
        m_Feff += m_Fgravity;
        m_Feff += m_Fcontact;
        m_Feff *= dT;

        break;

    default:
        LOG(FATAL) << "Update type not supported";
    }
}

void
FemDeformableBodyModel::computeImplicitSystemRHSAndLHS(kinematicState&       stateAtT,
                                                       kinematicState&       newState,
                                                       const StateUpdateType updateType)
{
    const auto&  uPrev = stateAtT.getQ();
    const auto&  vPrev = stateAtT.getQDot();
    auto&        u     = newState.getQ();
    const auto&  v     = newState.getQDot();
    const double dT    = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case StateUpdateType::DeltaVelocity:
        // LHS
        this->updateMassMatrix();
        m_internalForceModel->getForceAndMatrix(u, m_Finternal, m_K);
        this->updateDampingMatrix();

        m_Keff = m_M;
        if (m_damped)
        {
            m_Keff += dT * m_C;
        }
        m_Keff += (dT * dT) * m_K;

        // RHS
        m_Feff = m_K * -(uPrev - u + v * dT);

        if (m_damped)
        {
            m_Feff -= m_C * v;
        }

        m_Feff -= m_Finternal;
        m_Feff += m_FexplicitExternal;
        m_Feff += m_Fgravity;
        m_Feff += m_Fcontact;
        m_Feff *= dT;
        m_Feff += m_M * (vPrev - v);
        break;

    default:
        LOG(FATAL) << "Update type not supported";
    }
}

bool
FemDeformableBodyModel::initializeExplicitExternalForces()
{
    m_FexplicitExternal.resize(m_numDof);
    m_FexplicitExternal.setZero();

    return true;
}

void
FemDeformableBodyModel::updateDampingMatrix()
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
FemDeformableBodyModel::applyBoundaryConditions(SparseMatrixd& M, const bool withCompliance) const
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
FemDeformableBodyModel::applyBoundaryConditions(Vectord& x) const
{
    for (auto& index : m_fixedNodeIds)
    {
        const auto _3Index = 3 * index;
        x(_3Index) = x(_3Index + 1) = x(_3Index + 2) = 0.0;
    }
}

void
FemDeformableBodyModel::updateMassMatrix()
{
    // Do nothing for now as topology changes are not supported yet!
}

void
FemDeformableBodyModel::updatePhysicsGeometry()
{
    auto                                     volMesh = std::dynamic_pointer_cast<PointSet>(m_geometry);
    auto&                                    u       = m_currentState->getQ();
    std::shared_ptr<VecDataArray<double, 3>> displacementsPtr =
        std::dynamic_pointer_cast<VecDataArray<double, 3>>(volMesh->getVertexAttribute("displacements"));
    std::copy_n(u.data(), displacementsPtr->size() * 3, reinterpret_cast<double*>(displacementsPtr->getVoidPointer()));

    const VecDataArray<double, 3>& initPositions = *volMesh->getVertexPositions(Geometry::DataType::PreTransform);
    VecDataArray<double, 3>&       positions     = *volMesh->getVertexPositions();
    const VecDataArray<double, 3>& displacements = *displacementsPtr;
    for (int i = 0; i < displacementsPtr->size(); i++)
    {
        positions[i] = initPositions[i] + displacements[i];
    }
}

void
FemDeformableBodyModel::updateBodyPreviousStates()
{
    m_previousState->setU(m_currentState->getQ());
    m_previousState->setV(m_currentState->getQDot());
}

void
FemDeformableBodyModel::updateBodyStates(const Vectord& solution, const StateUpdateType updateType)
{
    this->updateBodyPreviousStates();
    this->updateBodyIntermediateStates(solution, updateType);
}

void
FemDeformableBodyModel::updateBodyIntermediateStates(
    const Vectord&        solution,
    const StateUpdateType updateType)
{
    const auto& uPrev = m_previousState->getQ();
    //auto&        u     = m_currentState->getQ();
    const auto&  v  = m_currentState->getQDot();
    const double dT = m_timeIntegrator->getTimestepSize();

    switch (updateType)
    {
    case StateUpdateType::DeltaVelocity:
        m_currentState->setV(v + solution);
        m_currentState->setU(uPrev + dT * v);

        break;

    case StateUpdateType::Velocity:
        m_currentState->setV(solution);
        m_currentState->setU(uPrev + dT * v);

        break;

    default:
        LOG(FATAL) << "Unknown state update type";
    }
    m_qSol = m_currentState->getQ();
}

NonLinearSystem<SparseMatrixd>::VectorFunctionType
FemDeformableBodyModel::getFunction()
{
#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif

    // Function to evaluate the nonlinear objective function given the current state
    return [&, this](const Vectord& q, const bool semiImplicit) -> const Vectord&
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
#ifdef WIN32
#pragma warning( pop )
#endif
}

NonLinearSystem<SparseMatrixd>::MatrixFunctionType
FemDeformableBodyModel::getFunctionGradient()
{
#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif
    // Gradient of the nonlinear objective function given the current state
    return [&, this](const Vectord& q) -> const SparseMatrixd&
           {
               this->computeImplicitSystemLHS(*m_previousState.get(), *m_currentState.get(), m_updateType);

               if (this->m_implementFixedBC)
               {
                   applyBoundaryConditions(m_Keff);
               }
               return m_Keff;
           };
#ifdef WIN32
#pragma warning( pop )
#endif
}

NonLinearSystem<SparseMatrixd>::VectorMatrixFunctionType
FemDeformableBodyModel::getFunctionAndGradient()
{
#ifdef WIN32
#pragma warning( push )
#pragma warning( disable : 4100 )
#endif

    // Function to evaluate the nonlinear objective function given the current state
    // return [&, this](const Vectord& q, const bool semiImplicit) -> NonLinearSolver<SparseMatrixd>::VecMatPair
    return [&, this](const Vectord& q, const bool semiImplicit)
           {
               (semiImplicit) ?
               this->computeSemiImplicitSystemRHSAndLHS(*m_previousState.get(), *m_currentState.get(), m_updateType) :
               this->computeImplicitSystemRHSAndLHS(*m_previousState.get(), *m_currentState.get(), m_updateType);
               if (this->m_implementFixedBC)
               {
                   applyBoundaryConditions(m_Feff);
                   applyBoundaryConditions(m_Keff);
               }
               return std::make_pair(&m_Feff, &m_Keff);
           };
#ifdef WIN32
#pragma warning( pop )
#endif
}

NonLinearSystem<SparseMatrixd>::UpdateFunctionType
FemDeformableBodyModel::getUpdateFunction()
{
    // Function to evaluate the nonlinear objective function given the current state
    return [&, this](const Vectord& q, const bool fullyImplicit) -> void
           {
               (fullyImplicit) ?
               this->updateBodyIntermediateStates(q, m_updateType) :
               this->updateBodyStates(q, m_updateType);
           };
}

NonLinearSystem<SparseMatrixd>::UpdatePrevStateFunctionType
FemDeformableBodyModel::getUpdatePrevStateFunction()
{
    // Function to evaluate the nonlinear objective function given the current state
    return [&, this]() -> void
           {
               this->updateBodyPreviousStates();
           };
}

void
FemDeformableBodyModel::initializeEigenMatrixFromVegaMatrix(const vega::SparseMatrix& vegaMatrix,
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

void
FemDeformableBodyModel::setFixedSizeTimeStepping()
{
    m_timeStepSizeType = TimeSteppingType::Fixed;
    m_timeIntegrator->setTimestepSizeToDefault();
}

void
FemDeformableBodyModel::setTimeStep(const double timeStep)
{
    m_timeIntegrator->setTimestepSize(timeStep);
}

double
FemDeformableBodyModel::getTimeStep() const
{
    return m_timeIntegrator->getTimestepSize();
};

void
FemDeformableBodyModel::initGraphEdges(std::shared_ptr<TaskNode> source, std::shared_ptr<TaskNode> sink)
{
    // Setup graph connectivity
    m_taskGraph->addEdge(source, m_solveNode);
    m_taskGraph->addEdge(m_solveNode, sink);
}
} // namespace imstk