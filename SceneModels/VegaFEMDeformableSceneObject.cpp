// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "VegaFEMDeformableSceneObject.h"

// VegaFEM includes
#include "Mesh/VegaVolumetricMesh.h"
#include "Core/MakeUnique.h"
#include "IO/IOMesh.h"

// Vega includes
#include "configFile.h"
#include "generateMassMatrix.h"
#include "sparseMatrix.h"
#include "forceModel.h"
#include "StVKStiffnessMatrix.h"
#include "StVKForceModel.h"
#include "StVKElementABCD.h"
#include "StVKElementABCDLoader.h"
#include "StVKInternalForcesMT.h"
#include "StVKStiffnessMatrixMT.h"
#include "linearFEMForceModel.h"
#include "corotationalLinearFEM.h"
#include "corotationalLinearFEMMT.h"
#include "corotationalLinearFEMForceModel.h"
#include "isotropicHyperelasticFEM.h"
#include "isotropicHyperelasticFEMMT.h"
#include "isotropicHyperelasticFEMForceModel.h"
#include "StVKIsotropicMaterial.h"
#include "neoHookeanIsotropicMaterial.h"
#include "MooneyRivlinIsotropicMaterial.h"
#include "graph.h"

///
/// \brief Vega configuration parser interface. This class interfaces with
///  Vega's configuration parser and define relevant configurations for our
///  solvers.
///
///     Option Name                                 Description
/// ------------------------------- ------------------------------------------------------
///     femMethod                    FEM method used
///                                     [StVK (default), Corotational, Linear, Invertible, none]
///     invertibleMaterial           Constitutive law used
///                                     [StVK (default), NeoHookean, MooneyRivlin, none]
///     fixedDOFFilename             List of fixed degrees of freedom
///                                     [path to file containing indices]
///     dampingMassCoefficient       Mass matrix scaling factor for damping matrix computation
///                                     [default = 0.1; C = dampingMassCoefficient*M + dampingStiffnessCoefficient*K]
///     dampingStiffnessCoefficient  Stiffness matrix factor for damping matrix computation
///                                     [default = 0.01; C = dampingMassCoefficient*M + dampingStiffnessCoefficient*K]
///     dampingLaplacianCoefficient  Laplacian damping matrix factor.
///                                     [default = 0.0]
///     deformationCompliance        Compliance factor.
///                                     [default = 1.0]
///     gravity                      Gravity constant.
///                                     [default = -9.81]
///     compressionResistance       Compression resistance parameter for the invertible methods
///                                     [default = 500.0]
///     inversionThreshold          Inversion threshold parameter for the invertible methods
///                                     [default = -infinity]
///     numberOfThreads             Number of threads spawned by the force model
///                                     [default = 0]
///
class VegaFEMDeformableSceneObject::VegaConfiguration
{
public:
    enum class MethodType
    {
        StVK,
        Corotational,
        Linear,
        Invertible,
        none
    };

    enum class InvertibleMaterialType
    {
        StVK,
        NeoHookean,
        MooneyRivlin,
        none
    };

public:

    ///
    /// \brief Constructor
    ///
    VegaConfiguration(const std::string &configurationFile, bool verbose = false);

    std::string vegaConfigFile; ///> Store configuration file.

    std::map<std::string, double> floatsOptionMap;  ///> Map for floating point configuration variables.
    std::map<std::string, int> intsOptionMap;       ///> Map for int configuration variables.
    std::map<std::string, std::string> stringsOptionMap;///> Map for string configuration variables.

    MethodType forceModelType; ///> Force model type used.
    InvertibleMaterialType isotropicMaterialType; ///> Constitutive law for nonlinear materials.
};

//---------------------------------------------------------------------------
VegaFEMDeformableSceneObject::
VegaConfiguration::VegaConfiguration(const std::string &configurationFile, bool verbose)
{
    if(configurationFile.empty())
    {
        // TODO: Log this.
        std::cout << "Empty configuration filename." << std::endl;
    }

    this->vegaConfigFile = configurationFile;

    ConfigFile vegaConfigFileOptions;

    char femMethod[256];
    vegaConfigFileOptions.addOptionOptional("femMethod",
                                            femMethod,
                                            "StVK");

    char invertibleMaterial[256];
    vegaConfigFileOptions.addOptionOptional("invertibleMaterial",
                                            invertibleMaterial,
                                            "StVK");

    char fixedDOFFilename[256];
    vegaConfigFileOptions.addOptionOptional("fixedDOFFilename",
                                            fixedDOFFilename,
                                            "");

    double dampingMassCoefficient = 0.1;
    vegaConfigFileOptions.addOptionOptional("dampingMassCoefficient",
                                            &dampingMassCoefficient,
                                            dampingMassCoefficient);

    double dampingStiffnessCoefficient = 0.01;
    vegaConfigFileOptions.addOptionOptional("dampingStiffnessCoefficient",
                                            &dampingStiffnessCoefficient,
                                            dampingStiffnessCoefficient);

    double dampingLaplacianCoefficient = 0.0;
    vegaConfigFileOptions.addOptionOptional("dampingLaplacianCoefficient",
                                            &dampingLaplacianCoefficient,
                                            dampingLaplacianCoefficient);

    double deformationCompliance = 1.0;
    vegaConfigFileOptions.addOptionOptional("deformationCompliance",
                                            &deformationCompliance,
                                            deformationCompliance);

    double gravity = -9.81;
    vegaConfigFileOptions.addOptionOptional("gravity",
                                            &gravity,
                                            gravity);

    double compressionResistance = 500.0;
    vegaConfigFileOptions.addOptionOptional("compressionResistance",
                                            &compressionResistance,
                                            compressionResistance);

    double inversionThreshold = -std::numeric_limits< double >::max();
    vegaConfigFileOptions.addOptionOptional("inversionThreshold",
                                            &inversionThreshold,
                                            inversionThreshold);

    int numberOfThreads = 0;
    vegaConfigFileOptions.addOptionOptional("numberOfThreads",
                                            &numberOfThreads,
                                            numberOfThreads);

    // Parse the configuration file
    if(!configurationFile.empty() &&
        vegaConfigFileOptions.parseOptions(configurationFile.data()) != 0)
    {
        /// TODO: Log this.
    }

    // Print option variables
    if(verbose)
    {
        vegaConfigFileOptions.printOptions();
    }

    // Store parsed string values
    this->stringsOptionMap.emplace("femMethod", femMethod);
    this->stringsOptionMap.emplace("invertibleMaterial", invertibleMaterial);
    this->stringsOptionMap.emplace("fixedDOFFilename", fixedDOFFilename);

    // Store parsed floating point values
    this->floatsOptionMap.emplace("dampingMassCoefficient", dampingMassCoefficient);
    this->floatsOptionMap.emplace("dampingLaplacianCoefficient",
                                  dampingLaplacianCoefficient);
    this->floatsOptionMap.emplace("dampingStiffnessCoefficient",
                                  dampingStiffnessCoefficient);
    this->floatsOptionMap.emplace("deformationCompliance", deformationCompliance);
    this->floatsOptionMap.emplace("gravity", gravity);
    this->floatsOptionMap.emplace("compressionResistance", compressionResistance);
    this->floatsOptionMap.emplace("inversionThreshold", inversionThreshold);

    // Store parsed int values
    this->intsOptionMap.emplace("numberOfThreads", numberOfThreads);

    // Set up some variables
    if(this->stringsOptionMap["femMethod"] == "StVK")
    {
        this->forceModelType = MethodType::StVK;
    }
    else if(this->stringsOptionMap["femMethod"] == "Corotational")
    {
        this->forceModelType = MethodType::StVK;
    }
    else if(this->stringsOptionMap["femMethod"] == "Linear")
    {
        this->forceModelType = MethodType::StVK;
    }
    else if(this->stringsOptionMap["femMethod"] == "Invertible")
    {
        this->forceModelType = MethodType::StVK;
    }
    else
    {
        this->forceModelType = MethodType::none;
    }

    if(this->stringsOptionMap["invertibleMaterial"] == "StVK")
    {
        this->isotropicMaterialType = InvertibleMaterialType::StVK;
    }
    else if(this->stringsOptionMap["invertibleMaterial"] == "NeoHookean")
    {
        this->isotropicMaterialType = InvertibleMaterialType::NeoHookean;
    }
    else if(this->stringsOptionMap["invertibleMaterial"] == "MooneyRivlin")
    {
        this->isotropicMaterialType = InvertibleMaterialType::MooneyRivlin;
    }
    else
    {
        this->isotropicMaterialType = InvertibleMaterialType::none;
    }
}

//---------------------------------------------------------------------------
VegaFEMDeformableSceneObject::VegaFEMDeformableSceneObject(const std::string &meshFilename, const std::string &vegaConfigFileName)
{
    this->loadVolumeMesh(meshFilename);
    if(!this->volumetricMesh)
    {
        // TODO: Rise error and log
        return;
    }

    this->configure(vegaConfigFileName);
    this->initialize();
}

//---------------------------------------------------------------------------
VegaFEMDeformableSceneObject::~VegaFEMDeformableSceneObject() {}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::loadVolumeMesh(const std::string &fileName)
{
    auto ioMesh = std::make_shared<IOMesh>();
    ioMesh->read(fileName);

    this->volumetricMesh =
        std::static_pointer_cast<VegaVolumetricMesh>(ioMesh->getMesh());

    if(!this->volumetricMesh)
    {
        // TODO: Print error message
        return;
    }
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::loadInitialStates()
{
    this->initialState = std::make_shared<OdeSystemState>(this->numOfDOF);
    auto &positions = initialState->getPositions();

    auto vertices = this->volumetricMesh->getVegaMesh()->getVertices();
    size_t numVertices = this->volumetricMesh->getVegaMesh()->getNumVertices();

    if(3*numVertices != this->numOfDOF)
    {
        // TODO: Raise error.
        return;
    }

    for(size_t i = 0, idx = 0; i < numVertices; ++i, idx+=3)
    {
        vertices[i]->convertToArray(positions.data()+idx);
    }

    auto boundaryConditions = this->loadBoundaryConditions();
    initialState->setBoundaryConditions(boundaryConditions);

    this->currentState = std::make_shared<OdeSystemState>();
    *this->currentState = *this->initialState;

    this->newState = std::make_shared<OdeSystemState>();
    this->previousState = std::make_shared<OdeSystemState>();
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::initialize()
{
    DeformableSceneObject::initialize();

    this->loadInitialStates();

    this->initConstitutiveModel();
    this->initForceModel();
    this->initMassMatrix();
    this->initDampingMatrix();
    this->initTangentStiffnessMatrix();
}

//---------------------------------------------------------------------------
bool VegaFEMDeformableSceneObject::configure(const std::string &configFile)
{
    this->vegaFemConfig = Core::make_unique<VegaConfiguration>(configFile);

    this->setMassMatrix();
    this->setTangentStiffnessMatrix();
    this->setDampingMatrix();
    this->setOdeRHS();

    size_t numNodes = this->volumetricMesh->getNumberOfVertices();
    this->numOfDOF = 3 * numNodes;

    this->f.resize(this->numOfDOF);
    this->f.setZero();
    return false;
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::initMassMatrix(bool saveToDisk)
{
    if(!this->volumetricMesh)
    {
        // TODO: log this
        return;
    }

    SparseMatrix *matrix;
    GenerateMassMatrix::computeMassMatrix(this->volumetricMesh->getVegaMesh().get(),
                                          &matrix,
                                          true);
    this->vegaMassMatrix.reset(matrix);

    // Flatten the internal mass matrix arrays and store them locally.
    this->flattenVegaSparseMatrix(this->vegaMassMatrix,
                                  this->massMatrixColIndices,
                                  this->massMatrixRowPointers,
                                  this->massMatrixValues);

    // Construct the Eigen mass matrix by mapping the arrays
    this->M = Eigen::MappedSparseMatrix<double,Eigen::RowMajor>(
                  this->vegaMassMatrix->GetNumRows(),
                  this->vegaMassMatrix->GetNumColumns(),
                  this->vegaMassMatrix->GetNumEntries(),
                  this->massMatrixRowPointers.data(),
                  this->massMatrixColIndices.data(),
                  this->massMatrixValues.data());

    this->M.makeCompressed();
    if(saveToDisk)
    {
        char name[] = "ComputedMassMatrix.mass";
        this->vegaMassMatrix->Save(name);
    }
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::initTangentStiffnessMatrix()
{
    if(!this->forceModel)
    {
        // TODO: log this
        return;
    }

    SparseMatrix *matrix;
    this->forceModel->GetTangentStiffnessMatrixTopology(&matrix);

    if(!matrix)
    {
        // TODO: log this
        return;
    }

    this->vegaTangentStiffnessMatrix.reset(matrix);

    if(!this->vegaMassMatrix)
    {
        // TODO: log this
        return;
    }

    this->vegaTangentStiffnessMatrix->BuildSubMatrixIndices(*this->vegaMassMatrix.get());

    if(!this->dampingMatrix)
    {
        // TODO: log this
        return;
    }

    this->vegaTangentStiffnessMatrix->BuildSubMatrixIndices(*this->dampingMatrix.get(), 1);

    // Flatten the internal mass matrix arrays and store them locally.
    this->flattenVegaSparseMatrix(this->vegaTangentStiffnessMatrix,
                                  this->tangentStiffnessMatrixColIndices,
                                  this->tangentStiffnessMatrixRowPointers,
                                  this->tangentStiffnessMatrixValues);

    // Construct the eigen stiffness matrix by mapping the arrays
    this->K = Eigen::MappedSparseMatrix<double,Eigen::RowMajor>(
                  this->vegaTangentStiffnessMatrix->GetNumRows(),
                  this->vegaTangentStiffnessMatrix->GetNumColumns(),
                  this->vegaTangentStiffnessMatrix->GetNumEntries(),
                  this->tangentStiffnessMatrixRowPointers.data(),
                  this->tangentStiffnessMatrixColIndices.data(),
                  this->tangentStiffnessMatrixValues.data());
    this->K.makeCompressed();
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::initDampingMatrix()
{
    auto meshGraph = this->volumetricMesh->getMeshGraph();

    if(!meshGraph)
    {
        // TODO: log this
        return;
    }

    SparseMatrix *matrix;
    meshGraph->GetLaplacian(&matrix, 1);

    if(!matrix)
    {
        // TODO: log this
        return;
    }

    this->dampingMatrix.reset(matrix);

    matrix->ScalarMultiply(
        this->vegaFemConfig->floatsOptionMap.at("dampingLaplacianCoefficient"));

    // Flatten the internal mass matrix arrays and store them locally.
    this->flattenVegaSparseMatrix(this->dampingMatrix,
                                  this->dampingMatrixColIndices,
                                  this->dampingMatrixColPointers,
                                  this->dampingMatrixValues);

    // Construct the Eigen damping matrix by mapping the arrays
    this->D = Eigen::MappedSparseMatrix<double,Eigen::RowMajor>(
                  this->dampingMatrix->GetNumRows(),
                  this->dampingMatrix->GetNumColumns(),
                  this->dampingMatrix->GetNumEntries(),
                  this->dampingMatrixColPointers.data(),
                  this->dampingMatrixColIndices.data(),
                  this->dampingMatrixValues.data());
    this->D.makeCompressed();
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::initConstitutiveModel()
{
    auto numThreads = this->vegaFemConfig->intsOptionMap.at("numberOfThreads");
    auto gravity = this->vegaFemConfig->floatsOptionMap.at("gravity");
    auto withGravity = gravity != 0.0 ? true : false;
    auto mesh = this->volumetricMesh->getVegaMesh();
    auto tetMesh = std::dynamic_pointer_cast<TetMesh>(mesh);

    switch(this->vegaFemConfig->forceModelType)
    {
        case VegaConfiguration::MethodType::StVK:
        {
            unsigned int loadingFlag = 1; // 0 = use low-memory version (slower),
            // 1 = use high-memory version (faster)
            StVKElementABCD *precomputedIntegrals
                = StVKElementABCDLoader::load(mesh.get(),
                                              loadingFlag);

            if(!precomputedIntegrals)
            {
                std::cout << "VEGA: error! unable to load the StVK integrals."
                << std::endl;
                return;
            }

            if(numThreads > 0)
            {
                this->stVKInternalForces = std::make_shared<StVKInternalForcesMT>(
                                               mesh.get(),
                                               precomputedIntegrals,
                                               withGravity,
                                               gravity,
                                               numThreads);

                this->stVKStiffnessMatrix =
                    std::make_shared<StVKStiffnessMatrixMT>(stVKInternalForces.get(),
                                                numThreads);
            }
            else
            {
                this->stVKInternalForces = std::make_shared<StVKInternalForces>(
                                               mesh.get(),
                                               precomputedIntegrals,
                                               withGravity,
                                               gravity);

                this->stVKStiffnessMatrix = std::make_shared<StVKStiffnessMatrix>(
                                                stVKInternalForces.get());
            }

            break;
        }

        case VegaConfiguration::MethodType::Corotational:
        {
            if(!tetMesh)
            {
                return;
            }

            if(numThreads > 0)
            {
                this->corrotationalInternalForces =
                    std::make_shared<CorotationalLinearFEMMT>(tetMesh.get(),
                            numThreads);
            }
            else
            {
                this->corrotationalInternalForces =
                    std::make_shared<CorotationalLinearFEM>(tetMesh.get());
            }

            break;
        }

        case VegaConfiguration::MethodType::Invertible:
        {
            if(!tetMesh)
            {
                return;
            }

            auto compressionResistance =
                this->vegaFemConfig->floatsOptionMap.at("compressionResistance");
            auto withCompressionResistance = compressionResistance == 0.0 ? 0 : 1;

            switch(this->vegaFemConfig->isotropicMaterialType)
            {
                case VegaConfiguration::InvertibleMaterialType::StVK:
                {
                    this->isotropicMaterial =
                        std::make_shared<StVKIsotropicMaterial>(
                            tetMesh.get(),
                            withCompressionResistance,
                            compressionResistance);
                    break;
                }

                case VegaConfiguration::InvertibleMaterialType::NeoHookean:
                {
                    this->isotropicMaterial =
                        std::make_shared<NeoHookeanIsotropicMaterial>(
                            tetMesh.get(),
                            withCompressionResistance,
                            compressionResistance);
                    break;
                }

                case VegaConfiguration::InvertibleMaterialType::MooneyRivlin:
                {
                    this->isotropicMaterial =
                        std::make_shared<MooneyRivlinIsotropicMaterial>(
                            tetMesh.get(),
                            withCompressionResistance,
                            compressionResistance);
                    break;
                }

                default:
                {
                    std::cout << "Error unknown material type." << std::endl;
                }

            }

            if(!this->isotropicMaterial)
            {
                return;
            }

            auto inversionThreshold =
                this->vegaFemConfig->floatsOptionMap.at("inversionThreshold");

            if(numThreads > 0)
            {
                this->isotropicHyperElasticForces =
                    std::make_shared<IsotropicHyperelasticFEMMT>(tetMesh.get(),
                            this->isotropicMaterial.get(),
                            inversionThreshold,
                            withGravity,
                            gravity,
                            numThreads);
            }
            else
            {
                this->isotropicHyperElasticForces =
                    std::make_shared<IsotropicHyperelasticFEM>(tetMesh.get(),
                            this->isotropicMaterial.get(),
                            inversionThreshold,
                            withGravity,
                            gravity);
            }

            break;
        }

        default:
        {
            std::cout <<  "Unknown force model type." << std::endl;
        }

    }
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::initForceModel()
{
    switch(this->vegaFemConfig->forceModelType)
    {
        case VegaConfiguration::MethodType::StVK:
        {
            if(!stVKInternalForces || !stVKStiffnessMatrix)
            {
                return;
            }

            this->forceModel = std::make_shared<StVKForceModel>(
                                   stVKInternalForces.get(),
                                   stVKStiffnessMatrix.get());

            auto &uInitial = this->getInitialState()->getPositions();
            auto &uCurrent = this->currentState->getPositions();
            this->forceModel->GetInternalForce(uInitial.data(), uCurrent.data());
            break;
        }

        case VegaConfiguration::MethodType::Linear:
        {
            if(!stVKInternalForces)
            {
                return;
            }

            this->forceModel = std::make_shared<LinearFEMForceModel>(
                                   stVKInternalForces.get());
            break;
        }

        case VegaConfiguration::MethodType::Corotational:
        {
            if(!this->corrotationalInternalForces)
            {
                return;
            }

            this->forceModel = std::make_shared<CorotationalLinearFEMForceModel>(
                                   this->corrotationalInternalForces.get());
            break;
        }

        case VegaConfiguration::MethodType::Invertible:
        {
            if(!this->isotropicHyperElasticForces)
            {
                return;
            }

            this->forceModel = std::make_shared<IsotropicHyperelasticFEMForceModel>(
                                   this->isotropicHyperElasticForces.get());
            break;
        }

        default:
        {
            std::cerr << "Unknown force model" << std::endl;
        }
    }
}

//---------------------------------------------------------------------------
std::vector< std::size_t > VegaFEMDeformableSceneObject::loadBoundaryConditions()
{
    auto fileName = this->vegaFemConfig->stringsOptionMap.at("fixedDOFFilename");
    std::vector<size_t> fixedVertices;

    if(fileName.empty())
    {
        return fixedVertices;
    }

    /// Read the list of vertices from file
    std::ifstream file(fileName.data());

    if(file.is_open())
    {
        size_t index;

        while(!file.eof())
        {
            file >> index;
            fixedVertices.emplace_back(index);
        }
    }

    file.close();

    return fixedVertices; // Guaranteed not to copy the vector
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::flattenVegaSparseMatrix(std::shared_ptr<SparseMatrix> matrix,
                                                           std::vector< int > &colIndices,
                                                           std::vector<int> &rowPointers,
                                                           std::vector< double > &values)
{
    auto rowLengths = matrix->GetRowLengths();
    auto colPointers = matrix->GetColumnIndices();
    auto nonZeroValues = matrix->GetEntries();

    // Clean the arrays
    colIndices.clear();
    rowPointers.clear();
    values.clear();

    rowPointers.push_back(0);
    // Flatten the internal mass matrix arrays an store them.
    for(int row = 0, end = matrix->GetNumRows(); row < end; ++row)
    {
        colIndices.insert(std::end(colIndices),
                          colPointers[row],
                          colPointers[row] + rowLengths[row]);

        values.insert(std::end(values),
                      nonZeroValues[row],
                      nonZeroValues[row] + rowLengths[row]);

        rowPointers.push_back(rowPointers[row]+rowLengths[row]);
    }
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::updateValuesFromMatrix(std::shared_ptr<SparseMatrix> matrix,
                                                          std::vector<double> &values)
{
    auto rowLengths = matrix->GetRowLengths();
    auto nonZeroValues = matrix->GetEntries();

    // Flatten the internal non-zeros value array and store it in values.
    auto pos = std::begin(values);

    for(int row = 0, end = matrix->GetNumRows(); row < end; ++row)
    {
        /// This operation should not add new values to the array since the matrices
        /// structures should remain the same. It just replaces the values in the array.
        values.insert(pos,
                      nonZeroValues[row],
                      nonZeroValues[row] + rowLengths[row]);

        pos += rowLengths[row];
    }
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::setOdeRHS()
{
    auto odeRHS = [this](const OdeSystemState & s)
    {
        this->f = -this->C * s.getVelocities() - this->K * s.getPositions();
        return this->f;
    };
    this->setFunction(odeRHS);
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::setTangentStiffnessMatrix()
{
    auto tangentStiffness = [&,this](const OdeSystemState & s)
    {
        double *data = const_cast<double*>(s.getPositions().data());
        this->forceModel->GetTangentStiffnessMatrix(data,
                this->vegaTangentStiffnessMatrix.get());
        this->updateValuesFromMatrix(this->vegaTangentStiffnessMatrix,
                                     this->tangentStiffnessMatrixValues);
        return this->K;
    };
    this->setJaconbianFx(tangentStiffness);
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::setMassMatrix()
{
    auto massMatrix = [this](const OdeSystemState & /*s*/)
    {
        return this->M;
    };
    this->setMass(massMatrix);
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::setDampingMatrix()
{
    const auto &dampingStiffnessCoefficient =
        this->vegaFemConfig->floatsOptionMap.at("dampingStiffnessCoefficient");

    const auto &dampingMassCoefficient =
        this->vegaFemConfig->floatsOptionMap.at("dampingMassCoefficient");

    auto raleighDamping = [&,this](const OdeSystemState & /*s*/)
    {
        this->C = dampingStiffnessCoefficient * this->K +
                  dampingMassCoefficient * this->M;
        return this->C;
    };
    this->setJaconbianFv(raleighDamping);
}
