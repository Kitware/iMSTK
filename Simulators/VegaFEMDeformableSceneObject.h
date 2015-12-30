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

#ifndef VEGAFEMDEFORMABLESCENEOBJECT_H
#define VEGAFEMDEFORMABLESCENEOBJECT_H

// STL includes
#include <memory>

// iMSTK includes
#include "Simulators/DeformableSceneObject.h"
#include "Mesh/VegaVolumetricMesh.h"
#include "Core/MakeUnique.h"
#include "IO/IOMesh.h"

// Vega includes
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

class VegaFEMDeformableSceneObject : public DeformableSceneObject
{
public:
    ///
    /// \brief Constructor. This is the only constructor and takes a valid mesh file name
    ///     and a valid VegaFEM configuration file.
    ///
    VegaFEMDeformableSceneObject(const std::string &meshFilename,
                                 const std::string &vegaConfigFileName)
        : vegaFemConfig(Core::make_unique<VegaConfiguration>(vegaConfigFileName))
    {
        this->loadVolumeMesh(meshFilename);
        this->initMassMatrix();
        this->initTangentStiffnessMatrix();
        this->initDampingMatrix();

        size_t numNodes = this->volumetricMesh->getNumberOfVertices();
        size_t numOfDOF = 3 * numNodes;

        this->f.resize(numOfDOF);
        this->f.setZero();

        this->positions = Eigen::Map<core::Vectord>(
            &this->volumetricMesh->getVegaMesh()->getVertex(0)[0],
            numOfDOF);

        auto initialState = std::make_shared<OdeSystemState>(numOfDOF);
        initialState->getPositions() = this->positions;

        auto boundaryConditions = this->loadBoundaryConditions();
        initialState->setBoundaryConditions(boundaryConditions);

        *this->currentState = *initialState;

        this->generateConstitutiveModel();
        this->createForceModel();

    }

    VegaFEMDeformableSceneObject() = delete;
    ~VegaFEMDeformableSceneObject() = default;

    ///
    /// \brief Load the Vega volume. This is a .veg file containing vertices, connections
    ///     and material properties.
    ///
    /// \param fileName Path to Vega (.veg) mesh.
    ///
    void loadVolumeMesh(const std::string &fileName)
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

    ///
    /// \brief Initialize the mass matrix and convert it to an Eigen container.
    ///
    /// \param saveToDisk Dave mass matrix to disk if true.
    ///
    void initMassMatrix(bool saveToDisk = false)
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

        size_t numNonZeros = std::accumulate(this->vegaMassMatrix->GetRowLengths(),
                                             this->vegaMassMatrix->GetRowLengths() +
                                             this->vegaMassMatrix->GetNumRows());

        // Flatten the internal mass matrix arrays and store them locally.
        this->flattenVegaSparseMatrix(this->vegaMassMatrix,
                                      this->massMatrixColPointers,
                                      this->massMatrixValues);

        // Construct the Eigen mass matrix by mapping the arrays
        this->M = Eigen::MappedSparseMatrix<core::SparseMatrixd>(
                      this->vegaMassMatrix->GetNumRows(),
                      this->vegaMassMatrix->GetNumColumns(),
                      this->vegaMassMatrix->GetNumEntries(),
                      this->vegaMassMatrix->GetRowLengths(),
                      this->massMatrixColPointers.data(),
                      this->massMatrixValues.data());

        if(saveToDisk)
        {
            this->vegaMassMatrix->Save("ComputedMassMatrix.mass");
        }
    }

    ///
    /// \brief Initialize the tangent stiffness matrix and convert it to an Eigen
    ///  container. Upon return the this->M matrix will contain the matrix entries.
    ///
    void initTangentStiffnessMatrix()
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
        this->K = Eigen::MappedSparseMatrix<core::SparseMatrixd>(
                      this->vegaTangentStiffnessMatrix->GetNumRows(),
                      this->vegaTangentStiffnessMatrix->GetNumColumns(),
                      this->vegaTangentStiffnessMatrix->GetNumEntries(),
                      this->vegaTangentStiffnessMatrix->GetRowLengths(),
                      this->tangentStiffnessMatrixColPointers.data(),
                      this->tangentStiffnessMatrixValues.data());
    }

    ///
    /// \brief Initialize the Lagrangian matrix and convert it to an Eigen
    ///  container.
    ///
    void initDampingMatrix()
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

        // Construct the eigen damping matrix by mapping the arrays
        this->D = Eigen::MappedSparseMatrix<core::SparseMatrixd>(
                      this->dampingMatrix->GetNumRows(),
                      this->dampingMatrix->GetNumColumns(),
                      this->dampingMatrix->GetNumEntries(),
                      this->dampingMatrix->GetRowLengths(),
                      this->dampingMatrixColPointers.data(),
                      this->dampingMatrixValues.data());
    }

    ///
    /// \brief Initialize the constitutive model used by Vegas force model.
    ///
    void generateConstitutiveModel()
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

//
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
                                this->isotropicMaterial,
                                inversionThreshold,
                                withGravity,
                                gravity,
                                numThreads);
                }
                else
                {
                    this->corrotationalInternalForces =
                        std::make_shared<IsotropicHyperelasticFEM>(tetMesh.get(),
                                this->isotropicMaterial,
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

    ///
    /// \brief Instantiate the Vega force model.
    ///
    void createForceModel()
    {
        int numThreads = this->vegaFemConfig->intsOptionMap.at("numberOfThreads");

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
                                       this->isotropicHyperElasticForces);
                break;
            }

            default:
            {
                std::cerr << "Unknown force model" << std::endl;
            }
        }
    }


    ///
    /// \brief Load boundary conditions. The file contains the indices of the degrees of
    ///     freedom that are subjected to Dirichlet boundary conditions.
    ///
    std::vector<size_t> loadBoundaryConditions()
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

    ///
    /// \brief Helper function to interface Vega sparse matrices with Eigen matrices.
    ///
    void flattenVegaSparseMatrix(std::shared_ptr<SparseMatrix> matrix,
                                 std::vector<int> &colIndices,
                                 std::vector<int> &values)
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
                              colPointers[row]+rowLengths[row]);

            values.insert(std::end(values),
                          nonZeroValues[row],
                          nonZeroValues[row]+rowLengths[row]);
        }
    }

    ///
    /// \brief Helper function to interface Vega sparse matrices with Eigen matrices.
    ///
    void updateValuesFromMatrix(std::shared_ptr<SparseMatrix> matrix,
                                std::vector<int> &values)
    {
        auto rowLengths = matrix->GetRowLengths();
        auto nonZeroValues = matrix->GetEntries();

        // Flatten the internal non-zeros value array and store it in values.
        auto pos = std::begin(values);

        for(int row = 0, end = matrix->GetNumRows(); row < end; ++row)
        {
            values.insert(pos,
                          nonZeroValues[row],
                          nonZeroValues[row]+rowLengths[row]);

            pos += rowLengths[row];
        }
    }

    ///
    /// \brief Set the ODE system right hand side function.
    ///
    void setOdeRHS()
    {
        auto odeRHS = [this](const OdeSystemState & s)
        {
            this->f = -this->C * s.getVelocities() - this->K * s.getPositions();
            return this->f;
        };
        this->setFunction(odeRHS);
    }

    ///
    /// \brief Set the the tangent stiffness matrix function to be evaluated by the ODE solver.
    ///
    void setTangentStiffnessMatrix()
    {
        auto tangentStiffness = [this](const OdeSystemState & s)
        {
            this->forceModel->GetTangentStiffnessMatrix(s.getPositions().data(),
                                                        this->vegaTangentStiffnessMatrix.get());
            this->updateValuesFromMatrix(this->vegaTangentStiffnessMatrix,
                                         this->tangentStiffnessMatrixValues);
            return this->K;
        };
        this->setJaconbianFx(tangentStiffness);
    }

    ///
    /// \brief Set the the mass matrix function to be evaluated by the ODE solver.
    ///
    void setMassMatrix()
    {
        auto massMatrix = [this](const OdeSystemState & s)
        {
            return this->M;
        };
        this->setMass(massMatrix);
    }

    ///
    /// \brief Set the the Raleigh damping matrix function to be evaluated by the ODE solver.
    ///
    void setDampingMatrix()
    {
        auto dampingStiffnessCoefficient =
            this->vegaFemConfig->floatsOptionMap.at("dampingStiffnessCoefficient");

        auto dampingMassCoefficient =
            this->vegaFemConfig->floatsOptionMap.at("dampingMassCoefficient");

        auto raleighDamping = [this](const OdeSystemState & s)
        {
            this->C = dampingStiffnessCoefficient * this->K +
                dampingMassCoefficient * this->M;
            return this->C;
        };
        this->setJaconbianFv(raleighDamping);
    }

private:
    // Volumetric mesh
    std::shared_ptr<VegaVolumetricMesh> volumetricMesh;

    // Vega force model
    std::shared_ptr<ForceModel> forceModel;

    // Vega internal constitutive models
    std::shared_ptr<StVKInternalForces> stVKInternalForces;
    std::shared_ptr<CorotationalLinearFEM> corrotationalInternalForces;
    std::shared_ptr<StVKStiffnessMatrix> stVKStiffnessMatrix;
    std::shared_ptr<IsotropicMaterial> isotropicMaterial;
    std::shared_ptr<IsotropicHyperelasticFEM> isotropicHyperElasticForces;

    // Vega mass matrix
    std::shared_ptr<SparseMatrix> vegaMassMatrix;
    std::vector<double> massMatrixValues;
    std::vector<int> massMatrixColPointers;

    // Vega Tangent stiffness matrix
    std::shared_ptr<SparseMatrix> vegaTangentStiffnessMatrix;
    std::vector<double> tangentStiffnessMatrixValues;
    std::vector<int> tangentStiffnessMatrixColPointers;

    // Vega Laplacian damping matrix
    std::shared_ptr<SparseMatrix> dampingMatrix;
    std::vector<double> dampingMatrixValues;
    std::vector<int> dampingMatrixColPointers;

    // Vega-Eigen Maps
    core::Vectord positions;

    // Vega configuration parser interface
    class VegaConfiguration;
    std::unique_ptr<VegaConfiguration> vegaFemConfig;
};

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
    VegaConfiguration(const std::string &configurationFile, bool verbose = false)
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

    std::string vegaConfigFile; ///> Store configuration file.

    std::map<std::string, double> floatsOptionMap;  ///> Map for floating point configuration variables.
    std::map<std::string, int> intsOptionMap;       ///> Map for int configuration variables.
    std::map<std::string, std::string> stringsOptionMap;///> Map for string configuration variables.

    MethodType forceModelType; ///> Force model type used.
    InvertibleMaterialType isotropicMaterialType; ///> Constitutive law for nonlinear materials.
};

#endif // VEGAFEMDEFORMABLESCENEOBJECT_H
