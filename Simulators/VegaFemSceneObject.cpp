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

// SimMedTK includes
#include "Simulators/VegaFemSceneObject.h"
#include "Core/Factory.h"
#include "Core/RenderDelegate.h"
#include "IO/IOMesh.h"
#include "Mesh/VegaVolumetricMesh.h"
#include "Geometry/MeshModel.h"

#include <exception>

VegaFemSceneObject::VegaFemSceneObject() :
    staticSolver(0),
    graphicFrame(0),
    explosionFlag(0),
    positiveDefinite(0)
{
    performaceTracker.initialize();

    this->type = core::ClassType::VegaFemSceneObject;

    this->name = "VegaFem_SceneObject_" + std::to_string(this->getUniqueId()->getId());
}

VegaFemSceneObject::VegaFemSceneObject(const std::shared_ptr<ErrorLog> /*p_log*/,
                                       const std::string ConfigFile) :
    staticSolver(0),
    graphicFrame(0),
    explosionFlag(0),
    positiveDefinite(0)
{
    performaceTracker.initialize();

    this->type = core::ClassType::VegaFemSceneObject;

    this->name = "VegaFem_SceneObject_" + std::to_string(this->getUniqueId()->getId());

    ConfigFileName = ConfigFile;

    configure(ConfigFileName);

    initialize();
}

VegaFemSceneObject::~VegaFemSceneObject()
{
}

std::shared_ptr<SceneObject> VegaFemSceneObject::clone()
{
    return safeDownCast<SceneObject>();
}

bool VegaFemSceneObject::configure(const std::string &ConfigFile)
{
    femConfig = std::make_shared<VegaObjectConfig>();

    if (ConfigFile.compare("__none") != 0)
    {
        femConfig->setFemObjConfuguration(ConfigFile, false);

        std::cout << "VEGA: Initialized the VegaFemSceneObject and configured using file-" <<
                                                               ConfigFile.c_str() << std::endl;

        return 1;
    }
    else
    {
        PRINT_ERROR_LOCATION
        std::cout << "VEGA: error! configuration file invalid!"
                  << ConfigFile.c_str() << std::endl;
        return 0;
    }
}

void VegaFemSceneObject::resetToInitialState()
{
    if (!topologyAltered)
    {
        performaceTracker.initialize();

        graphicFrame = 0;
        pulledVertex = -1;
        explosionFlag = 0;
        timestepCounter = 0;
        subTimestepCounter = 0;

        loadInitialStates();

        performaceTracker.clearFpsBuffer();
    }
    else
    {
        PRINT_ERROR_LOCATION
        std::cout << "VEGA: error! Initial state undetermined as the topology is altered.\n";
    }
}

void VegaFemSceneObject::initialize()
{
    setDeformableModelType();
    loadVolumeMesh();
    loadSurfaceMesh();

    loadFixedBC();

    // make room for deformation and force vectors
    // TODO: Clean these variables
    u.resize(3 * numNodes);
    uvel.resize(3 * numNodes);
    uaccel.resize(3 * numNodes);
    f_extBase.resize(3 * numNodes);
    f_ext.resize(3 * numNodes);

    loadInitialStates();
    loadScriptedExternalForces();
    createForceModel();
    initializeTimeIntegrator();

    performaceTracker.clearFpsBuffer();
    performaceTracker.objectPerformanceCounter.StartCounter();

    std::cout << "Inititializing fem object done. \n";
}

void VegaFemSceneObject::setDeformableModelType()
{

    // set deformable material type
    if (strcmp(femConfig->volumetricMeshFilename, "__none") != 0)
    {
        if (strcmp(femConfig->deformableObjectMethod, "StVK") == 0)
        {
            femConfig->deformableObject = VegaObjectConfig::STVK;
        }

        if (strcmp(femConfig->deformableObjectMethod, "CLFEM") == 0)
        {
            femConfig->deformableObject = VegaObjectConfig::COROTLINFEM;
        }

        if (strcmp(femConfig->deformableObjectMethod, "LinearFEM") == 0)
        {
            femConfig->deformableObject = VegaObjectConfig::LINFEM;
        }

        if (strcmp(femConfig->deformableObjectMethod, "InvertibleFEM") == 0)
        {
            femConfig->deformableObject = VegaObjectConfig::INVERTIBLEFEM;
        }
    }

    if (femConfig->deformableObject == VegaObjectConfig::UNSPECIFIED)
    {
        PRINT_ERROR_LOCATION
        std::cout << "VEGA: error! no deformable model specified." << std::endl;
    }
}

void VegaFemSceneObject::loadVolumeMesh()
{
    // load mesh
    if ((femConfig->deformableObject == VegaObjectConfig::STVK) ||
        (femConfig->deformableObject == VegaObjectConfig::COROTLINFEM) ||
        (femConfig->deformableObject == VegaObjectConfig::LINFEM) ||
        (femConfig->deformableObject == VegaObjectConfig::INVERTIBLEFEM))
    {

        std::cout << "VEGA: Loading volumetric mesh from file "
                  << femConfig->volumetricMeshFilename << "..." << std::endl;

        auto meshModel = std::make_shared<MeshModel>();

        meshModel->load(femConfig->volumetricMeshFilename);

        this->physicsModel = meshModel;

        this->volumetricMesh = std::static_pointer_cast<VegaVolumetricMesh>(this->physicsModel->getMesh());

        if(!this->volumetricMesh)
        {
            // TODO: Print error message and log
            return;
        }

        if (!this->volumetricMesh)
        {
            PRINT_ERROR_LOCATION
            std::cout << "VEGA: error! unable to load the volumetric mesh from"
                                        << femConfig->volumetricMeshFilename << std::endl;
            return;
        }

        numNodes = this->volumetricMesh->getNumberOfVertices();
        std::cout << "VEGA: Num vertices: " << this->volumetricMesh->getNumberOfVertices() <<
                         ". Num elements: " << this->volumetricMesh->getNumberOfElements() << std::endl;

        // load mass matrix
        if (strcmp(femConfig->massMatrixFilename, "__none") == 0)
        {
            std::string massMatFilenameStr(femConfig->volumetricMeshFilename);
            massMatFilenameStr += ".mass";
            std::ifstream massMatFileName(massMatFilenameStr);

            if (!massMatFileName.good())
            {
                std::cout << "VEGA: mass matrix file was not specified! Generating mass matrix file. \n";

                SparseMatrix *tempMassMatrix;
                GenerateMassMatrix::computeMassMatrix(this->volumetricMesh->getVegaMesh().get(), &tempMassMatrix, false);

                std::cout << femConfig->massMatrixFilename << std::endl;
                if (tempMassMatrix->Save(femConfig->massMatrixFilename) != 0)
                {
                    PRINT_ERROR_LOCATION
                    std::cout << "VEGA:  error saving mass matrix"
                        << femConfig->massMatrixFilename << std::endl;
                }

                delete tempMassMatrix;
            }
        }

        std::cout << "VEGA: Loading the mass matrix from file " << femConfig->massMatrixFilename <<
                                                                                 "..." << std::endl;
        // get the mass matrix
        std::shared_ptr<SparseMatrixOutline> massMatrixOutline = std::make_shared<SparseMatrixOutline>(femConfig->massMatrixFilename, 3);
        massMatrix = std::make_shared<SparseMatrix>(massMatrixOutline.get());

        if (femConfig->deformableObject == VegaObjectConfig::STVK ||
            femConfig->deformableObject == VegaObjectConfig::LINFEM)
        {

            unsigned int loadingFlag = 0; // 0 = use low-memory version, 1 = use high-memory version
            auto precomputedIntegrals = StVKElementABCDLoader::load(this->volumetricMesh->getVegaMesh().get(), loadingFlag);

            if (precomputedIntegrals == nullptr)
            {
                PRINT_ERROR_LOCATION
                std::cout << "VEGA: error! unable to load the StVK integrals.\n";
            }

            std::cout << "VEGA: Generating internal forces and stiffness matrix models...\n";
            fflush(nullptr);

            if (femConfig->numInternalForceThreads == 0)
            {
                stVKInternalForces = std::make_shared<StVKInternalForces>(
                    this->volumetricMesh->getVegaMesh().get(),
                    precomputedIntegrals,
                    femConfig->addGravity,
                    femConfig->g);
                stVKStiffnessMatrix = std::make_shared<StVKStiffnessMatrix>(stVKInternalForces.get());
            }
            else
            {
                stVKInternalForces = std::make_shared<StVKInternalForcesMT>(
                    this->volumetricMesh->getVegaMesh().get(),
                    precomputedIntegrals,
                    femConfig->addGravity,
                    femConfig->g,
                    femConfig->numInternalForceThreads);
                stVKStiffnessMatrix = std::make_shared<StVKStiffnessMatrixMT>(
                    stVKInternalForces.get(),
                    femConfig->numInternalForceThreads);
            }
        }
    }

    auto meshGraph = this->volumetricMesh->getMeshGraph();
    if (meshGraph)
    {
        int scaleRows = 1;
        SparseMatrix *sm;
        meshGraph->GetLaplacian(&sm, scaleRows);
        LaplacianDampingMatrix.reset(sm);
        LaplacianDampingMatrix->ScalarMultiply(femConfig->dampingLaplacianCoef);
    }
}

void VegaFemSceneObject::loadSurfaceMesh()
{
    // initialize the embedded triangle rendering mesh

    if (strcmp(femConfig->secondaryRenderingMeshFilename, "__none") != 0)
    {
        auto ioMesh = std::make_shared<IOMesh>();
        ioMesh->read(femConfig->secondaryRenderingMeshFilename);

        if (ioMesh->getMesh() == nullptr)
        {
            std::cout << "VEGA: Secondary rendering mesh is not initialized!\n";
            throw std::runtime_error("No secondary rendering mesh provided.");
        }
        else
        {
            std::cout << "VEGA: Secondary rendering mesh is initialized:\n\t\t"
                << ioMesh->getMesh()->getNumberOfVertices() << " vertices\n\t\t"
                << ioMesh->getMesh()->getTriangles().size() << " faces\n";
        }
        auto surfaceMesh = std::static_pointer_cast<SurfaceMesh>(ioMesh->getMesh());
        surfaceMesh->updateInitialVertices();

        // load interpolation structure
        if (strcmp(femConfig->secondaryRenderingMeshInterpolationFilename, "__none") == 0)
        {
            std::cerr << "VEGA:  error! no secondary rendering mesh interpolation filename specified." << std::endl;
            std::cerr << "VEGA:  error! weighs will be computed. Slow operation." << std::endl;
            this->volumetricMesh->attachSurfaceMesh(surfaceMesh);
            auto meshModel = std::make_shared<MeshModel>();
            meshModel->setModelMesh(surfaceMesh);
            this->visualModel = meshModel;
        }
        else
        {
            this->volumetricMesh->attachSurfaceMesh(surfaceMesh,
                                                femConfig->secondaryRenderingMeshInterpolationFilename);
            auto meshModel = std::make_shared<MeshModel>();
            meshModel->setModelMesh(surfaceMesh);
            this->visualModel = meshModel;
        }
    }

}

int VegaFemSceneObject::readBcFromFile(const char* filename, const int offset)
{
    // comma-separated text file of fixed vertices
    FILE * fin;
    fin = fopen(filename, "r");
    if (!fin)
    {
        PRINT_ERROR_LOCATION
        std::cout << "Error: could not open file "<< filename << ".\n";
        return 1;
    }

    int numFixed = 0;

    char s[4096];
    while (fgets(s, 4096, fin) != nullptr)
    {
        LoadList::stripBlanks(s);

        char * pch;
        pch = strtok(s, ",");
        while ((pch != nullptr) && (isdigit(*pch)))
        {
            numFixed++;
            pch = strtok(nullptr, ",");
        }
    }

    fixedVertices.resize(numFixed);

    rewind(fin);

    numFixed = 0;

    while (fgets(s, 4096, fin) != nullptr)
    {
        LoadList::stripBlanks(s);
        char * pch;
        pch = strtok(s, ",");
        while ((pch != nullptr) && (isdigit(*pch)))
        {
            fixedVertices[numFixed] = atoi(pch) - offset;
            numFixed++;
            pch = strtok(nullptr, ",");
        }
    }

    fclose(fin);

    return 0;
}

void VegaFemSceneObject::loadFixedBC()
{
    // read the fixed vertices
    // 1-indexed notation
    if (
      femConfig->fixedVerticesFilename[0] &&
      strcmp(femConfig->fixedVerticesFilename, "__none") != 0)
    {
        // set the offset to 1 because nodes are numbered from 1 in .bou file
        if (readBcFromFile(femConfig->fixedVerticesFilename, 1) != 0)
        {
            PRINT_ERROR_LOCATION
            throw std::logic_error("VEGA: error! reading fixed vertices.");
        }
        numFixedNodes = fixedVertices.size();

        // sort the list
        LoadList::sort(numFixedNodes, fixedVertices.data());
    }

    std::cout << "VEGA: Loaded " << numFixedNodes << " fixed vertices. They are : \n";
    LoadList::print(numFixedNodes, fixedVertices.data());

    numTotalDOF = 3 * numNodes;
    numFixedDof = 3 * numFixedNodes;
    numDOF = numTotalDOF - numFixedDof;

    std::cout << "VEGA: Fixed boundary vertices loaded.\n";
}

void VegaFemSceneObject::loadInitialStates()
{
    // load initial condition
    if (strcmp(femConfig->initialPositionFilename, "__none") != 0)
    {
        int m1, n1;

        auto aInitialPointer = uInitial.data();
        ReadMatrixFromDisk_(femConfig->initialPositionFilename, &m1, &n1, &aInitialPointer);

        if ((m1 != 3 * numNodes) || (n1 != 1))
        {
            PRINT_ERROR_LOCATION
            std::cout << "VEGA: error! initial position matrix size mismatch.\n";
        }
    }
    else
    {
        uInitial.resize(3 * numNodes);
    }

    // load initial velocity
    if (strcmp(femConfig->initialVelocityFilename, "__none") != 0)
    {
        int m1, n1;
        auto velInitialPointer = uInitial.data();
        ReadMatrixFromDisk_(femConfig->initialVelocityFilename, &m1, &n1, &velInitialPointer);

        if ((m1 != 3 * numNodes) || (n1 != 1))
        {
            PRINT_ERROR_LOCATION
            std::cout << "VEGA: error! initial position matrix size mismatch.\n";
        }
    }
}

void VegaFemSceneObject::loadScriptedExternalForces()
{
    // load force loads
    int numForceLoads = forceLoads.size();
    if (strcmp(femConfig->forceLoadsFilename, "__none") != 0)
    {
        int m1;
        double *data = forceLoads.data();
        ReadMatrixFromDisk_(femConfig->forceLoadsFilename, &m1, &numForceLoads, &data);

        if (m1 != 3 * numNodes)
        {
            PRINT_ERROR_LOCATION
            std::cout << "VEGA: error! Mismatch in the dimension of the force load matrix.\n";
        }
    }
}

void VegaFemSceneObject::initializeTimeIntegrator()
{

    int numFixedDOFs = 3 * numFixedNodes;
    std::vector<int> fixedDOFs(numFixedDOFs, 0);

    // initialize the integrator
    std::cout << "VEGA: Initializing the integrator, n = " << numNodes << "...\n";
    std::cout << "VEGA: Solver type - " << femConfig->solverMethod << std::endl;

    integratorBaseSparse = nullptr;

    if (femConfig->solver == VegaObjectConfig::IMPLICITNEWMARK)
    {
        implicitNewmarkSparse = std::make_shared<ImplicitNewmarkSparse>(
            3 * numNodes,
            femConfig->timeStep,
            massMatrix.get(),
            forceModel.get(),
            positiveDefinite,
            numFixedDOFs,
            fixedDOFs.data(),
            femConfig->dampingMassCoef,
            femConfig->dampingStiffnessCoef,
            femConfig->maxIterations,
            femConfig->epsilon,
            femConfig->newmarkBeta,
            femConfig->newmarkGamma,
            femConfig->numSolverThreads
            );

        integratorBaseSparse = implicitNewmarkSparse;
    }
    else if (femConfig->solver == VegaObjectConfig::IMPLICITBACKWARDEULER)
    {
        implicitNewmarkSparse = std::make_shared<ImplicitBackwardEulerSparse>(
            3 * numNodes,
            femConfig->timeStep,
            massMatrix.get(),
            forceModel.get(),
            positiveDefinite,
            numFixedDOFs,
            fixedDOFs.data(),
            femConfig->dampingMassCoef,
            femConfig->dampingStiffnessCoef,
            femConfig->maxIterations,
            femConfig->epsilon,
            femConfig->numSolverThreads);

        integratorBaseSparse = implicitNewmarkSparse;
    }
    else if (femConfig->solver == VegaObjectConfig::EULER)
    {
        int symplectic = 0;
        integratorBaseSparse = std::make_shared<EulerSparse>(
            3 * numNodes,
            femConfig->timeStep,
            massMatrix.get(),
            forceModel.get(),
            symplectic,
            numFixedDOFs,
            fixedDOFs.data(),
            femConfig->dampingMassCoef
            );
    }
    else if (femConfig->solver == VegaObjectConfig::SYMPLECTICEULER)
    {
        int symplectic = 1;
        integratorBaseSparse = std::make_shared<EulerSparse>(
            3 * numNodes,
            femConfig->timeStep,
            massMatrix.get(),
            forceModel.get(),
            symplectic,
            numFixedDOFs,
            fixedDOFs.data(),
            femConfig->dampingMassCoef
            );
    }
    else if (femConfig->solver == VegaObjectConfig::CENTRALDIFFERENCES)
    {
        integratorBaseSparse = std::make_shared<CentralDifferencesSparse>(
            3 * numNodes,
            femConfig->timeStep,
            massMatrix.get(),
            forceModel.get(),
            numFixedDOFs,
            fixedDOFs.data(),
            femConfig->dampingMassCoef,
            femConfig->dampingStiffnessCoef,
            femConfig->centralDifferencesTangentialDampingUpdateMode,
            femConfig->numSolverThreads
            );
    }

    //LinearSolver *linearSolver = new CGSolver(integratorBaseSparse->GetSystemMatrix());

    linearSolver = std::make_shared<CGSolver>(integratorBaseSparse->GetSystemMatrix());
    integratorBaseSparse->setLinearSolver(linearSolver.get());
    integratorBase = integratorBaseSparse;

    if (integratorBase == nullptr)
    {
        PRINT_ERROR_LOCATION
        std::cout << "VEGA: error! failed to initialize numerical integrator.\n";
    }

    // set integration parameters
    integratorBaseSparse->SetDampingMatrix(LaplacianDampingMatrix.get());
    integratorBase->ResetToRest();
    integratorBase->SetState(uInitial.data(), velInitial.data());
    integratorBase->SetTimestep(femConfig->timeStep / femConfig->substepsPerTimeStep);

    if (implicitNewmarkSparse != nullptr)
    {
        implicitNewmarkSparse->UseStaticSolver(staticSolver);

        if (velInitial.data() != nullptr)
        {
            implicitNewmarkSparse->SetState(implicitNewmarkSparse->Getq(), velInitial.data());
        }
    }
}

void VegaFemSceneObject::createForceModel()
{

    // create force models, to be used by the integrator
    std::cout << "VEGA: Creating force models..." << std::endl;

    switch (femConfig->deformableObject)
    {
    case VegaObjectConfig::STVK:
    {

        auto stVKForceModel = std::make_shared<StVKForceModel>(stVKInternalForces.get(),
                                                          stVKStiffnessMatrix.get());
        stVKForceModel->GetInternalForce(uInitial.data(), u.data());
        forceModel = stVKForceModel;

        break;
    }
    case VegaObjectConfig::COROTLINFEM:
    {
        TetMesh * tetMesh = dynamic_cast<TetMesh*>(volumetricMesh->getVegaMesh().get());

        if (tetMesh == nullptr)
        {
            PRINT_ERROR_LOCATION
            std::cout << "VEGA: error! the input mesh is not a tet mesh (CLFEM deformable model).\n";
        }

        CorotationalLinearFEM * corotationalLinearFEM;

        if (femConfig->numInternalForceThreads == 0)
        {
            corotationalLinearFEM = new CorotationalLinearFEM(tetMesh);
        }
        else
        {
            corotationalLinearFEM = new CorotationalLinearFEMMT(tetMesh, femConfig->numInternalForceThreads);
        }

        auto corotationalLinearFEMForceModel = std::make_shared<CorotationalLinearFEMForceModel>(
                                            corotationalLinearFEM, femConfig->corotationalLinearFEM_warp);
        forceModel = corotationalLinearFEMForceModel;

        break;
    }
    case VegaObjectConfig::LINFEM:
    {
        auto linearFEMForceModel =
                    std::make_shared<LinearFEMForceModel>(stVKInternalForces.get());
        forceModel = linearFEMForceModel;

        break;
    }
    case VegaObjectConfig::INVERTIBLEFEM:
    {
        std::shared_ptr<TetMesh> tetMesh = std::static_pointer_cast<TetMesh>(volumetricMesh->getVegaMesh());

        if (tetMesh == nullptr)
        {
            PRINT_ERROR_LOCATION
            std::cout << "VEGA: error! the input mesh is not a tet mesh (Invertible FEM deformable model).\n";
        }

        IsotropicMaterial * isotropicMaterial = nullptr;

        // create the invertible material model
        if (strcmp(femConfig->invertibleMaterialString, "StVK") == 0)
        {
            femConfig->invertibleMaterial = femConfig->invertibleMaterialType::INV_STVK;
        }

        if (strcmp(femConfig->invertibleMaterialString, "neoHookean") == 0)
        {
            femConfig->invertibleMaterial = femConfig->invertibleMaterialType::INV_NEOHOOKEAN;
        }

        if (strcmp(femConfig->invertibleMaterialString, "MooneyRivlin") == 0)
        {
            femConfig->invertibleMaterial = femConfig->invertibleMaterialType::INV_MOONEYRIVLIN;
        }

        switch (femConfig->invertibleMaterial)
        {
        case VegaObjectConfig::INV_STVK:
        {

            isotropicMaterial = new StVKIsotropicMaterial(
                tetMesh.get(),
                femConfig->enableCompressionResistance,
                femConfig->compressionResistance);
            std::cout << "VEGA: Invertible material: StVK.\n";
            break;
        }

        case VegaObjectConfig::INV_NEOHOOKEAN:
            isotropicMaterial = new NeoHookeanIsotropicMaterial(
                tetMesh.get(),
                femConfig->enableCompressionResistance,
                femConfig->compressionResistance);
            std::cout << "VEGA: Invertible material: neo-Hookean.\n";
            break;

        case VegaObjectConfig::INV_MOONEYRIVLIN:
            isotropicMaterial = new MooneyRivlinIsotropicMaterial(
                tetMesh.get(),
                femConfig->enableCompressionResistance,
                femConfig->compressionResistance);
            std::cout << "VEGA: Invertible material: Mooney-Rivlin.\n";
            break;

        default:
            PRINT_ERROR_LOCATION
            std::cout << "VEGA: error! invalid invertible material type.\n";
            break;
        }

        // create the invertible FEM deformable model
        IsotropicHyperelasticFEM * isotropicHyperelasticFEM;
        if (femConfig->numInternalForceThreads == 0)
        {
            isotropicHyperelasticFEM = new IsotropicHyperelasticFEM(
                tetMesh.get(), isotropicMaterial,
                femConfig->inversionThreshold,
                femConfig->addGravity, femConfig->g);
        }
        else
        {
            isotropicHyperelasticFEM = new IsotropicHyperelasticFEMMT(
                tetMesh.get(),
                isotropicMaterial,
                femConfig->inversionThreshold,
                femConfig->addGravity,
                femConfig->g,
                femConfig->numInternalForceThreads
                );
        }
        // create force model for the invertible FEM class
        auto isotropicHyperelasticFEMForceModel =
                       std::make_shared<IsotropicHyperelasticFEMForceModel>(isotropicHyperelasticFEM);

        forceModel = isotropicHyperelasticFEMForceModel;

        break;
    }
    default:
    {
        PRINT_ERROR_LOCATION
        std::cout <<  "VEGA: error! Scene object is not of FE type!" << std::endl;
        break;
    }
    }

}

void VegaFemSceneObject::update(double /*dt*/)
{
    int i;

    // reset external forces (usually to zero)
    for (i = 0; i < 3 * numNodes; i++)
    {
        f_ext[i] = f_extBase[i];
    }

    if (femConfig->singleStepMode <= 1)
    {
        // apply external user interaction forces
        applyUserInteractionForces();

        // apply pre-defined external forces
        //applyScriptedExternalForces();

        // apply external forces arising from contact
        applyContactForces();

        // set forces to the integrator
        integratorBaseSparse->SetExternalForces(f_ext.data());

        // timestep the dynamics
        advanceOneTimeStep();

        timestepCounter++;

        double *q = integratorBase->Getq();
        for (i = 0; i < 3 * numNodes; i++)
        {
            u[i] = q[i];
        }
        std::cout << core::Matrixd::Map(q,3,numNodes) << std::endl;
        std::cout << std::endl;
        std::cout << std::endl;
        auto positions = core::Vectord::Map(q,3*numNodes);

        this->volumetricMesh->updateAttachedMeshes(positions);

        if (femConfig->singleStepMode == 1)
        {
            femConfig->singleStepMode = 2;
        }

        graphicFrame++;

        if (femConfig->lockAt30Hz)
        {
            double elapsedTime;

            do
            {
                performaceTracker.objectPerformanceCounter.StopCounter();
                elapsedTime = performaceTracker.objectPerformanceCounter.GetElapsedTime();
            } while (1.0 * graphicFrame / elapsedTime >= 30.0);
        }
    }

    // update stasts
    updatePerformanceMetrics();
}

inline void VegaFemSceneObject::advanceOneTimeStep()
{
    VegaPerformanceCounter *pt =  &performaceTracker;
    for (int i = 0; i < femConfig->substepsPerTimeStep; i++)
    {
        int code = integratorBase->DoTimestep();

        pt->forceAssemblyLocalTime = integratorBaseSparse->GetForceAssemblyTime();
        pt->systemSolveLocalTime = integratorBaseSparse->GetSystemSolveTime();

        // average forceAssemblyTime over last "forceAssemblyBufferSize" samples
        pt->forceAssemblyTime += 1.0 / pt->forceAssemblyBufferSize *
            (pt->forceAssemblyLocalTime - pt->forceAssemblyBuffer[pt->forceAssemblyHead]);
        pt->forceAssemblyBuffer[pt->forceAssemblyHead] = pt->forceAssemblyLocalTime;
        pt->forceAssemblyHead = (pt->forceAssemblyHead + 1) % pt->forceAssemblyBufferSize;

        // average systemSolveTime over last "systemSolveBufferSize" samples
        pt->systemSolveTime += 1.0 / pt->systemSolveBufferSize *
            (pt->systemSolveLocalTime - pt->systemSolveBuffer[pt->systemSolveHead]);
        pt->systemSolveBuffer[pt->systemSolveHead] = pt->systemSolveLocalTime;
        pt->systemSolveHead = (pt->systemSolveHead + 1) % pt->systemSolveBufferSize;

        if (code != 0)
        {
            std::cout << "VEGA: The integrator went unstable."
                <<"Reduce the timestep, or increase the number of substeps per timestep.\n";
            integratorBase->ResetToRest();

            for (int i = 0, end = 3*numNodes; i < end; ++i)
            {
                f_ext[i] = 0;
                f_extBase[i] = 0;
            }

            integratorBase->SetExternalForcesToZero();
            explosionFlag = 1;
            pt->explosionCounter.StartCounter();
            break;
        }

        // update UI performance indicators
        if (strcmp(femConfig->outputFilename, "__none") != 0)
        {
            char s[4096];
            sprintf(s, "%s.u.%04d", femConfig->outputFilename, subTimestepCounter);
            std::cout << "VEGA: Saving deformation to " << s << ".\n";
            WriteMatrixToDisk_(s, 3 * numNodes, 1, integratorBase->Getq());
            sprintf(s, "%s.f.%04d", femConfig->outputFilename, subTimestepCounter);
            std::cout << "VEGA: Saving forces to " << s << ".\n";
            WriteMatrixToDisk_(s, 3 * numNodes, 1, integratorBase->GetExternalForces());
        }

        subTimestepCounter++;
    }
}

void VegaFemSceneObject::setPulledVertex(const core::Vec3d &userPos)
{
    Vec3d position(userPos(0), userPos(1), userPos(2));
    pulledVertex = this->volumetricMesh->getVegaMesh()->getClosestVertex(position);
}

inline void VegaFemSceneObject::applyUserInteractionForces()
{
    // determine force in case user is pulling on a vertex
    if (pulledVertex != -1)
    {
        auto meshGraph = this->volumetricMesh->getMeshGraph();
        if(!meshGraph)
        {
            std::cerr << "Mesh graph has not being set.\n";
            return;
        }

        double externalForce[3];

        externalForce[0] = 0.0;
        externalForce[1] = 0.5;
        externalForce[2] = 0.0;

        for (int j = 0; j < 3; j++)
        {
            externalForce[j] *= femConfig->deformableObjectCompliance;
        }

        // register force on the pulled vertex
        f_ext[3 * pulledVertex + 0] += externalForce[0];
        f_ext[3 * pulledVertex + 1] += externalForce[1];
        f_ext[3 * pulledVertex + 2] += externalForce[2];

        // distribute force over the neighboring vertices
        std::set<int> affectedVertices;
        std::set<int> lastLayerVertices;

        affectedVertices.insert(pulledVertex);
        lastLayerVertices.insert(pulledVertex);

        for (int j = 1; j < femConfig->forceNeighborhoodSize; j++)
        {
            // linear kernel
            double forceMagnitude = 1.0 * (femConfig->forceNeighborhoodSize - j) / femConfig->forceNeighborhoodSize;

            std::set<int> newAffectedVertices;

            for (std::set<int> ::iterator iter = lastLayerVertices.begin(); iter != lastLayerVertices.end(); iter++)
            {
                // traverse all neighbors and check if they were already previously inserted
                int vtx = *iter;
                int deg = meshGraph->GetNumNeighbors(vtx);

                for (int k = 0; k < deg; k++)
                {
                    int vtxNeighbor = meshGraph->GetNeighbor(vtx, k);

                    if (affectedVertices.find(vtxNeighbor) == affectedVertices.end())
                    {
                        // discovered new vertex
                        newAffectedVertices.insert(vtxNeighbor);
                    }
                }
            }

            lastLayerVertices.clear();

            for(const auto &v : newAffectedVertices)
            for (std::set<int> ::iterator iter = newAffectedVertices.begin(); iter != newAffectedVertices.end(); iter++)
            {
                // apply force
                f_ext[3*v+0] += forceMagnitude * externalForce[0];
                f_ext[3*v+1] += forceMagnitude * externalForce[1];
                f_ext[3*v+2] += forceMagnitude * externalForce[2];

                // generate new layers
                lastLayerVertices.insert(v);
                affectedVertices.insert(v);
            }
        }
    }
}

inline void VegaFemSceneObject::applyScriptedExternalForces()
{
    // apply any scripted force loads
    if (timestepCounter < this->forceLoads.size())
    {
        std::cout << "External forces read from the binary input file.\n";

        size_t offset = 3 * numNodes*timestepCounter;
        for (size_t i = 0, end = 3u * numNodes; i < end; i++)
        {
            f_ext[i] += forceLoads[offset + i];
        }
    }
}

inline void VegaFemSceneObject::updateSecondaryRenderingMesh()
{

}

inline void VegaFemSceneObject::updatePerformanceMetrics()
{
    VegaPerformanceCounter *pt = &performaceTracker;
    //update window title at 5 Hz
    pt->objectPerformanceCounter.StopCounter();
    double elapsedTime = pt->objectPerformanceCounter.GetElapsedTime();

    if (elapsedTime >= 1.0 / 5)
    {
        pt->objectPerformanceCounter.StartCounter();
        double fpsLocal = graphicFrame / elapsedTime;

        // average fps over last "fpsBufferSize" samples
        pt->fps += 1.0 / pt->fpsBufferSize * (fpsLocal - pt->fpsBuffer[pt->fpsHead]);
        pt->fpsBuffer[pt->fpsHead] = fpsLocal;
        pt->fpsHead = (pt->fpsHead + 1) % pt->fpsBufferSize;

        graphicFrame = 0;

        if (femConfig->syncTimestepWithGraphics && femConfig->singleStepMode == 0)
        {
            femConfig->timeStep = 1.0 / pt->fps;
            integratorBase->SetTimestep(femConfig->timeStep / femConfig->substepsPerTimeStep);
        }
    }
    //std::cout << "FPS: " << pt->fps << std::endl;
}

void VegaFemSceneObject::printInfo() const
{
    std::cout << "\t-------------------------------------\n";
    std::cout << "\t Name        : " << this->getName() << std::endl;
    std::cout << "\t Model       : " << "--"  << std::endl;
    std::cout << "\t # Nodes     : " << this->getNumNodes() << std::endl;
    std::cout << "\t # TotalDOF  : " << this->getNumDof() << std::endl;
    std::cout << "\t-------------------------------------\n";
}

core::Vec3d VegaFemSceneObject::getDisplacementOfNodeWithDofID(const int dofID) const
{
    core::Vec3d disp(u[dofID], u[dofID + 1], u[dofID + 2]);

    return disp;
}

core::Vec3d VegaFemSceneObject::getAccelerationOfNodeWithDofID(const int dofID) const
{
    core::Vec3d accn(uaccel[dofID], uaccel[dofID + 1], uaccel[dofID + 2]);

    return accn;
}

int VegaFemSceneObject::getNumNodes() const
{
    return numNodes;
}

int VegaFemSceneObject::getNumTotalDof() const
{
    return numTotalDOF;
}

int VegaFemSceneObject::getNumDof() const
{
    return numDOF;
}

int VegaFemSceneObject::getNumFixedNodes() const
{
    return numFixedNodes;
}

int VegaFemSceneObject::getNumFixedDof() const
{
    return numFixedDof;
}
std::shared_ptr< VegaVolumetricMesh > VegaFemSceneObject::getVolumetricMesh()
{
    return this->volumetricMesh;
}
