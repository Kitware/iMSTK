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

#include "Simulators/DeformableSceneObject.h"
#include "Mesh/VegaVolumetricMesh.h"
#include "Core/MakeUnique.h"
#include "IO/IOMesh.h"
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

class VegaFEMDeformableSceneObject : public DeformableSceneObject
{
public:
    ///
    /// \brief Constructors.
    ///
    VegaFEMDeformableSceneObject(const std::string &meshFilename,
                                 const std::string &vegaConfigFileName)
        : vegaFemConfig(Core::make_unique<VegaConfiguration>(vegaConfigFileName))
    {
        this->loadVolumeMesh(meshFilename);
        this->computeMassMatrix();

        size_t numNodes = this->volumetricMesh->getNumberOfVertices();
        size_t numOfDOF = 3 * numNodes;

        this->f.resize(numOfDOF);
        this->f.setZero();

        this->positions = Eigen::Map<core::Vectord>(&this->volumetricMesh->getVegaMesh()->getVertex(0)[0],
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

    void loadVolumeMesh(const std::string &fileName)
    {
        auto ioMesh = std::make_shared<IOMesh>();
        ioMesh->read(fileName);

        this->volumetricMesh = std::static_pointer_cast<VegaVolumetricMesh>(ioMesh->getMesh());

        if(!this->volumetricMesh)
        {
            // TODO: Print error message
            return;
        }
    }

    void computeMassMatrix(bool saveToDisk = false)
    {
        if(!this->volumetricMesh)
        {
            return;
        }

        this->vegaMassMatrix = std::make_shared<SparseMatrix>();
        GenerateMassMatrix::computeMassMatrix(this->volumetricMesh->getVegaMesh().get(),
                                              &this->vegaMassMatrix,
                                              false);

        this->M = Eigen::MappedSparseMatrix<core::SparseMatrixd>(this->vegaMassMatrix->GetNumRows(),
                  this->vegaMassMatrix->GetNumColumns(),
                  this->vegaMassMatrix->GetNumEntries(),
                  this->vegaMassMatrix->GetRowLengths(),
                  this->vegaMassMatrix->GetColumnIndices()[0],
                  this->vegaMassMatrix->GetDataHandle()[0]);

        if(saveToDisk)
        {
            this->vegaMassMatrix->Save("ComputedMassMatrix.mass");
        }
    }

    void generateConstitutiveModel()
    {
        int numThreads = this->vegaFemConfig->intsOptionMap.at("numberOfThreads");
        auto mesh = this->volumetricMesh->getVegaMesh();
        auto tetMesh = std::dynamic_pointer_cast<TetMesh>(mesh);

        switch(this->vegaFemConfig->forceModelType)
        {
            case VegaConfiguration::MethodType::StVK:
            {
                unsigned int loadingFlag = 1; // 0 = use low-memory version (slower), 1 = use high-memory version (faster)
                StVKElementABCD *precomputedIntegrals = StVKElementABCDLoader::load(mesh.get(),
                                                        loadingFlag);

                if(numThreads > 0)
                {
                    if(!precomputedIntegrals)
                    {
                        std::cout << "VEGA: error! unable to load the StVK integrals." << std::endl;
                        return;
                    }

                    this->stVKInternalForces = std::make_shared<StVKInternalForces>(
                                                   mesh.get(),
                                                   precomputedIntegrals,
                                                   true,
                                                   this->vegaFemConfig->floatsOptionMap.at("gravity"));

                    this->stVKStiffnessMatrix = std::make_shared<StVKStiffnessMatrix>(stVKInternalForces.get());
                }
                else
                {
                    this->stVKInternalForces = std::make_shared<StVKInternalForcesMT>(
                                                   mesh.get(),
                                                   precomputedIntegrals,
                                                   true,
                                                   this->vegaFemConfig->floatsOptionMap.at("gravity"),
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
                    this->corrotationalInternalForces = std::make_shared<CorotationalLinearFEMMT>(tetMesh.get(),
                                                        numThreads);
                }
                else
                {
                    this->corrotationalInternalForces = std::make_shared<CorotationalLinearFEM>(tetMesh.get());
                }

                break;
            }

            case VegaConfiguration::MethodType::Invertible:
            {
                if(!tetMesh)
                {
                    return;
                }

                auto compressionResistance = this->vegaFemConfig->floatsOptionMap.at("compressionResistance");

                switch(this->vegaFemConfig->isotropicMaterialType)
                {
                    case VegaConfiguration::InvertibleMaterialType::StVK:
                    {
                        this->isotropicMaterial = std::make_shared<StVKIsotropicMaterial>(tetMesh.get(),
                                                  true,
                                                  compressionResistance);
                        break;
                    }

                    case VegaConfiguration::InvertibleMaterialType::NeoHookean:
                    {
                        this->isotropicMaterial = std::make_shared<NeoHookeanIsotropicMaterial>(tetMesh.get(),
                                                  true,
                                                  compressionResistance);
                        break;
                    }

                    case VegaConfiguration::InvertibleMaterialType::MooneyRivlin:
                    {
                        this->isotropicMaterial = std::make_shared<MooneyRivlinIsotropicMaterial>(tetMesh.get(),
                                                  true,
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

                auto inversionThreshold = this->vegaFemConfig->floatsOptionMap.at("inversionThreshold");

                if(numThreads > 0)
                {
                    this->isotropicHyperElasticForces = std::make_shared<IsotropicHyperelasticFEMMT>(tetMesh.get(),
                                                        this->isotropicMaterial,
                                                        inversionThreshold,
                                                        true,
                                                        this->vegaFemConfig->floatsOptionMap.at("gravity"),
                                                        numThreads);
                }
                else
                {
                    this->corrotationalInternalForces = std::make_shared<IsotropicHyperelasticFEM>(tetMesh.get(),
                                                        this->isotropicMaterial,
                                                        inversionThreshold,
                                                        true,
                                                        this->vegaFemConfig->floatsOptionMap.at("gravity"));
                }

                break;
            }

            default:
            {
                std::cout <<  "Unknown force model type." << std::endl;
            }

        }
    }

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

                this->forceModel = std::make_shared<StVKForceModel>(stVKInternalForces.get(),
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

                this->forceModel = std::make_shared<LinearFEMForceModel>(stVKInternalForces.get());

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

                this->forceModel = std::make_shared<IsotropicHyperelasticFEMForceModel>(this->isotropicHyperElasticForces);



                break;
            }

            default:
            {
                std::cerr << "Unknown force model" << std::endl;
            }
        }
    }

    std::shared_ptr<SparseMatrix> getLaplacianDampingMatrix()
    {
        auto meshGraph = this->volumetricMesh->getMeshGraph();

        if(!meshGraph)
        {
            return nullptr;
        }

        SparseMatrix *laplacian;
        meshGraph->GetLaplacian(&laplacian, 1);

        if(!laplacian)
        {
            return nullptr;
        }

        std::shared_ptr<SparseMatrix> matrix;
        matrix.reset(laplacian);
        matrix->ScalarMultiply(
            this->vegaFemConfig->floatsOptionMap.at("dampingLaplacianCoefficient"));
        return matrix;
    }

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

        return fixedVertices; // Guaranteed not to copy the vector
    }

private:
    std::shared_ptr<VegaVolumetricMesh> volumetricMesh; ///< Volumetric mesh

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

    // Vega-Eigen Maps
    core::Vectord positions;

    class VegaConfiguration;
    std::unique_ptr<VegaConfiguration> vegaFemConfig;

};

// VegaFEM includes
#include "configFile.h"

///
/// \brief Vega configuration parser interface. This class interfaces with
///  vegas configuration parser and define relavant configurations for our
///  solvers.
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
        vegaConfigFileOptions.addOption("femMethod",
                                        femMethod,
                                        "StVK");

        char invertibleMaterial[256];
        vegaConfigFileOptions.addOption("invertibleMaterial",
                                        invertibleMaterial,
                                        "StVK");

        char fixedDOFFilename[256];
        vegaConfigFileOptions.addOption("fixedDOFFilename",
                                        fixedDOFFilename,
                                        "");

        double dampingMassCoefficient;
        vegaConfigFileOptions.addOption("dampingMassCoefficient",
                                        &dampingMassCoefficient);

        double dampingLaplacianCoefficient;
        vegaConfigFileOptions.addOption("dampingLaplacianCoefficient",
                                        &dampingLaplacianCoefficient,
                                        0.0);

        double dampingStiffnessCoefficient;
        vegaConfigFileOptions.addOption("dampingStiffnessCoefficient",
                                        &dampingStiffnessCoefficient);

        double deformationCompliance;
        vegaConfigFileOptions.addOption("deformationCompliance",
                                        &deformationCompliance,
                                        1.0);

        double gravity = -9.81;
        vegaConfigFileOptions.addOption("gravity",
                                        &gravity,
                                        gravity);

        double compressionResistance = 500;
        vegaConfigFileOptions.addOption("compressionResistance",
                                        &compressionResistance,
                                        compressionResistance);

        double inversionThreshold;
        vegaConfigFileOptions.addOption("inversionThreshold",
                                        &inversionThreshold);

        int numberOfThreads;
        vegaConfigFileOptions.addOption("numberOfThreads",
                                        &numberOfThreads,
                                        0);

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

        // Store parsed floating point values
        this->floatsOptionMap.emplace("dampingMassCoefficient", dampingMassCoefficient);
        this->floatsOptionMap.emplace("dampingLaplacianCoefficient", dampingLaplacianCoefficient);
        this->floatsOptionMap.emplace("dampingStiffnessCoefficient", dampingStiffnessCoefficient);
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

    std::string vegaConfigFile; ///> Store configuration file.

    std::map<std::string, double> floatsOptionMap; ///> Map for floating point configuration variables
    std::map<std::string, int> intsOptionMap;///> Map for int configuration variables
    std::map<std::string, std::string> stringsOptionMap;///> Map for string configuration variables

    MethodType forceModelType;
    InvertibleMaterialType isotropicMaterialType;
};

#endif // VEGAFEMDEFORMABLESCENEOBJECT_H
