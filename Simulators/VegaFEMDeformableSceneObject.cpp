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
#include "configFile.h"

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
///     materialDensity             Homogenious material density
///                                     [default = 1000]
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
    if(this->vegaConfigFile.empty())
    {
        std::cout << "Empty configuration filename." << std::endl;
        return;
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

    double materialDensity = 1000.0;
    vegaConfigFileOptions.addOptionOptional("materialDensity",
                                            &materialDensity,
                                            materialDensity);

    int numberOfThreads = 0;
    vegaConfigFileOptions.addOptionOptional("numberOfThreads",
                                            &numberOfThreads,
                                            numberOfThreads);

    // Parse the configuration file
    if(vegaConfigFileOptions.parseOptions(configurationFile.data()) != 0)
    {
        std::cout << "VEGA: error! parsing options.\n";
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
    this->floatsOptionMap.emplace("materialDensity", materialDensity);

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
        // TODO: Rise error
        return;
    }

    this->configure(vegaConfigFileName);

    this->loadInitialStates();

    this->generateConstitutiveModel();
    this->createForceModel();
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
    auto initialState = std::make_shared<OdeSystemState>(this->numOfDOF);
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

    *this->currentState = *initialState;
}

//---------------------------------------------------------------------------
bool VegaFEMDeformableSceneObject::configure(const std::string &configFile)
{
    this->vegaFemConfig = Core::make_unique<VegaConfiguration>(configFile);

    this->initMassMatrix();
    this->initTangentStiffnessMatrix();
    this->initDampingMatrix();

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
        return;
    }

    SparseMatrix *matrix;
    GenerateMassMatrix::computeMassMatrix(this->volumetricMesh->getVegaMesh().get(),
                                          &matrix,
                                          true);
    this->vegaMassMatrix.reset(matrix);

    // Flatten the internal mass matrix arrays and store them locally.
    this->flattenVegaSparseMatrix(this->vegaMassMatrix,
                                  this->massMatrixColPointers,
                                  this->massMatrixValues);

    // Construct the Eigen mass matrix by mapping the arrays
    this->M = Eigen::MappedSparseMatrix<double>(
                  this->vegaMassMatrix->GetNumRows(),
                  this->vegaMassMatrix->GetNumColumns(),
                  this->vegaMassMatrix->GetNumEntries(),
                  this->vegaMassMatrix->GetRowLengths(),
                  this->massMatrixColPointers.data(),
                  this->massMatrixValues.data());

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
        return;
    }

    SparseMatrix *matrix;
    this->forceModel->GetTangentStiffnessMatrixTopology(&matrix);

    if(!matrix)
    {
        return;
    }

    this->vegaTangentStiffnessMatrix.reset(matrix);

    if(!this->vegaMassMatrix)
    {
        return;
    }

    this->vegaTangentStiffnessMatrix->BuildSubMatrixIndices(*this->vegaMassMatrix.get());

    if(!this->dampingMatrix)
    {
        return;
    }

    this->vegaTangentStiffnessMatrix->BuildSubMatrixIndices(*this->dampingMatrix.get(), 1);

    // Flatten the internal mass matrix arrays and store them locally.
    this->flattenVegaSparseMatrix(this->vegaTangentStiffnessMatrix,
                                  this->tangentStiffnessMatrixColPointers,
                                  this->tangentStiffnessMatrixValues);

    // Construct the eigen stiffness matrix by mapping the arrays
    this->K = Eigen::MappedSparseMatrix<double>(
                  this->vegaTangentStiffnessMatrix->GetNumRows(),
                  this->vegaTangentStiffnessMatrix->GetNumColumns(),
                  this->vegaTangentStiffnessMatrix->GetNumEntries(),
                  this->vegaTangentStiffnessMatrix->GetRowLengths(),
                  this->tangentStiffnessMatrixColPointers.data(),
                  this->tangentStiffnessMatrixValues.data());
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::initDampingMatrix()
{
    auto meshGraph = this->volumetricMesh->getMeshGraph();

    if(!meshGraph)
    {
        return;
    }

    SparseMatrix *matrix;
    meshGraph->GetLaplacian(&matrix, 1);

    if(!matrix)
    {
        return;
    }

    this->dampingMatrix.reset(matrix);

    matrix->ScalarMultiply(
        this->vegaFemConfig->floatsOptionMap.at("dampingLaplacianCoefficient"));

    // Flatten the internal mass matrix arrays and store them locally.
    this->flattenVegaSparseMatrix(this->dampingMatrix,
                                  this->dampingMatrixColPointers,
                                  this->dampingMatrixValues);

    // Construct the Eigen damping matrix by mapping the arrays
    this->D = Eigen::MappedSparseMatrix<double>(
                  this->dampingMatrix->GetNumRows(),
                  this->dampingMatrix->GetNumColumns(),
                  this->dampingMatrix->GetNumEntries(),
                  this->dampingMatrix->GetRowLengths(),
                  this->dampingMatrixColPointers.data(),
                  this->dampingMatrixValues.data());
}

//---------------------------------------------------------------------------
void VegaFEMDeformableSceneObject::generateConstitutiveModel()
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

            if(numThreads > 0)
            {
                if(!precomputedIntegrals)
                {
                    std::cout << "VEGA: error! unable to load the StVK integrals."
                              << std::endl;
                    return;
                }

                this->stVKInternalForces = std::make_shared<StVKInternalForces>(
                                               mesh.get(),
                                               precomputedIntegrals,
                                               withGravity,
                                               gravity);

                this->stVKStiffnessMatrix =
                    std::make_shared<StVKStiffnessMatrix>(stVKInternalForces.get());
            }
            else
            {
                this->stVKInternalForces = std::make_shared<StVKInternalForcesMT>(
                                               mesh.get(),
                                               precomputedIntegrals,
                                               withGravity,
                                               gravity,
                                               numThreads);

                this->stVKStiffnessMatrix = std::make_shared<StVKStiffnessMatrixMT>(
                                                stVKInternalForces.get(),
                                                numThreads);
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
void VegaFEMDeformableSceneObject::createForceModel()
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
                                                           std::vector< double > &values)
{
    auto rowLengths = matrix->GetRowLengths();
    auto colPointers = matrix->GetColumnIndices();
    auto nonZeroValues = matrix->GetEntries();

    // Clean the arrays
    colIndices.clear();
    values.clear();

    // Flatten the internal mass matrix arrays an store them.
    for(int row = 0, end = matrix->GetNumRows(); row < end; ++row)
    {
        colIndices.insert(std::end(colIndices),
                          colPointers[row],
                          colPointers[row] + rowLengths[row]);

        values.insert(std::end(values),
                      nonZeroValues[row],
                      nonZeroValues[row] + rowLengths[row]);
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
    auto tangentStiffness = [this](const OdeSystemState & s)
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
