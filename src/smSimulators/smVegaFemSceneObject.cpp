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
#include "smSimulators/smVegaFemSceneObject.h"

smVegaFemSceneObject::smVegaFemSceneObject()
{
    setDefaults();
}

smVegaFemSceneObject::smVegaFemSceneObject(std::shared_ptr<smErrorLog> p_log, smString ConfigFile)
{

    setDefaults();

    ConfigFileName = ConfigFile;

    if (ConfigFile.compare(vega_string_none) != 0)
    {
        femConfig = std::make_shared<smVegaConfigFemObject>();
        femConfig->setFemObjConfuguration(ConfigFile);
        std::cout << "VEGA: Initialized the VegaFemSceneObject and configured using file-" <<
                                                                            ConfigFile.c_str() << std::endl;

        initSimulation();
    }
    else
    {
        std::cout << "VEGA: NOTE-Initialized the VegaFemSceneObject, but not configured yet!\n";
    }
}

void smVegaFemSceneObject::setDefaults()
{
    
    performaceTracker.initialize();

    enableTextures = 0;
    staticSolver = 0;
    graphicFrame = 0;
    pulledVertex = -1;
    explosionFlag = 0;
    timestepCounter = 0;
    subTimestepCounter = 0;
    integratorBase = nullptr;
    implicitNewmarkSparse = nullptr;
    integratorBaseSparse = nullptr;
    forceModel = nullptr;
    stVKInternalForces = nullptr;
    stVKStiffnessMatrix = nullptr;
    stVKForceModel = nullptr;
    corotationalLinearFEMForceModel = nullptr;
    positiveDefinite = 0;
    topologyAltered = false;

    volumetricMesh = nullptr;
    tetMesh = nullptr;
    meshGraph = nullptr;

    massMatrix = nullptr;
    LaplacianDampingMatrix = nullptr;

    deformableObjectRenderingMesh = nullptr;
    secondaryDeformableObjectRenderingMesh = nullptr;

    secondaryDeformableObjectRenderingMesh_interpolation_vertices = nullptr;
    secondaryDeformableObjectRenderingMesh_interpolation_weights = nullptr;

    renderUsingVega = false;
    importAndUpdateVolumeMeshToSmtk = false;

    type = SIMMEDTK_SMVEGAFEMSCENEOBJECT;
}


bool smVegaFemSceneObject::configure(smString ConfigFile)
{
    femConfig = std::make_shared<smVegaConfigFemObject>();

    if (ConfigFile.compare(vega_string_none) != 0)
    {
        femConfig->setFemObjConfuguration(ConfigFile);

        std::cout << "VEGA: Initialized the VegaFemSceneObject and configured using file-" <<
                                                                        ConfigFile.c_str() << std::endl;

        initSimulation();

        return 1;
    }
    else
    {
        std::cout << "VEGA: Configured file invalid!" << ConfigFile.c_str() << std::endl;
        return 0;
    }
}

void smVegaFemSceneObject::duplicateAtInitialization(std::shared_ptr<smVegaFemSceneObject> &newSO)
{
    if (newSO == nullptr)
    {
        newSO = std::make_shared<smVegaFemSceneObject>(nullptr, this->ConfigFileName);
    }
}

// WIP
void smVegaFemSceneObject::duplicateAtRuntime(std::shared_ptr<smVegaFemSceneObject> &newSO)
{
    //if(newSO==nullptr)
    //{
    //    newSO = std::make_shared<smVegaFemSceneObject>();
    //}
    //else
    //{
    //    newSO->setDefaults();
    //}

    //// Copy the config settings
    //newSO->femConfig = std::make_shared<smVegaConfigFemObject>(*this->femConfig);
    //newSO->femConfig->deformableObject = this->femConfig->deformableObject;

    //newSO->volumetricMesh = std::make_shared<VolumetricMesh>(*this->volumetricMesh.get());

    //newSO->n = this->volumetricMesh->getNumVertices();
    //newSO->meshGraph = std::make_shared<Graph>(*this->meshGraph);

    //// load mass matrix
    ////newSO->massMatrixOutline;

    //newSO->massMatrix = std::make_shared<SparseMatrix>(*this->massMatrix.get());

    //if (this->femConfig->deformableObject == STVK || this->femConfig->deformableObject == LINFEM)  //LINFEM constructed from stVKInternalForces{
    //{

    //    unsigned int loadingFlag = 0; // 0 = use low-memory version, 1 = use high-memory version
    //    std::shared_ptr<StVKElementABCD> precomputedIntegrals = std::shared_ptr<StVKElementABCD>(StVKElementABCDLoader::load(volumetricMesh.get(), loadingFlag));

    //    if (precomputedIntegrals == nullptr)
    //    {
    //        printf("VEGA Error: unable to load the StVK integrals.\n");
    //        exit(1);
    //    }

    //    printf("VEGA: Generating internal forces and stiffness matrix models...\n");
    //    fflush(nullptr);

    //    if (femConfig->numInternalForceThreads == 0)
    //        stVKInternalForces = std::make_shared<StVKInternalForces>(volumetricMesh.get(),
    //        precomputedIntegrals.get(),
    //        femConfig->addGravity,
    //        femConfig->g);
    //    else
    //        stVKInternalForces = std::make_shared<StVKInternalForcesMT>(volumetricMesh.get(),
    //        precomputedIntegrals.get(), femConfig->addGravity,
    //        femConfig->g, femConfig->numInternalForceThreads);

    //    if (femConfig->numInternalForceThreads == 0)
    //    {
    //        stVKStiffnessMatrix = std::make_shared<StVKStiffnessMatrix>(stVKInternalForces.get());
    //    }
    //    else
    //        stVKStiffnessMatrix = std::make_shared<StVKStiffnessMatrixMT>(stVKInternalForces.get(),
    //        femConfig->numInternalForceThreads);
    //}
    //int scaleRows = 1;
    //SparseMatrix *sm;
    //meshGraph->GetLaplacian(&sm, scaleRows);
    //LaplacianDampingMatrix.reset(sm);
    //LaplacianDampingMatrix->ScalarMultiply(femConfig->dampingLaplacianCoef);


    //newSO->integratorBase = std::make_shared<IntegratorBase>(this->integratorBase->Getr(),
    //                                                        this->integratorBase->GetTimeStep(),
    //                                                        this->integratorBase->GetDampingMassCoef(),
    //                                                        this->integratorBase->GetDampingStiffnessCoef());

    //newSO->createForceModel();
    //newSO->initializeTimeIntegrator();

    //int numDof = 3 * this->volumetricMesh->getNumVertices;
    //// make room for deformation and force vectors
    //newSO->u.resize(numDof);
    //newSO->uvel.resize(numDof);
    //newSO->uaccel.resize(numDof);
    //newSO->f_extBase.resize(numDof);
    //newSO->f_ext.resize(numDof);
    //newSO->f_contact.resize(numDof);

    //// copy the states
    //newSO->u = this->u;
    //newSO->uvel = this->uvel;
    //newSO->uaccel = this->uaccel;
    //newSO->f_extBase = this->f_extBase;
    //newSO->f_ext = this->f_ext;
    //newSO->f_contact = this->f_contact;
}

void smVegaFemSceneObject::resetToInitialState()
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
        //volumetricMesh->Reset();//WIP no stored initial positions
        deformableObjectRenderingMesh->ResetDeformationToRest();

        performaceTracker.clearFpsBuffer();
    }
    else
    {
        std::cout << "VEGA: error! Initial state undetermined as the topology is altered.\n";
    }
}

// Initialize the parameters and properties of the simulation object
void smVegaFemSceneObject::initSimulation()
{

    volumetricMesh = nullptr;

    setDeformableModelType();
    loadVolumeMesh();
    loadSurfaceMesh();

    if (importAndUpdateVolumeMeshToSmtk)
    {
        this->smtkVolumeMesh = std::make_shared<smVolumeMesh>();
        this->smtkVolumeMesh->importVolumeMeshFromVegaFormat(this->volumetricMesh, true);
    }
    
    if(!renderUsingVega)
    {
        this->smtkSurfaceMesh = std::make_shared<smSurfaceMesh>();
        if (strcmp(femConfig->secondaryRenderingMeshFilename, "__none") == 0)
        {
            this->smtkSurfaceMesh->importSurfaceMeshFromVegaFormat(this->deformableObjectRenderingMesh->GetMesh(), true);
        }
        else
        {
            this->smtkSurfaceMesh->importSurfaceMeshFromVegaFormat(this->secondaryDeformableObjectRenderingMesh->GetMesh(), true);
        }

    }

    loadFixedBC();

    // make room for deformation and force vectors
    u.resize(3 * numNodes);
    uvel.resize(3 * numNodes);
    uaccel.resize(3 * numNodes);
    f_extBase.resize(3 * numNodes);
    f_ext.resize(3 * numNodes);
    f_contact.resize(3 * numNodes);

    loadInitialStates();
    loadScriptedExternalForces();
    createForceModel();
    initializeTimeIntegrator();

    performaceTracker.clearFpsBuffer();
    performaceTracker.titleBarCounter.StartCounter();

    std::cout << "Init simulator done. \n";

    smtkSurfaceMesh->printPrimitiveDetails();

    smtkSurfaceMesh->initVertexNeighbors();
    smtkSurfaceMesh->updateTriangleNormals();
    smtkSurfaceMesh->updateVertexNormals();
}


// Set the type of formulation used to model the deformation
void smVegaFemSceneObject::setDeformableModelType()
{

    // set deformable material type
    if (strcmp(femConfig->volumetricMeshFilename, "__none") != 0)
    {
        if (strcmp(femConfig->deformableObjectMethod, "StVK") == 0)
        {
            femConfig->deformableObject = STVK;
        }

        if (strcmp(femConfig->deformableObjectMethod, "CLFEM") == 0)
        {
            femConfig->deformableObject = COROTLINFEM;
        }

        if (strcmp(femConfig->deformableObjectMethod, "LinearFEM") == 0)
        {
            femConfig->deformableObject = LINFEM;
        }

        if (strcmp(femConfig->deformableObjectMethod, "InvertibleFEM") == 0)
        {
            femConfig->deformableObject = INVERTIBLEFEM;
        }
    }

    if (femConfig->deformableObject == UNSPECIFIED)
    {
        std::cout << "VEGA Error: no deformable model specified." << std::endl;
        exit(1);
    }
}

// Load specified meshes
void smVegaFemSceneObject::loadVolumeMesh()
{

    // load mesh
    if ((femConfig->deformableObject == STVK) || (femConfig->deformableObject == COROTLINFEM) ||
        (femConfig->deformableObject == LINFEM) || (femConfig->deformableObject == INVERTIBLEFEM))
    {

        std::cout << "VEGA: Loading volumetric mesh from file " << femConfig->volumetricMeshFilename << "..." << std::endl;

        volumetricMesh = std::shared_ptr<VolumetricMesh>(VolumetricMeshLoader::load(femConfig->volumetricMeshFilename));

        if (volumetricMesh == nullptr)
        {
            std::cout << "VEGA Error: unable to load the volumetric mesh from"
                                        << femConfig->volumetricMeshFilename << std::endl;
            exit(1);
        }

        numNodes = volumetricMesh->getNumVertices();
        std::cout << "VEGA: Num vertices: " << numNodes << ". Num elements: " << volumetricMesh->getNumElements() << std::endl;
        meshGraph = std::shared_ptr<Graph>(GenerateMeshGraph::Generate(volumetricMesh.get()));

        // load mass matrix
        if (strcmp(femConfig->massMatrixFilename, "__none") == 0)
        {
            std::cout << "VEGA Error: mass matrix for the StVK deformable model not specified" <<
                                                                            femConfig->massMatrixFilename << std::endl;
            exit(1);
        }

        std::cout << "VEGA: Loading the mass matrix from file " << femConfig->massMatrixFilename << "..." << std::endl;

        // get the mass matrix
        std::shared_ptr<SparseMatrixOutline> massMatrixOutline;

        try
        {
            // 3 is expansion flag to indicate this is a mass matrix; and does 3x3 identity block expansion
            massMatrixOutline = std::make_shared<SparseMatrixOutline>(femConfig->massMatrixFilename, 3);
        }
        catch (int exceptionCode)
        {
            std::cout << "VEGA: Error loading mass matrix" << femConfig->massMatrixFilename << std::endl;
            exit(1);
        }

        massMatrix = std::make_shared<SparseMatrix>(massMatrixOutline.get());

        if (femConfig->deformableObject == STVK || femConfig->deformableObject == LINFEM)  //LINFEM constructed from stVKInternalForces{
        {

            unsigned int loadingFlag = 0; // 0 = use low-memory version, 1 = use high-memory version
            std::shared_ptr<StVKElementABCD> precomputedIntegrals = std::shared_ptr<StVKElementABCD>(StVKElementABCDLoader::load(volumetricMesh.get(), loadingFlag));

            if (precomputedIntegrals == nullptr)
            {
                std::cout << "VEGA Error: unable to load the StVK integrals.\n";
                exit(1);
            }

            std::cout << "VEGA: Generating internal forces and stiffness matrix models...\n";
            fflush(nullptr);

            if (femConfig->numInternalForceThreads == 0)
                stVKInternalForces = std::make_shared<StVKInternalForces>(volumetricMesh.get(),
                precomputedIntegrals.get(),
                femConfig->addGravity,
                femConfig->g);
            else
                stVKInternalForces = std::make_shared<StVKInternalForcesMT>(volumetricMesh.get(),
                precomputedIntegrals.get(), femConfig->addGravity,
                femConfig->g, femConfig->numInternalForceThreads);

            if (femConfig->numInternalForceThreads == 0)
            {
                stVKStiffnessMatrix = std::make_shared<StVKStiffnessMatrix>(stVKInternalForces.get());
            }
            else
                stVKStiffnessMatrix = std::make_shared<StVKStiffnessMatrixMT>(stVKInternalForces.get(),
                femConfig->numInternalForceThreads);
        }
    }

    int scaleRows = 1;
    SparseMatrix *sm;
    meshGraph->GetLaplacian(&sm, scaleRows);
    LaplacianDampingMatrix.reset(sm);
    LaplacianDampingMatrix->ScalarMultiply(femConfig->dampingLaplacianCoef);
}

// Load the rendering mesh if it is designated
void smVegaFemSceneObject::loadSurfaceMesh()
{
    // initialize the rendering mesh for the volumetric mesh
    if (strcmp(femConfig->renderingMeshFilename, "__none") == 0)
    {
        std::cout << "VEGA Error: rendering mesh was not specified.\n";
        exit(1);
    }

    deformableObjectRenderingMesh = std::make_shared<smVegaSceneObjectDeformable>(femConfig->renderingMeshFilename);

    deformableObjectRenderingMesh->ResetDeformationToRest();
    deformableObjectRenderingMesh->BuildNeighboringStructure();
    deformableObjectRenderingMesh->BuildNormals();
    //deformableObjectRenderingMesh->SetMaterialAlpha(0.5);

    // initialize the embedded triangle rendering mesh
    secondaryDeformableObjectRenderingMesh = nullptr;

    if (strcmp(femConfig->secondaryRenderingMeshFilename, "__none") != 0)
    {

        secondaryDeformableObjectRenderingMesh = std::make_shared<smVegaSceneObjectDeformable>(femConfig->secondaryRenderingMeshFilename);

        if (secondaryDeformableObjectRenderingMesh == nullptr)
        {
            std::cout << "VEGA: Secondary rendering mesh is not initialized!\n";
            exit(1);
        }
        else
        {
            std::cout << "VEGA: Secondary rendering mesh is initialized:\n\t\t" 
                << secondaryDeformableObjectRenderingMesh->GetNumVertices() << " vertices\n\t\t"
                << secondaryDeformableObjectRenderingMesh->GetNumFaces() << " faces\n";
        }

        secondaryDeformableObjectRenderingMesh->ResetDeformationToRest();
        secondaryDeformableObjectRenderingMesh->BuildNeighboringStructure();
        secondaryDeformableObjectRenderingMesh->BuildNormals();

        //uSecondary = new double[3 * secondaryDeformableObjectRenderingMesh->Getn()]();

        uSecondary.resize(3 * secondaryDeformableObjectRenderingMesh->Getn());

        // load interpolation structure
        if (strcmp(femConfig->secondaryRenderingMeshInterpolationFilename, "__none") == 0)
        {
            std::cout << "VEGA Error: no secondary rendering mesh interpolation filename specified.\n";
            exit(1);
        }

        secondaryDeformableObjectRenderingMesh_interpolation_numElementVertices =
            VolumetricMesh::getNumInterpolationElementVertices(femConfig->secondaryRenderingMeshInterpolationFilename);

        if (secondaryDeformableObjectRenderingMesh_interpolation_numElementVertices < 0)
        {
            std::cout << "VEGA Error: unable to open file " << femConfig->secondaryRenderingMeshInterpolationFilename << "." << std::endl;
            exit(1);
        }

        std::cout << "VEGA: Num interpolation element vertices:" <<
            secondaryDeformableObjectRenderingMesh_interpolation_numElementVertices << std::endl;
      
        VolumetricMesh::loadInterpolationWeights(
            femConfig->secondaryRenderingMeshInterpolationFilename,
            secondaryDeformableObjectRenderingMesh->Getn(),
            secondaryDeformableObjectRenderingMesh_interpolation_numElementVertices,
            &secondaryDeformableObjectRenderingMesh_interpolation_vertices,
            &secondaryDeformableObjectRenderingMesh_interpolation_weights
            );
    }
    else
    {
        femConfig->renderSecondaryDeformableObject = 0;
    }
}

// Load the data related to the vertices that will remain fixed
void smVegaFemSceneObject::loadFixedBC()
{
    int numFixedVertices = fixedVertices.size();

    // read the fixed vertices
    // 1-indexed notation
    if (strcmp(femConfig->fixedVerticesFilename, "__none") == 0)
    {
        fixedVertices.clear();
    }
    else
    {
        int * data = fixedVertices.data();
        if (LoadList::load(femConfig->fixedVerticesFilename, &numFixedVertices, &data) != 0)
        {
            throw std::logic_error("VEGA: Error reading fixed vertices.");
        }

        LoadList::sort(numFixedVertices, fixedVertices.data());
    }

    std::cout << "VEGA: Loaded %d fixed vertices. They are:\n" << numFixedVertices << std::endl;
    LoadList::print(numFixedVertices, fixedVertices.data());

    // create 0-indexed fixed DOFs
    int numFixedDOFs = 3 * numFixedVertices;
    int * fixedDOFs = new int[numFixedDOFs];

    for (int i = 0; i < numFixedVertices; i++)
    {
        fixedDOFs[3 * i + 0] = 3 * fixedVertices[i] - 3;
        fixedDOFs[3 * i + 1] = 3 * fixedVertices[i] - 2;
        fixedDOFs[3 * i + 2] = 3 * fixedVertices[i] - 1;
    }

    for (int i = 0; i < numFixedVertices; i++)
    {
        fixedVertices[i]--;
    }
    
    numTotalDOF = 3 * numNodes;
    numFixedNodes = numFixedVertices;
    numFixedDof = 3 * numFixedVertices;
    numDOF = numTotalDOF - numFixedDof;

    std::cout << "VEGA: Boundary vertices processed.\n";


}

// load initial displacements and velocities of the nodes
void smVegaFemSceneObject::loadInitialStates()
{

    // load initial condition
    if (strcmp(femConfig->initialPositionFilename, "__none") != 0)
    {
        int m1, n1;

        auto aInitialPointer = uInitial.data();
        ReadMatrixFromDisk_(femConfig->initialPositionFilename, &m1, &n1, &aInitialPointer);

        if ((m1 != 3 * numNodes) || (n1 != 1))
        {
            std::cout << "VEGA Error: initial position matrix size mismatch.\n";
            exit(1);
        }
    }
    else
    {
        //uInitial = new double[3*n]();

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
            std::cout << "VEGA Error: initial position matrix size mismatch.\n";
            exit(1);
        }
    }
}

void smVegaFemSceneObject::loadScriptedExternalForces()
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
            std::cout << "VEGA Error: Mismatch in the dimension of the force load matrix.\n";
            exit(1);
        }
    }
}

// Inititialize the time integrator
void smVegaFemSceneObject::initializeTimeIntegrator()
{

    int numFixedDOFs = 3 * this->fixedVertices.size();
    std::vector<int> fixedDOFs(numFixedDOFs, 0);

    // initialize the integrator
    std::cout << "VEGA: Initializing the integrator, n = " << numNodes << "..." << std::endl;
    std::cout << "VEGA: Solver type: " << femConfig->solverMethod << std::endl;

    integratorBaseSparse = nullptr;

    if (femConfig->solver == IMPLICITNEWMARK)
    {
        implicitNewmarkSparse = std::make_shared<ImplicitNewmarkSparse>(3 * numNodes, femConfig->timeStep,
            massMatrix.get(), forceModel.get(), positiveDefinite,
            numFixedDOFs, fixedDOFs.data(),
            femConfig->dampingMassCoef,
            femConfig->dampingStiffnessCoef,
            femConfig->maxIterations,
            femConfig->epsilon,
            femConfig->newmarkBeta,
            femConfig->newmarkGamma,
            femConfig->numSolverThreads);
        integratorBaseSparse = implicitNewmarkSparse;
    }
    else if (femConfig->solver == IMPLICITBACKWARDEULER)
    {
        implicitNewmarkSparse = std::make_shared<ImplicitBackwardEulerSparse>(3 * numNodes, femConfig->timeStep,
            massMatrix.get(), forceModel.get(), positiveDefinite,
            numFixedDOFs, fixedDOFs.data(),
            femConfig->dampingMassCoef,
            femConfig->dampingStiffnessCoef,
            femConfig->maxIterations,
            femConfig->epsilon,
            femConfig->numSolverThreads);
        integratorBaseSparse = implicitNewmarkSparse;
    }
    else if (femConfig->solver == EULER)
    {
        int symplectic = 0;
        integratorBaseSparse = std::make_shared<EulerSparse>(3 * numNodes, femConfig->timeStep,
            massMatrix.get(), forceModel.get(), symplectic,
            numFixedDOFs, fixedDOFs.data(),
            femConfig->dampingMassCoef);
    }
    else if (femConfig->solver == SYMPLECTICEULER)
    {
        int symplectic = 1;
        integratorBaseSparse = std::make_shared<EulerSparse>(3 * numNodes, femConfig->timeStep, massMatrix.get(),
            forceModel.get(), symplectic, numFixedDOFs,
            fixedDOFs.data(), femConfig->dampingMassCoef);
    }
    else if (femConfig->solver == CENTRALDIFFERENCES)
    {
        integratorBaseSparse = std::make_shared<CentralDifferencesSparse>(3 * numNodes, femConfig->timeStep,
            massMatrix.get(), forceModel.get(),
            numFixedDOFs, fixedDOFs.data(),
            femConfig->dampingMassCoef,
            femConfig->dampingStiffnessCoef,
            femConfig->centralDifferencesTangentialDampingUpdateMode,
            femConfig->numSolverThreads);
    }

    LinearSolver *linearSolver = new CGSolver(integratorBaseSparse->GetSystemMatrix());
    integratorBaseSparse->setLinearSolver(linearSolver);
    integratorBase = integratorBaseSparse;

    if (integratorBase == nullptr)
    {
        std::cout << "VEGA Error: failed to initialize numerical integrator.\n";
        exit(1);
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

// Create the force model
void smVegaFemSceneObject::createForceModel()
{

    // create force models, to be used by the integrator
    std::cout << "VEGA: Creating force models..." << std::endl;

    switch (femConfig->deformableObject)
    {
    case STVK:
    {

        stVKForceModel = std::make_shared<StVKForceModel>(stVKInternalForces.get(), stVKStiffnessMatrix.get());
        forceModel = stVKForceModel;
        stVKForceModel->GetInternalForce(uInitial.data(), u.data());

        break;
    }
    case COROTLINFEM:
    {
        TetMesh * tetMesh = dynamic_cast<TetMesh*>(volumetricMesh.get());

        if (tetMesh == nullptr)
        {
            std::cout << "VEGA Error: the input mesh is not a tet mesh (CLFEM deformable model).\n";
            exit(1);
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

        corotationalLinearFEMForceModel = std::make_shared<CorotationalLinearFEMForceModel>(
            corotationalLinearFEM, femConfig->corotationalLinearFEM_warp);
        forceModel = corotationalLinearFEMForceModel;

        break;
    }
    case LINFEM:
    {
        std::shared_ptr<LinearFEMForceModel> linearFEMForceModel = 
                    std::make_shared<LinearFEMForceModel>(stVKInternalForces.get());
        forceModel = linearFEMForceModel;

        break;
    }
    case INVERTIBLEFEM:
    {
        TetMesh * tetMesh = dynamic_cast<TetMesh*>(volumetricMesh.get());

        if (tetMesh == nullptr)
        {
            std::cout << "VEGA Error: the input mesh is not a tet mesh (Invertible FEM deformable model).\n";
            exit(1);
        }

        IsotropicMaterial * isotropicMaterial = nullptr;

        // create the invertible material model
        if (strcmp(femConfig->invertibleMaterialString, "StVK") == 0)
        {
            femConfig->invertibleMaterial = INV_STVK;
        }

        if (strcmp(femConfig->invertibleMaterialString, "neoHookean") == 0)
        {
            femConfig->invertibleMaterial = INV_NEOHOOKEAN;
        }

        if (strcmp(femConfig->invertibleMaterialString, "MooneyRivlin") == 0)
        {
            femConfig->invertibleMaterial = INV_MOONEYRIVLIN;
        }

        switch (femConfig->invertibleMaterial)
        {
        case INV_STVK:
        {

            isotropicMaterial = new StVKIsotropicMaterial(tetMesh,
                femConfig->enableCompressionResistance,
                femConfig->compressionResistance);
            std::cout << "VEGA: Invertible material: StVK.\n";
            break;
        }

        case INV_NEOHOOKEAN:
            isotropicMaterial = new NeoHookeanIsotropicMaterial(tetMesh,
                femConfig->enableCompressionResistance,
                femConfig->compressionResistance);
            std::cout << "VEGA: Invertible material: neo-Hookean.\n";
            break;

        case INV_MOONEYRIVLIN:
            isotropicMaterial = new MooneyRivlinIsotropicMaterial(tetMesh,
                femConfig->enableCompressionResistance,
                femConfig->compressionResistance);
            std::cout << "VEGA: Invertible material: Mooney-Rivlin.\n";
            break;

        default:
            std::cout << "VEGA Error: invalid invertible material type.\n";
            exit(1);
            break;
        }

        // create the invertible FEM deformable model
        IsotropicHyperelasticFEM * isotropicHyperelasticFEM;

        if (femConfig->numInternalForceThreads == 0)
            isotropicHyperelasticFEM = new IsotropicHyperelasticFEM(tetMesh, isotropicMaterial,
            femConfig->inversionThreshold,
            femConfig->addGravity, femConfig->g);
        else
            isotropicHyperelasticFEM = new IsotropicHyperelasticFEMMT(tetMesh,
            isotropicMaterial,
            femConfig->inversionThreshold,
            femConfig->addGravity,
            femConfig->g,
            femConfig->numInternalForceThreads);

        // create force model for the invertible FEM class
        std::shared_ptr<IsotropicHyperelasticFEMForceModel> isotropicHyperelasticFEMForceModel =
            std::make_shared<IsotropicHyperelasticFEMForceModel>(isotropicHyperelasticFEM);
        forceModel = isotropicHyperelasticFEMForceModel;

        break;
    }
    default:
    {
        std::cout << "VEGA: Error! Scene object is not of FE type!" << std::endl;
        break;
    }
    }

}

// Update the deformations by time stepping
void smVegaFemSceneObject::advanceDynamics()
{
    performaceTracker.cpuLoadCounter.StartCounter();

    // reset external forces (usually to zero)
    memcpy(f_ext.data(), f_extBase.data(), sizeof(double) * 3 * numNodes);

    if ((!femConfig->lockScene) && (!femConfig->pauseSimulation) && (femConfig->singleStepMode <= 1))
    {

        // apply external user interaction forces
        applyUserInteractionForces();

        // apply pre-defined external forces
        applyScriptedExternalForces();

        // apply external forces arising from contact
        applyContactForces();

        // set forces to the integrator
        integratorBaseSparse->SetExternalForces(f_ext.data());

        // timestep the dynamics
        advanceOneTimeStep();

        timestepCounter++;

        memcpy(u.data(), integratorBase->Getq(), sizeof(double) * 3 * numNodes);
        
        std::cout << "Time step: " << timestepCounter << std::endl;

        if (importAndUpdateVolumeMeshToSmtk)
        {
            smtkVolumeMesh->updateVolumeMeshFromVegaFormat(this->volumetricMesh);
        }

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
                performaceTracker.titleBarCounter.StopCounter();
                elapsedTime = performaceTracker.titleBarCounter.GetElapsedTime();
            } while (1.0 * graphicFrame / elapsedTime >= 30.0);
        }
    }

    deformableObjectRenderingMesh->SetVertexDeformations(u.data());

    // update the secondary mesh
    updateSecondaryRenderingMesh();

    if (!renderUsingVega)
    {

        if (strcmp(femConfig->secondaryRenderingMeshFilename, "__none") == 0)
        {
            this->smtkSurfaceMesh->updateSurfaceMeshFromVegaFormat(this->deformableObjectRenderingMesh->GetMesh());
        }
        else
        {
            this->smtkSurfaceMesh->updateSurfaceMeshFromVegaFormat(this->secondaryDeformableObjectRenderingMesh->GetMesh());
        }
    }

    smtkSurfaceMesh->updateTriangleNormals();
    smtkSurfaceMesh->updateVertexNormals();

    // update stasts
    updatePerformanceMetrics();

    performaceTracker.cpuLoadCounter.StopCounter();
    double cpuTimePerGraphicsFrame = performaceTracker.cpuLoadCounter.GetElapsedTime();
    performaceTracker.cpuLoad = cpuTimePerGraphicsFrame * performaceTracker.fps;
}

// Advance in time by a specificed amount and a chosen time stepping scheme
inline void smVegaFemSceneObject::advanceOneTimeStep()
{
    smVegaPerformanceCounter *pt =  &performaceTracker;
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
            std::cout << "VEGA: The integrator went unstable. Reduce the timestep, or increase the number of substeps per timestep.\n";
            integratorBase->ResetToRest();

            for (int i = 0; i < 3 * numNodes; i++)
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


void smVegaFemSceneObject::applyContactForces()
{
    if (f_contact.size() != 0)
    {
        for (int i = 0; i < f_contact.size(); i++)
        {
            f_ext[i] += f_contact[i];
        }
    }
}


void smVegaFemSceneObject::setContactForcesToZero()
{
    f_contact.assign(f_contact.size(), 0.0);
}

void smVegaFemSceneObject::setPulledVertex(const smVec3d &userPos)
{
    pulledVertex = this->volumetricMesh->getClosestVertex(Vec3d(userPos(0), userPos(1), userPos(2)));
}

// Forces as a result of user interaction (through an interface such as mouse or haptic device) with the scene during runtime are added here
inline void smVegaFemSceneObject::applyUserInteractionForces()
{

    // determine force in case user is pulling on a vertex
    if (pulledVertex != -1)
    {

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

            for (std::set<int> ::iterator iter = newAffectedVertices.begin(); iter != newAffectedVertices.end(); iter++)
            {
                // apply force
                f_ext[3 * *iter + 0] += forceMagnitude * externalForce[0];
                f_ext[3 * *iter + 1] += forceMagnitude * externalForce[1];
                f_ext[3 * *iter + 2] += forceMagnitude * externalForce[2];

                // generate new layers
                lastLayerVertices.insert(*iter);
                affectedVertices.insert(*iter);
            }
        }
    }
}


// Forces that are defined by the user before the start of the simulation is added to the external force vector here
inline void smVegaFemSceneObject::applyScriptedExternalForces()
{

    // apply any scripted force loads
    if (timestepCounter < this->forceLoads.size())
    {
        std::cout << "  External forces read from the binary input file.\n";

        size_t offset = 3 * numNodes*timestepCounter;
        for (size_t i = 0, end = 3u * numNodes; i < end; i++)
        {
            f_ext[i] += forceLoads[offset + i];
        }
    }

}

// Use the computed displacemetnt update to interpolate to the secondary display mesh
inline void smVegaFemSceneObject::updateSecondaryRenderingMesh()
{

    // interpolate deformations from volumetric mesh to rendering triangle mesh
    if (secondaryDeformableObjectRenderingMesh != nullptr)
    {
        VolumetricMesh::interpolate(
            u.data(), uSecondary.data(),
            secondaryDeformableObjectRenderingMesh->Getn(),
            secondaryDeformableObjectRenderingMesh_interpolation_numElementVertices,
            secondaryDeformableObjectRenderingMesh_interpolation_vertices,
            secondaryDeformableObjectRenderingMesh_interpolation_weights
            );
        secondaryDeformableObjectRenderingMesh->SetVertexDeformations(uSecondary.data());
    }

    if (femConfig->useRealTimeNormals)
    {
        // recompute normals
        deformableObjectRenderingMesh->BuildNormals();

        if (secondaryDeformableObjectRenderingMesh != nullptr)
        {
            secondaryDeformableObjectRenderingMesh->BuildNormals();
        }
    }

    if (explosionFlag)
    {
        performaceTracker.explosionCounter.StopCounter();

        if (performaceTracker.explosionCounter.GetElapsedTime() > 4.0) // the message will appear on screen for 4 seconds
        {
            explosionFlag = 0;
        }
    }
}

// Updates the stats related to timing, fps etc. Also updates window title with real-time information
inline void smVegaFemSceneObject::updatePerformanceMetrics()
{
    smVegaPerformanceCounter *pt = &performaceTracker;
    //update window title at 5 Hz
    pt->titleBarCounter.StopCounter();
    double elapsedTime = pt->titleBarCounter.GetElapsedTime();

    if (elapsedTime >= 1.0 / 5)
    {
        pt->titleBarCounter.StartCounter();
        double fpsLocal = graphicFrame / elapsedTime;

        // average fps over last "fpsBufferSize" samples
        pt->fps += 1.0 / pt->fpsBufferSize * (fpsLocal - pt->fpsBuffer[pt->fpsHead]);
        pt->fpsBuffer[pt->fpsHead] = fpsLocal;
        pt->fpsHead = (pt->fpsHead + 1) % pt->fpsBufferSize;

        graphicFrame = 0;

        if ((femConfig->syncTimestepWithGraphics) && ((!femConfig->lockScene) &&
            (!femConfig->pauseSimulation) && (femConfig->singleStepMode == 0)))
        {
            femConfig->timeStep = 1.0 / pt->fps;
            integratorBase->SetTimestep(femConfig->timeStep / femConfig->substepsPerTimeStep);
        }
    }
}

void smVegaFemSceneObject::draw()
{
    smtkSurfaceMesh->draw();
}

void smVegaFemSceneObject::setContactForceOfNodeWithDofID(const int dofID, const smVec3d force)
{
    f_contact[dofID    ] = force(0);
    f_contact[dofID + 1] = force(1);
    f_contact[dofID + 2] = force(2);
}

smVec3d smVegaFemSceneObject::getVelocityOfNodeWithDofID(const int dofID) const
{
    smVec3d vel(uvel[dofID], uvel[dofID + 1], uvel[dofID + 2]);

    return vel;
}

smVec3d smVegaFemSceneObject::getDisplacementOfNodeWithDofID(const int dofID) const
{
    smVec3d disp(u[dofID], u[dofID + 1], u[dofID + 2]);

    return disp;
}

smVec3d smVegaFemSceneObject::getAccelerationOfNodeWithDofID(const int dofID) const
{
    smVec3d accn(uaccel[dofID], uaccel[dofID + 1], uaccel[dofID + 2]);

    return accn;
}

int smVegaFemSceneObject::getNumNodes() const
{
    return numNodes;
}

int smVegaFemSceneObject::getNumTotalDof() const
{
    return numTotalDOF;
}

int smVegaFemSceneObject::getNumDof() const
{
    return numDOF;
}

int smVegaFemSceneObject::getNumFixedNodes() const
{
    return numFixedNodes;
}

int smVegaFemSceneObject::getNumFixedDof() const
{
    return numFixedDof;
}