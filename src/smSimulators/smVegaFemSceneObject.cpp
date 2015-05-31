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
    fps = 0.0;
    fpsHead = 0;
    cpuLoad = 0;
    forceAssemblyTime = 0.0;
    forceAssemblyLocalTime = 0.0;
    forceAssemblyHead = 0;
    systemSolveTime = 0.0;
    systemSolveLocalTime = 0.0;
    systemSolveHead = 0;
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

    fpsBufferSize = 5; ///< buffer size to display fps
    forceAssemblyBufferSize = VEGA_PERFORMANCE_REC_BUFFER_SIZE;
    systemSolveBufferSize = VEGA_PERFORMANCE_REC_BUFFER_SIZE;

    volumetricMesh = nullptr;
    tetMesh = nullptr;
    meshGraph = nullptr;

    massMatrix = nullptr;
    LaplacianDampingMatrix = nullptr;

    deformableObjectRenderingMesh = nullptr;
    secondaryDeformableObjectRenderingMesh = nullptr;

    renderUsingVega = true;
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

void smVegaFemSceneObject::duplicateAtRuntime(std::shared_ptr<smVegaFemSceneObject> &newSO)
{
    //if(newSO==nullptr)
    //{
    //newSO = std::make_shared<smVegaFemSceneObject>();
    //}
    //else
    //{
    //newSO->setDefaults();
    //}

    //newSO->femConfig = std::make_shared<smVegaConfigFemObject>(this->femConfig);

    //newSO->deformableObject = this->deformableObject;

    //newSO->volumetricMesh = std::make_shared<VolumetricMesh>(this->volumetricMesh->get());

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
        fps = 0.0;
        fpsHead = 0;
        cpuLoad = 0;
        forceAssemblyTime = 0.0;
        forceAssemblyLocalTime = 0.0;
        forceAssemblyHead = 0;
        systemSolveTime = 0.0;
        systemSolveLocalTime = 0.0;
        systemSolveHead = 0;
        graphicFrame = 0;
        pulledVertex = -1;
        explosionFlag = 0;
        timestepCounter = 0;
        subTimestepCounter = 0;

        loadInitialStates();

        // clear fps buffer
        int i;
        for (i = 0; i < fpsBufferSize; i++)
        {
            fpsBuffer[i] = 0.0;
        }

        for (i = 0; i < forceAssemblyBufferSize; i++)
        {
            forceAssemblyBuffer[i] = 0.0;
        }

        for (i = 0; i < systemSolveBufferSize; i++)
        {
            systemSolveBuffer[i] = 0.0;
        }
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

    setDeformableModel();
    loadMeshes();
    loadRenderingMesh();

    if (importAndUpdateVolumeMeshToSmtk)
    {
        this->smtkVolumeMesh = std::make_shared<smVolumeMesh>();
        this->smtkVolumeMesh->importVolumeMeshDataFromVEGA_Format(this->volumetricMesh, true);
    }

    if (!renderUsingVega)
    {
        this->smtkSurfaceMesh = std::make_shared<smSurfaceMesh>();
        if (strcmp(femConfig->secondaryRenderingMeshFilename, "__none") == 0)
        {
            this->smtkSurfaceMesh->importSurfaceMeshDataFromVEGA_Format(this->deformableObjectRenderingMesh->GetMesh(), true);
        }
        else
        {
            this->smtkSurfaceMesh->importSurfaceMeshDataFromVEGA_Format(this->secondaryDeformableObjectRenderingMesh->GetMesh(), true);
        }

    }

    loadFixedBC();

    // make room for deformation and force vectors
    u.resize(3 * n);
    uvel.resize(3 * n);
    uaccel.resize(3 * n);
    f_extBase.resize(3 * n);
    f_ext.resize(3 * n);
    f_contact.resize(3 * n);

    loadInitialStates();
    loadScriptedExternalFroces();
    createForceModel();
    initializeTimeIntegrator();

    // clear fps buffer
    for (int i = 0; i < fpsBufferSize; i++)
    {
        fpsBuffer[i] = 0.0;
    }

    for (int i = 0; i < forceAssemblyBufferSize; i++)
    {
        forceAssemblyBuffer[i] = 0.0;
    }

    for (int i = 0; i < systemSolveBufferSize; i++)
    {
        systemSolveBuffer[i] = 0.0;
    }

    titleBarCounter.StartCounter();

    std::cout << "Init simulator done. \n";
}


// Set the type of formulation used to model the deformation
void smVegaFemSceneObject::setDeformableModel()
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
void smVegaFemSceneObject::loadMeshes()
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

        n = volumetricMesh->getNumVertices();
        std::cout << "VEGA: Num vertices: "<< n <<". Num elements: "<< volumetricMesh->getNumElements() << std::endl;
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
void smVegaFemSceneObject::loadRenderingMesh()
{
    // initialize the rendering mesh for the volumetric mesh
    if (strcmp(femConfig->renderingMeshFilename, "__none") == 0)
    {
        std::cout << "VEGA Error: rendering mesh was not specified.\n";
        exit(1);
    }

    deformableObjectRenderingMesh = std::make_shared<SceneObjectDeformable>(femConfig->renderingMeshFilename);

    if (enableTextures)
    {
        deformableObjectRenderingMesh->SetUpTextures(SceneObject::MODULATE, SceneObject::NOMIPMAP);
    }

    deformableObjectRenderingMesh->ResetDeformationToRest();
    deformableObjectRenderingMesh->BuildNeighboringStructure();
    deformableObjectRenderingMesh->BuildNormals();
    deformableObjectRenderingMesh->SetMaterialAlpha(0.5);

    // initialize the embedded triangle rendering mesh
    secondaryDeformableObjectRenderingMesh = nullptr;

    if (strcmp(femConfig->secondaryRenderingMeshFilename, "__none") != 0)
    {

        secondaryDeformableObjectRenderingMesh = std::make_shared<SceneObjectDeformable>(femConfig->secondaryRenderingMeshFilename);

        if (enableTextures)
        {
            secondaryDeformableObjectRenderingMesh->SetUpTextures(SceneObject::MODULATE, SceneObject::NOMIPMAP);
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

        int *s = secondaryDeformableObjectRenderingMesh_interpolation_vertices.data();
        double *p = secondaryDeformableObjectRenderingMesh_interpolation_weights.data();
        VolumetricMesh::loadInterpolationWeights(
            femConfig->secondaryRenderingMeshInterpolationFilename,
            secondaryDeformableObjectRenderingMesh->Getn(),
            secondaryDeformableObjectRenderingMesh_interpolation_numElementVertices,
            &s,
            &p
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

        if ((m1 != 3 * n) || (n1 != 1))
        {
            std::cout << "VEGA Error: initial position matrix size mismatch.\n";
            exit(1);
        }
    }
    else
    {
        //uInitial = new double[3*n]();

        uInitial.resize(3 * n);
    }

    // load initial velocity
    if (strcmp(femConfig->initialVelocityFilename, "__none") != 0)
    {
        int m1, n1;
        auto velInitialPointer = uInitial.data();
        ReadMatrixFromDisk_(femConfig->initialVelocityFilename, &m1, &n1, &velInitialPointer);

        if ((m1 != 3 * n) || (n1 != 1))
        {
            std::cout << "VEGA Error: initial position matrix size mismatch.\n";
            exit(1);
        }
    }
}

void smVegaFemSceneObject::loadScriptedExternalFroces()
{
    // load force loads
    int numForceLoads = forceLoads.size();
    if (strcmp(femConfig->forceLoadsFilename, "__none") != 0)
    {
        int m1;
        double *data = forceLoads.data();
        ReadMatrixFromDisk_(femConfig->forceLoadsFilename, &m1, &numForceLoads, &data);

        if (m1 != 3 * n)
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
    std::cout << "VEGA: Initializing the integrator, n = "<< n << "..." << std::endl;
    std::cout << "VEGA: Solver type: " << femConfig->solverMethod << std::endl;

    integratorBaseSparse = nullptr;

    if (femConfig->solver == IMPLICITNEWMARK)
    {
        implicitNewmarkSparse = std::make_shared<ImplicitNewmarkSparse>(3 * n, femConfig->timeStep,
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
        implicitNewmarkSparse = std::make_shared<ImplicitBackwardEulerSparse>(3 * n, femConfig->timeStep,
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
        integratorBaseSparse = std::make_shared<EulerSparse>(3 * n, femConfig->timeStep,
            massMatrix.get(), forceModel.get(), symplectic,
            numFixedDOFs, fixedDOFs.data(),
            femConfig->dampingMassCoef);
    }
    else if (femConfig->solver == SYMPLECTICEULER)
    {
        int symplectic = 1;
        integratorBaseSparse = std::make_shared<EulerSparse>(3 * n, femConfig->timeStep, massMatrix.get(),
            forceModel.get(), symplectic, numFixedDOFs,
            fixedDOFs.data(), femConfig->dampingMassCoef);
    }
    else if (femConfig->solver == CENTRALDIFFERENCES)
    {
        integratorBaseSparse = std::make_shared<CentralDifferencesSparse>(3 * n, femConfig->timeStep,
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

    cpuLoadCounter.StartCounter();

    // reset external forces (usually to zero)
    memcpy(f_ext.data(), f_extBase.data(), sizeof(double) * 3 * n);

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

        memcpy(u.data(), integratorBase->Getq(), sizeof(double) * 3 * n);

        if (importAndUpdateVolumeMeshToSmtk)
        {
            smtkVolumeMesh->updateVolumeMeshDataFromVEGA_Format(this->volumetricMesh);
        }

        if (femConfig->singleStepMode == 1)
        {
            femConfig->singleStepMode = 2;
        }

        //printf("VEGA: F"); fflush(nullptr);
        graphicFrame++;

        if (femConfig->lockAt30Hz)
        {
            double elapsedTime;

            do
            {
                titleBarCounter.StopCounter();
                elapsedTime = titleBarCounter.GetElapsedTime();
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
            this->smtkSurfaceMesh->updateSurfaceMeshDataFromVEGA_Format(this->deformableObjectRenderingMesh->GetMesh());
        }
        else
        {
            this->smtkSurfaceMesh->updateSurfaceMeshDataFromVEGA_Format(this->secondaryDeformableObjectRenderingMesh->GetMesh());
        }
    }

    // update stasts
    updateStats();

    cpuLoadCounter.StopCounter();
    double cpuTimePerGraphicsFrame = cpuLoadCounter.GetElapsedTime();
    cpuLoad = cpuTimePerGraphicsFrame * fps;
}

// Advance in time by a specificed amount and a chosen time stepping scheme
inline void smVegaFemSceneObject::advanceOneTimeStep()
{

    for (int i = 0; i < femConfig->substepsPerTimeStep; i++)
    {
        int code = integratorBase->DoTimestep();
        forceAssemblyLocalTime = integratorBaseSparse->GetForceAssemblyTime();
        systemSolveLocalTime = integratorBaseSparse->GetSystemSolveTime();

        // average forceAssemblyTime over last "forceAssemblyBufferSize" samples
        forceAssemblyTime += 1.0 / forceAssemblyBufferSize *
            (forceAssemblyLocalTime - forceAssemblyBuffer[forceAssemblyHead]);
        forceAssemblyBuffer[forceAssemblyHead] = forceAssemblyLocalTime;
        forceAssemblyHead = (forceAssemblyHead + 1) % forceAssemblyBufferSize;

        // average systemSolveTime over last "systemSolveBufferSize" samples
        systemSolveTime += 1.0 / systemSolveBufferSize *
            (systemSolveLocalTime - systemSolveBuffer[systemSolveHead]);
        systemSolveBuffer[systemSolveHead] = systemSolveLocalTime;
        systemSolveHead = (systemSolveHead + 1) % systemSolveBufferSize;

        if (code != 0)
        {
            std::cout << "VEGA: The integrator went unstable. Reduce the timestep, or increase the number of substeps per timestep.\n";
            integratorBase->ResetToRest();

            for (int i = 0; i < 3 * n; i++)
            {
                f_ext[i] = 0;
                f_extBase[i] = 0;
            }

            integratorBase->SetExternalForcesToZero();
            explosionFlag = 1;
            explosionCounter.StartCounter();
            break;
        }

        // update UI performance indicators
        if (strcmp(femConfig->outputFilename, "__none") != 0)
        {
            char s[4096];
            sprintf(s, "%s.u.%04d", femConfig->outputFilename, subTimestepCounter);
            std::cout << "VEGA: Saving deformation to " << s << ".\n";
            WriteMatrixToDisk_(s, 3 * n, 1, integratorBase->Getq());
            sprintf(s, "%s.f.%04d", femConfig->outputFilename, subTimestepCounter);
            std::cout << "VEGA: Saving forces to " << s << ".\n";
            WriteMatrixToDisk_(s, 3 * n, 1, integratorBase->GetExternalForces());
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

        size_t offset = 3 * n*timestepCounter;
        for (size_t i = 0, end = 3u * n; i < end; i++)
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
            secondaryDeformableObjectRenderingMesh_interpolation_vertices.data(),
            secondaryDeformableObjectRenderingMesh_interpolation_weights.data()
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
        explosionCounter.StopCounter();

        if (explosionCounter.GetElapsedTime() > 4.0) // the message will appear on screen for 4 seconds
        {
            explosionFlag = 0;
        }
    }
}

// Updates the stats related to timing, fps etc. Also updates window title with real-time information
inline void smVegaFemSceneObject::updateStats()
{

    //update window title at 5 Hz
    titleBarCounter.StopCounter();
    double elapsedTime = titleBarCounter.GetElapsedTime();

    if (elapsedTime >= 1.0 / 5)
    {
        titleBarCounter.StartCounter();
        double fpsLocal = graphicFrame / elapsedTime;

        // average fps over last "fpsBufferSize" samples
        fps += 1.0 / fpsBufferSize * (fpsLocal - fpsBuffer[fpsHead]);
        fpsBuffer[fpsHead] = fpsLocal;
        fpsHead = (fpsHead + 1) % fpsBufferSize;

        if (femConfig->displayWindowTitle == 1)
        {
            // update window title
            char windowTitle[4096] = "unknown defo model";

            if (femConfig->deformableObject == STVK)
                sprintf(windowTitle, "%s | %s | Elements: %d | DOFs: %d | %.1f Hz |"
                "Defo CPU Load: %d%%", "StVK", femConfig->solverMethod,
                volumetricMesh->getNumElements(), volumetricMesh->getNumVertices() * 3,
                fps, smInt(100 * cpuLoad + 0.5));

            if (femConfig->deformableObject == COROTLINFEM)
                sprintf(windowTitle, "%s:%d | %s | Elements: %d | DOFs: %d | %.1f Hz |"
                "Defo CPU Load: %d%%", "CLFEM", femConfig->corotationalLinearFEM_warp,
                femConfig->solverMethod, volumetricMesh->getNumElements(),
                volumetricMesh->getNumVertices() * 3, fps, smInt(100 * cpuLoad + 0.5));

            if (femConfig->deformableObject == LINFEM)
                sprintf(windowTitle, "%s | %s | Elements: %d | DOFs: %d | %.1f Hz |"
                "Defo CPU Load: %d%%", "LinFEM", femConfig->solverMethod,
                volumetricMesh->getNumElements(), volumetricMesh->getNumVertices() * 3, fps,
                smInt(100 * cpuLoad + 0.5));

            if (femConfig->deformableObject == INVERTIBLEFEM)
            {
                char materialType[96] = "Invertible FEM";

                if (femConfig->invertibleMaterial == INV_STVK)
                {
                    strcpy(materialType, "Invertible StVK");
                }

                if (femConfig->invertibleMaterial == INV_NEOHOOKEAN)
                {
                    strcpy(materialType, "Invertible neo-Hookean");
                }

                if (femConfig->invertibleMaterial == INV_MOONEYRIVLIN)
                {
                    strcpy(materialType, "Invertible Mooney-Rivlin");
                }

                sprintf(windowTitle, "%s | %s | Elements: %d | DOFs: %d | %.1f Hz |"
                    "Defo CPU Load: %d%%", materialType, femConfig->solverMethod,
                    volumetricMesh->getNumElements(), volumetricMesh->getNumVertices() * 3,
                    fps, smInt(100 * cpuLoad + 0.5));
            }

            //             glutSetWindowTitle(windowTitle);
        }

        graphicFrame = 0;

        if ((femConfig->syncTimestepWithGraphics) && ((!femConfig->lockScene) &&
            (!femConfig->pauseSimulation) && (femConfig->singleStepMode == 0)))
        {
            femConfig->timeStep = 1.0 / fps;
            integratorBase->SetTimestep(femConfig->timeStep / femConfig->substepsPerTimeStep);
        }
    }
}

void smVegaFemSceneObject::setRenderUsingVega(bool vegaRender)
{
    this->renderUsingVega = vegaRender;
}

void smVegaFemSceneObject::draw(const smDrawParam &p_params)
{
    if (!renderUsingVega)
    {
        smtkSurfaceMesh->draw(p_params);
    }
    else
    {
        renderWithVega();
    }
}

// Displays the fem object with primary or secondary mesh, fixed vertices, vertices interacted with, ground plane etc.
void smVegaFemSceneObject::renderWithVega()
{

    glEnable(GL_LIGHTING);

    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    //glStencilFunc(GL_ALWAYS, 0, ~(0u));

    // render embedded triangle mesh
    if (femConfig->renderSecondaryDeformableObject)
    {
        secondaryDeformableObjectRenderingMesh->Render();
    }

    glStencilFunc(GL_ALWAYS, 1, ~(0u));

    // render the main deformable object (surface of volumetric mesh)
    if (femConfig->renderDeformableObject)
    {
        if (femConfig->renderSecondaryDeformableObject)
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_BLEND);

            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1.0, 1.0);
            glDrawBuffer(GL_NONE);
            deformableObjectRenderingMesh->Render();
            glDisable(GL_POLYGON_OFFSET_FILL);
            glDrawBuffer(GL_BACK);
            glEnable(GL_LIGHTING);
        }

        glColor3f(0.0, 0.0, 0.0);
        deformableObjectRenderingMesh->Render();

        if (femConfig->renderVertices)
        {
            glDisable(GL_LIGHTING);
            glColor3f(0.5, 0, 0);
            glPointSize(8.0);
            deformableObjectRenderingMesh->RenderVertices();
            glEnable(GL_LIGHTING);
        }

        if (femConfig->renderSecondaryDeformableObject)
        {
            glDisable(GL_BLEND);
        }
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);

    /* glStencilFunc(GL_ALWAYS, 1, ~(0u));
     glColor3f(0, 0, 0);*/

    if (femConfig->renderWireframe)
    {
        deformableObjectRenderingMesh->RenderEdges();
    }

    // disable stencil buffer modifications
    //glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    //glColor3f(0, 0, 0);

    /*if (femConfig->renderAxes)
    {
    glLineWidth(1.0);
    }*/

    // render the currently pulled vertex
    if (pulledVertex >= 0)
    {
        glColor3f(0, 1, 0);
        double pulledVertexPos[3];
        deformableObjectRenderingMesh->GetSingleVertexPositionFromBuffer(pulledVertex,
            &pulledVertexPos[0], &pulledVertexPos[1], &pulledVertexPos[2]);

        glEnable(GL_POLYGON_OFFSET_POINT);
        glPolygonOffset(-1.0, -1.0);
        glPointSize(8.0);
        glBegin(GL_POINTS);
        glVertex3d(pulledVertexPos[0], pulledVertexPos[1], pulledVertexPos[2]);
        glEnd();
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    // render model fixed vertices
    if (femConfig->renderFixedVertices)
    {
        for (int i = 0; i < this->fixedVertices.size(); i++)
        {
            glColor3f(1, 0, 0);
            double fixedVertexPos[3];
            deformableObjectRenderingMesh->GetSingleVertexRestPosition(fixedVertices[i],
                &fixedVertexPos[0], &fixedVertexPos[1], &fixedVertexPos[2]);

            glEnable(GL_POLYGON_OFFSET_POINT);
            glPolygonOffset(-1.0, -1.0);
            glPointSize(12.0);
            glBegin(GL_POINTS);
            glVertex3d(fixedVertexPos[0], fixedVertexPos[1], fixedVertexPos[2]);
            glEnd();
            glDisable(GL_POLYGON_OFFSET_FILL);
        }
    }

    // ==== bitmap routines below here
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // print info in case of integrator blow-up
    char s[4096];
    glColor3f(1, 0, 0);

    if (explosionFlag)
    {
        /*sprintf(s, "The integrator went unstable.");
        int windowWidth = 800;
        int windowHeight = 600;
        double x1 = 10;
        double y1 = 25;
        double X1 = -1 + 2.0 * x1 / windowWidth;
        double Y1 = -1 + 2.0 * y1 / windowHeight;
        print_bitmap_string(X1, Y1, -1, GLUT_BITMAP_9_BY_15 , s);

        sprintf(s, "Reduce the timestep, or increase the number of substeps per timestep.");
        x1 = 10;
        y1 = 10;
        X1 = -1 + 2.0 * x1 / windowWidth;
        Y1 = -1 + 2.0 * y1 / windowHeight;
        print_bitmap_string(X1, Y1, -1, GLUT_BITMAP_9_BY_15 , s);*/

        std::cout << s, "VEGA: The integrator went unstable!!\n";
    }

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    //glutSwapBuffers();
}

//font is, for example, GLUT_BITMAP_9_BY_15
void smVegaFemSceneObject::print_bitmap_string(float x, float y, float z, void * font, char * s)
{

   /* glRasterPos3f(x, y, z);

    if (s && strlen(s))
    {
        while (*s)
        {
            glutBitmapCharacter(font, *s);
            s++;
        }
    }*/
}
