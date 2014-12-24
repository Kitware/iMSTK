#include "smSimulators/smVegaFemSceneObject.h"

smVegaFemSceneObject::smVegaFemSceneObject(smErrorLog *p_log, string ConfigFile)
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
    numForceLoads = 0;
    forceLoads = NULL;
    integratorBase = NULL;
    implicitNewmarkSparse = NULL;
    integratorBaseSparse = NULL;
    forceModel = NULL;
    stVKInternalForces = NULL;
    stVKStiffnessMatrix = NULL;
    stVKForceModel = NULL;
    massSpringSystemForceModel = NULL;
    corotationalLinearFEMForceModel = NULL;
    positiveDefinite = 0;

    volumetricMesh = NULL;
    tetMesh = NULL;
    meshGraph = NULL;

    massSpringSystem = NULL;
    renderMassSprings = NULL;
    massMatrix = NULL;
    LaplacianDampingMatrix = NULL;

    //states
    u = NULL;
    uvel = NULL;
    uaccel = NULL;
    f_ext = NULL;
    f_extBase = NULL;
    uSecondary = NULL;
    uInitial = NULL;
    velInitial = NULL;

    deformableObjectRenderingMesh = NULL;
    secondaryDeformableObjectRenderingMesh = NULL;

    secondaryDeformableObjectRenderingMesh_interpolation_vertices = NULL;
    secondaryDeformableObjectRenderingMesh_interpolation_weights = NULL;

    type = SOFMIS_SMFEMSCENEOBJECT;

    femConfig = new smVegaConfigFemObject();

    if (ConfigFile.compare(vega_string_none) != 0)
    {
        femConfig->setFemObjConfuguration(ConfigFile);
        printf("VEGA: Initialized the VegaFemSceneObject and configured using file %s\n!",
               ConfigFile);
    }
    else
    {
        printf("VEGA: NOTE: Initialized the VegaFemSceneObject, but not configured yet!\n");
    }

    initSimulation();
}


smVegaFemSceneObject::~smVegaFemSceneObject()
{

    if (u != NULL)
    {
        free(u);
    }

    if (uvel != NULL)
    {
        free(uvel);
    }

    if (uaccel != NULL)
    {
        free(uaccel);
    }

    if (f_ext != NULL)
    {
        free(f_ext);
    }

    if (f_extBase != NULL)
    {
        free(f_extBase);
    }

    if (uSecondary != NULL)
    {
        free(uSecondary);
    }

    if (uInitial != NULL)
    {
        free(uInitial);
    }

    if (velInitial != NULL)
    {
        free(velInitial);
    }

    //REVISIT!
    if (fixedVertices != NULL)
    {
        delete fixedVertices;
    }

    if (integratorBase != NULL)
    {
        delete integratorBase;
    }

    if (implicitNewmarkSparse != NULL)
    {
        delete implicitNewmarkSparse;
    }

    if (integratorBaseSparse != NULL)
    {
        delete integratorBaseSparse;
    }

    if (forceModel != NULL)
    {
        delete forceModel;
    }

    if (stVKInternalForces != NULL)
    {
        delete stVKInternalForces;
    }

    if (stVKStiffnessMatrix != NULL)
    {
        delete stVKStiffnessMatrix;
    }

    if (stVKForceModel != NULL)
    {
        delete stVKForceModel;
    }

    if (massSpringSystemForceModel != NULL)
    {
        delete massSpringSystemForceModel;
    }

    if (corotationalLinearFEMForceModel != NULL)
    {
        delete corotationalLinearFEMForceModel;
    }

    if (volumetricMesh != NULL)
    {
        delete volumetricMesh;
    }

    if (tetMesh != NULL)
    {
        delete tetMesh;
    }

    if (meshGraph != NULL)
    {
        delete meshGraph;
    }

    if (massSpringSystem != NULL)
    {
        delete massSpringSystem;
    }

    if (renderMassSprings != NULL)
    {
        delete renderMassSprings;
    }

    if (massMatrix != NULL)
    {
        delete massMatrix;
    }

    if (LaplacianDampingMatrix != NULL)
    {
        delete LaplacianDampingMatrix;
    }

    if (secondaryDeformableObjectRenderingMesh_interpolation_vertices != NULL)
    {
        delete secondaryDeformableObjectRenderingMesh_interpolation_vertices;
    }

    if (secondaryDeformableObjectRenderingMesh_interpolation_weights != NULL)
    {
        delete secondaryDeformableObjectRenderingMesh_interpolation_weights;
    }
}

// Initialize the parameters and properties of the simulation object
void smVegaFemSceneObject::initSimulation()
{

    volumetricMesh = NULL;
    massSpringSystem = NULL;

    setDeformableModel();
    loadMeshes();
    loadRenderingMesh();
    loadFixedBC();

    // make room for deformation and force vectors
    u = (double*) calloc(3 * n, sizeof(double));
    uvel = (double*) calloc(3 * n, sizeof(double));
    uaccel = (double*) calloc(3 * n, sizeof(double));
    f_ext = (double*) calloc(3 * n, sizeof(double));
    f_extBase = (double*) calloc(3 * n, sizeof(double));

    loadInitialStates();
    loadScriptedExternalFroces();
    createForceModel();
    initializeTimeIntegrator();

    // delete the config information that is not needed further
    // delete femConfig;

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

    printf("Init simulator done\n");
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

    if (strcmp(femConfig->massSpringSystemObjConfigFilename, "__none") != 0)
    {
        femConfig->massSpringSystemSource = OBJ;
    }
    else if (strcmp(femConfig->massSpringSystemTetMeshConfigFilename, "__none") != 0)
    {
        femConfig->massSpringSystemSource = TETMESH;
    }
    else if (strcmp(femConfig->massSpringSystemCubicMeshConfigFilename, "__none") != 0)
    {
        femConfig->massSpringSystemSource = CUBICMESH;
    }
    else if (strncmp(femConfig->customMassSpringSystem, "chain", 5) == 0)
    {
        femConfig->massSpringSystemSource = CHAIN;
    }

    if ((femConfig->massSpringSystemSource == OBJ)
            || (femConfig->massSpringSystemSource == TETMESH)
            || (femConfig->massSpringSystemSource == CUBICMESH)
            || (femConfig->massSpringSystemSource == CHAIN))
    {
        femConfig->deformableObject = MASSSPRING;
    }

    if (femConfig->deformableObject == UNSPECIFIED)
    {
        printf("VEGA Error: no deformable model specified.\n");
        exit(1);
    }
}


// Load specified meshes
void smVegaFemSceneObject::loadMeshes()
{

    // load mesh
    if ((femConfig->deformableObject == STVK) || (femConfig->deformableObject == COROTLINFEM)
            || (femConfig->deformableObject == LINFEM)
            || (femConfig->deformableObject == INVERTIBLEFEM))
    {

        printf("VEGA: Loading volumetric mesh from file %s...\n", femConfig->volumetricMeshFilename);

        volumetricMesh = VolumetricMeshLoader::load(femConfig->volumetricMeshFilename);

        if (volumetricMesh == NULL)
        {
            printf("VEGA Error: unable to load the volumetric mesh from %s.\n",
                   femConfig->volumetricMeshFilename);
            exit(1);
        }

        n = volumetricMesh->getNumVertices();
        printf("VEGA: Num vertices: %d. Num elements: %d\n", n, volumetricMesh->getNumElements());
        meshGraph = GenerateMeshGraph::Generate(volumetricMesh);

        // load mass matrix
        if (strcmp(femConfig->massMatrixFilename, "__none") == 0)
        {
            printf("VEGA Error: mass matrix for the StVK deformable model not specified (%s).\n",
                   femConfig->massMatrixFilename);
            exit(1);
        }

        printf("VEGA: Loading the mass matrix from file %s...\n", femConfig->massMatrixFilename);
        // get the mass matrix
        SparseMatrixOutline * massMatrixOutline;

        try
        {
            // 3 is expansion flag to indicate this is a mass matrix; and does 3x3 identity block expansion
            massMatrixOutline = new SparseMatrixOutline(femConfig->massMatrixFilename, 3);
        }
        catch (int exceptionCode)
        {
            printf("VEGA: Error loading mass matrix %s.\n", femConfig->massMatrixFilename);
            exit(1);
        }

        massMatrix = new SparseMatrix(massMatrixOutline);
        delete(massMatrixOutline);

        if (femConfig->deformableObject == STVK || femConfig->deformableObject == LINFEM)  //LINFEM constructed from stVKInternalForces{
        {

            unsigned int loadingFlag = 0; // 0 = use low-memory version, 1 = use high-memory version
            StVKElementABCD * precomputedIntegrals = StVKElementABCDLoader::load(volumetricMesh, loadingFlag);

            if (precomputedIntegrals == NULL)
            {
                printf("VEGA Error: unable to load the StVK integrals.\n");
                exit(1);
            }

            printf("VEGA: Generating internal forces and stiffness matrix models...\n");
            fflush(NULL);

            if (femConfig->numInternalForceThreads == 0)
                stVKInternalForces = new StVKInternalForces(volumetricMesh,
                        precomputedIntegrals,
                        femConfig->addGravity,
                        femConfig->g);
            else
                stVKInternalForces = new StVKInternalForcesMT(volumetricMesh,
                        precomputedIntegrals, femConfig->addGravity,
                        femConfig->g, femConfig->numInternalForceThreads);

            if (femConfig->numInternalForceThreads == 0)
            {
                stVKStiffnessMatrix = new StVKStiffnessMatrix(stVKInternalForces);
            }
            else
                stVKStiffnessMatrix = new StVKStiffnessMatrixMT(stVKInternalForces,
                        femConfig->numInternalForceThreads);
        }
    }

    // load mass spring system (if any)
    if (femConfig->deformableObject == MASSSPRING)
    {

        switch (femConfig->massSpringSystemSource)
        {
        case OBJ:
        {
            printf("VEGA: Loading mass spring system from an obj file...\n");
            MassSpringSystemFromObjMeshConfigFile massSpringSystemFromObjMeshConfigFile;
            MassSpringSystemObjMeshConfiguration massSpringSystemObjMeshConfiguration;

            if (massSpringSystemFromObjMeshConfigFile.GenerateMassSpringSystem(
                        femConfig->massSpringSystemObjConfigFilename, &massSpringSystem,
                        &massSpringSystemObjMeshConfiguration) != 0)
            {
                printf("VEGA: Error initializing the mass spring system.\n");
                exit(1);
            }

            strcpy(femConfig->renderingMeshFilename,
                   massSpringSystemObjMeshConfiguration.massSpringMeshFilename);
        }
        break;

        case TETMESH:
        {
            printf("VEGA: Loading mass spring system from a tet mesh file...\n");
            MassSpringSystemFromTetMeshConfigFile massSpringSystemFromTetMeshConfigFile;
            MassSpringSystemTetMeshConfiguration massSpringSystemTetMeshConfiguration;

            if (massSpringSystemFromTetMeshConfigFile.GenerateMassSpringSystem(
                        femConfig->massSpringSystemTetMeshConfigFilename,
                        &massSpringSystem,
                        &massSpringSystemTetMeshConfiguration) != 0)
            {
                printf("VEGA: Error initializing the mass spring system.\n");
                exit(1);
            }

            strcpy(femConfig->renderingMeshFilename,
                   massSpringSystemTetMeshConfiguration.surfaceMeshFilename);
        }
        break;

        case CUBICMESH:
        {
            printf("VEGA: Loading mass spring system from a cubic mesh file...\n");
            MassSpringSystemFromCubicMeshConfigFile massSpringSystemFromCubicMeshConfigFile;
            MassSpringSystemCubicMeshConfiguration massSpringSystemCubicMeshConfiguration;

            if (massSpringSystemFromCubicMeshConfigFile.GenerateMassSpringSystem(
                        femConfig->massSpringSystemCubicMeshConfigFilename,
                        &massSpringSystem,
                        &massSpringSystemCubicMeshConfiguration) != 0)
            {
                printf("VEGA: Error initializing the mass spring system.\n");
                exit(1);
            }

            strcpy(femConfig->renderingMeshFilename,
                   massSpringSystemCubicMeshConfiguration.surfaceMeshFilename);
        }
        break;

        case CHAIN:
        {
            int numParticles;
            double groupStiffness;
            sscanf(femConfig->customMassSpringSystem, "chain,%d,%lf", &numParticles, &groupStiffness);
            printf("VEGA: Creating a chain mass-spring system with %d particles...\n", numParticles);

            double * masses = (double*) malloc(sizeof(double) * numParticles);

            for (int i = 0; i < numParticles; i++)
            {
                masses[i] = 1.0;
            }

            double * restPositions = (double*) malloc(sizeof(double) * 3 * numParticles);

            for (int i = 0; i < numParticles; i++)
            {
                restPositions[3 * i + 0] = 0;
                restPositions[3 * i + 1] = (numParticles == 1) ? 0.0 : 1.0 * i / (numParticles - 1);
                restPositions[3 * i + 2] = 0;
            }

            int * edges = (int*) malloc(sizeof(int) * 2 * (numParticles - 1));

            for (int i = 0; i < numParticles - 1; i++)
            {
                edges[2 * i + 0] = i;
                edges[2 * i + 1] = i + 1;
            }

            int * edgeGroups = (int*) malloc(sizeof(int) * (numParticles - 1));

            for (int i = 0; i < numParticles - 1; i++)
            {
                edgeGroups[i] = 0;
            }

            double groupDamping = 0;

            massSpringSystem = new MassSpringSystem(numParticles, masses, restPositions,
                                                    numParticles - 1, edges, edgeGroups,
                                                    1, &groupStiffness, &groupDamping,
                                                    femConfig->addGravity);

            char s[96];
            sprintf(s, "chain-%d.obj", numParticles);
            massSpringSystem->CreateObjMesh(s);
            strcpy(femConfig->renderingMeshFilename, s);

            free(edgeGroups);
            free(edges);
            free(restPositions);
            free(masses);

            //femConfig->renderVertices = 1;
        }
        break;

        default:
            printf("VEGA Error: mesh spring system configuration file was not specified.\n");
            exit(1);
            break;
        }//end switch

        if (femConfig->addGravity)
        {
            massSpringSystem->SetGravity(femConfig->addGravity, femConfig->g);
        }

        if (femConfig->numInternalForceThreads > 0)
        {
            printf("VEGA: Launching threaded internal force evaluation: %d threads.\n",
                   femConfig->numInternalForceThreads);
            MassSpringSystemMT * massSpringSystemMT = new MassSpringSystemMT(*massSpringSystem,
                    femConfig->numInternalForceThreads);
            delete(massSpringSystem);
            massSpringSystem = massSpringSystemMT;
        }

        n = massSpringSystem->GetNumParticles();

        // create the mass matrix
        massSpringSystem->GenerateMassMatrix(&massMatrix, 1);
        delete(massMatrix);
        massSpringSystem->GenerateMassMatrix(&massMatrix);

        // create the mesh graph (used only for the distribution of user forces over neighboring vertices)
        meshGraph = new Graph(massSpringSystem->GetNumParticles(),
                              massSpringSystem->GetNumEdges(), massSpringSystem->GetEdges());
    }

    int scaleRows = 1;
    meshGraph->GetLaplacian(&LaplacianDampingMatrix, scaleRows);
    LaplacianDampingMatrix->ScalarMultiply(femConfig->dampingLaplacianCoef);
}

// Load the rendering mesh if it is designated
void smVegaFemSceneObject::loadRenderingMesh()
{

    // initialize the rendering mesh for the volumetric mesh
    if (strcmp(femConfig->renderingMeshFilename, "__none") == 0)
    {
        printf("VEGA Error: rendering mesh was not specified.\n");
        exit(1);
    }

    deformableObjectRenderingMesh = new SceneObjectDeformable(femConfig->renderingMeshFilename);

    if (enableTextures)
    {
        deformableObjectRenderingMesh->SetUpTextures(SceneObject::MODULATE, SceneObject::NOMIPMAP);
    }

    deformableObjectRenderingMesh->ResetDeformationToRest();
    deformableObjectRenderingMesh->BuildNeighboringStructure();
    deformableObjectRenderingMesh->BuildNormals();
    deformableObjectRenderingMesh->SetMaterialAlpha(0.5);

    // initialize the embedded triangle rendering mesh
    secondaryDeformableObjectRenderingMesh = NULL;

    if (strcmp(femConfig->secondaryRenderingMeshFilename, "__none") != 0)
    {

        secondaryDeformableObjectRenderingMesh = new SceneObjectDeformable(femConfig->secondaryRenderingMeshFilename);

        if (enableTextures)
        {
            secondaryDeformableObjectRenderingMesh->SetUpTextures(SceneObject::MODULATE, SceneObject::NOMIPMAP);
        }

        secondaryDeformableObjectRenderingMesh->ResetDeformationToRest();
        secondaryDeformableObjectRenderingMesh->BuildNeighboringStructure();
        secondaryDeformableObjectRenderingMesh->BuildNormals();

        uSecondary = (double*) calloc(3 * secondaryDeformableObjectRenderingMesh->Getn(), sizeof(double));

        // load interpolation structure
        if (strcmp(femConfig->secondaryRenderingMeshInterpolationFilename, "__none") == 0)
        {
            printf("VEGA Error: no secondary rendering mesh interpolation filename specified.\n");
            exit(1);
        }

        secondaryDeformableObjectRenderingMesh_interpolation_numElementVertices =
            VolumetricMesh::getNumInterpolationElementVertices(femConfig->secondaryRenderingMeshInterpolationFilename);

        if (secondaryDeformableObjectRenderingMesh_interpolation_numElementVertices < 0)
        {
            printf("VEGA Error: unable to open file %s.\n", femConfig->secondaryRenderingMeshInterpolationFilename);
            exit(1);
        }

        printf("VEGA: Num interpolation element vertices: %d\n",
               secondaryDeformableObjectRenderingMesh_interpolation_numElementVertices);

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

    if (!((femConfig->deformableObject == MASSSPRING) && (femConfig->massSpringSystemSource == CHAIN)))
    {
        // read the fixed vertices
        // 1-indexed notation
        if (strcmp(femConfig->fixedVerticesFilename, "__none") == 0)
        {
            numFixedVertices = 0;
            fixedVertices = NULL;
        }
        else
        {
            if (LoadList::load(femConfig->fixedVerticesFilename, &numFixedVertices, &fixedVertices) != 0)
            {
                printf("VEGA: Error reading fixed vertices.\n");
                exit(1);
            }

            LoadList::sort(numFixedVertices, fixedVertices);
        }
    }
    else
    {
        numFixedVertices = 1;
        fixedVertices = (int*) malloc(sizeof(int) * numFixedVertices);
        fixedVertices[0] = massSpringSystem->GetNumParticles();
    }

    printf("VEGA: Loaded %d fixed vertices. They are:\n", numFixedVertices);
    LoadList::print(numFixedVertices, fixedVertices);

    // create 0-indexed fixed DOFs
    int numFixedDOFs = 3 * numFixedVertices;
    int * fixedDOFs = (int*) malloc(sizeof(int) * numFixedDOFs);

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

    printf("VEGA: Boundary vertices processed.\n");
}

// load initial displacements and velocities of the nodes
void smVegaFemSceneObject::loadInitialStates()
{

    // load initial condition
    if (strcmp(femConfig->initialPositionFilename, "__none") != 0)
    {
        int m1, n1;
        ReadMatrixFromDisk_(femConfig->initialPositionFilename, &m1, &n1, &uInitial);

        if ((m1 != 3 * n) || (n1 != 1))
        {
            printf("VEGA Error: initial position matrix size mismatch.\n");
            exit(1);
        }
    }
    else if ((femConfig->deformableObject == MASSSPRING) && (femConfig->massSpringSystemSource == CHAIN))
    {
        uInitial = (double*) calloc(3 * n, sizeof(double));
        int numParticles = massSpringSystem->GetNumParticles();

        for (int i = 0; i < numParticles; i++)
        {
            uInitial[3 * i + 0] = 1.0 - ((numParticles == 1) ? 1.0 : 1.0 * i / (numParticles - 1));
            uInitial[3 * i + 1] = 1.0 - ((numParticles == 1) ? 0.0 : 1.0 * i / (numParticles - 1));
            uInitial[3 * i + 2] = 0.0;
        }
    }
    else
    {
        uInitial = (double*) calloc(3 * n, sizeof(double));
    }

    // load initial velocity
    if (strcmp(femConfig->initialVelocityFilename, "__none") != 0)
    {
        int m1, n1;
        ReadMatrixFromDisk_(femConfig->initialVelocityFilename, &m1, &n1, &velInitial);

        if ((m1 != 3 * n) || (n1 != 1))
        {
            printf("VEGA Error: initial position matrix size mismatch.\n");
            exit(1);
        }
    }
}

// load the scripted externalloads
void smVegaFemSceneObject::loadScriptedExternalFroces()
{

    // load force loads
    if (strcmp(femConfig->forceLoadsFilename, "__none") != 0)
    {
        int m1;
        ReadMatrixFromDisk_(femConfig->forceLoadsFilename, &m1, &numForceLoads, &forceLoads);

        if (m1 != 3 * n)
        {
            printf("VEGA Error: Mismatch in the dimension of the force load matrix.\n");
            exit(1);
        }
    }
}

// Inititialize the time integrator
void smVegaFemSceneObject::initializeTimeIntegrator()
{

    int numFixedDOFs = 3 * numFixedVertices;
    int * fixedDOFs = (int*) malloc(sizeof(int) * numFixedDOFs);

    // initialize the integrator
    printf("VEGA: Initializing the integrator, n = %d...\n", n);
    printf("VEGA: Solver type: %s\n", femConfig->solverMethod);

    integratorBaseSparse = NULL;

    if (femConfig->solver == IMPLICITNEWMARK)
    {
        implicitNewmarkSparse = new ImplicitNewmarkSparse(3 * n, femConfig->timeStep,
                massMatrix, forceModel, positiveDefinite,
                numFixedDOFs, fixedDOFs,
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
        implicitNewmarkSparse = new ImplicitBackwardEulerSparse(3 * n, femConfig->timeStep,
                massMatrix, forceModel, positiveDefinite,
                numFixedDOFs, fixedDOFs,
                femConfig->dampingMassCoef,
                femConfig->dampingStiffnessCoef,
                femConfig->maxIterations, femConfig->epsilon,
                femConfig->numSolverThreads);
        integratorBaseSparse = implicitNewmarkSparse;
    }
    else if (femConfig->solver == EULER)
    {
        int symplectic = 0;
        integratorBaseSparse = new EulerSparse(3 * n, femConfig->timeStep,
                                               massMatrix, forceModel, symplectic,
                                               numFixedDOFs, fixedDOFs,
                                               femConfig->dampingMassCoef);
    }
    else if (femConfig->solver == SYMPLECTICEULER)
    {
        int symplectic = 1;
        integratorBaseSparse = new EulerSparse(3 * n, femConfig->timeStep, massMatrix,
                                               forceModel, symplectic, numFixedDOFs,
                                               fixedDOFs, femConfig->dampingMassCoef);
    }
    else if (femConfig->solver == CENTRALDIFFERENCES)
    {
        integratorBaseSparse = new CentralDifferencesSparse(3 * n, femConfig->timeStep,
                massMatrix, forceModel,
                numFixedDOFs, fixedDOFs,
                femConfig->dampingMassCoef,
                femConfig->dampingStiffnessCoef,
                femConfig->centralDifferencesTangentialDampingUpdateMode,
                femConfig->numSolverThreads);
    }

    integratorBase = integratorBaseSparse;

    if (integratorBase == NULL)
    {
        printf("VEGA Error: failed to initialize numerical integrator.\n");
        exit(1);
    }

    // set integration parameters
    integratorBaseSparse->SetDampingMatrix(LaplacianDampingMatrix);
    integratorBase->ResetToRest();
    integratorBase->SetState(uInitial, velInitial);
    integratorBase->SetTimestep(femConfig->timeStep / femConfig->substepsPerTimeStep);

    if (implicitNewmarkSparse != NULL)
    {
        implicitNewmarkSparse->UseStaticSolver(staticSolver);

        if (velInitial != NULL)
        {
            implicitNewmarkSparse->SetState(implicitNewmarkSparse->Getq(), velInitial);
        }
    }
}

// Create the force model
void smVegaFemSceneObject::createForceModel()
{

    // create force models, to be used by the integrator
    printf("VEGA: Creating force models...\n");

    if (femConfig->deformableObject == STVK)
    {
        stVKForceModel = new StVKForceModel(stVKInternalForces, stVKStiffnessMatrix);
        forceModel = stVKForceModel;
        stVKForceModel->GetInternalForce(uInitial, u);
    }

    if (femConfig->deformableObject == COROTLINFEM)
    {
        TetMesh * tetMesh = dynamic_cast<TetMesh*>(volumetricMesh);

        if (tetMesh == NULL)
        {
            printf("VEGA Error: the input mesh is not a tet mesh (CLFEM deformable model).\n");
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

        corotationalLinearFEMForceModel = new CorotationalLinearFEMForceModel(
            corotationalLinearFEM, femConfig->corotationalLinearFEM_warp);
        forceModel = corotationalLinearFEMForceModel;
    }

    if (femConfig->deformableObject == LINFEM)
    {
        LinearFEMForceModel * linearFEMForceModel = new LinearFEMForceModel(stVKInternalForces);
        forceModel = linearFEMForceModel;
    }

    if (femConfig->deformableObject == INVERTIBLEFEM)
    {
        TetMesh * tetMesh = dynamic_cast<TetMesh*>(volumetricMesh);

        if (tetMesh == NULL)
        {
            printf("VEGA Error: the input mesh is not a tet mesh (Invertible FEM deformable model).\n");
            exit(1);
        }

        IsotropicMaterial * isotropicMaterial = NULL;

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
            printf("VEGA: Invertible material: StVK.\n");
            break;
        }

        case INV_NEOHOOKEAN:
            isotropicMaterial = new NeoHookeanIsotropicMaterial(tetMesh,
                    femConfig->enableCompressionResistance,
                    femConfig->compressionResistance);
            printf("VEGA: Invertible material: neo-Hookean.\n");
            break;

        case INV_MOONEYRIVLIN:
            isotropicMaterial = new MooneyRivlinIsotropicMaterial(tetMesh,
                    femConfig->enableCompressionResistance,
                    femConfig->compressionResistance);
            printf("VEGA: Invertible material: Mooney-Rivlin.\n");
            break;

        default:
            printf("VEGA Error: invalid invertible material type.\n");
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
        IsotropicHyperelasticFEMForceModel * isotropicHyperelasticFEMForceModel =
            new IsotropicHyperelasticFEMForceModel(isotropicHyperelasticFEM);
        forceModel = isotropicHyperelasticFEMForceModel;
    }

    if (femConfig->deformableObject == MASSSPRING)
    {
        massSpringSystemForceModel = new MassSpringSystemForceModel(massSpringSystem);
        forceModel = massSpringSystemForceModel;
        renderMassSprings = new RenderSprings();
    }
}

// Update the deformations by time stepping
void smVegaFemSceneObject::advanceDynamics()
{

    cpuLoadCounter.StartCounter();

    // reset external forces (usually to zero)
    memcpy(f_ext, f_extBase, sizeof(double) * 3 * n);

    if ((!femConfig->lockScene) && (!femConfig->pauseSimulation) && (femConfig->singleStepMode <= 1))
    {

        // apply external user interaction forces
        applyUserInteractionForces();

        // apply pre-defined external forces
        applyScriptedExternalForces();

        // set forces to the integrator
        integratorBaseSparse->SetExternalForces(f_ext);

        // timestep the dynamics
        advanceOneTimeStep();

        timestepCounter++;

        memcpy(u, integratorBase->Getq(), sizeof(double) * 3 * n);

        if (femConfig->singleStepMode == 1)
        {
            femConfig->singleStepMode = 2;
        }

        //printf("VEGA: F"); fflush(NULL);
        graphicFrame++;

        if (femConfig->lockAt30Hz)
        {
            double elapsedTime;

            do
            {
                titleBarCounter.StopCounter();
                elapsedTime = titleBarCounter.GetElapsedTime();
            }
            while (1.0 * graphicFrame / elapsedTime >= 30.0);
        }
    }

    deformableObjectRenderingMesh->SetVertexDeformations(u);

    // update the secondary mesh
    updateSecondaryRenderingMesh();

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
        //printf("."); fflush(NULL);

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
            printf("VEGA: The integrator went unstable. Reduce the timestep, or increase the number of substeps per timestep.\n");
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
            printf("VEGA: Saving deformation to %s.\n", s);
            WriteMatrixToDisk_(s, 3 * n, 1, integratorBase->Getq());
            sprintf(s, "%s.f.%04d", femConfig->outputFilename, subTimestepCounter);
            printf("VEGA: Saving forces to %s.\n", s);
            WriteMatrixToDisk_(s, 3 * n, 1, integratorBase->GetExternalForces());
        }

        subTimestepCounter++;
    }
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
        set<int> affectedVertices;
        set<int> lastLayerVertices;

        affectedVertices.insert(pulledVertex);
        lastLayerVertices.insert(pulledVertex);

        for (int j = 1; j < femConfig->forceNeighborhoodSize; j++)
        {
            // linear kernel
            double forceMagnitude = 1.0 * (femConfig->forceNeighborhoodSize - j) / femConfig->forceNeighborhoodSize;

            set<int> newAffectedVertices;

            for (set<int> :: iterator iter = lastLayerVertices.begin(); iter != lastLayerVertices.end(); iter++)
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

            for (set<int> :: iterator iter = newAffectedVertices.begin(); iter != newAffectedVertices.end(); iter++)
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
    if (timestepCounter < numForceLoads)
    {
        printf("  External forces read from the binary input file.\n");

        for (int i = 0; i < 3 * n; i++)
        {
            f_ext[i] += forceLoads[ELT(3 * n, i, timestepCounter)];
        }
    }

}


// Use the computed displacemetnt update to interpolate to the secondary display mesh
inline void smVegaFemSceneObject::updateSecondaryRenderingMesh()
{

    // interpolate deformations from volumetric mesh to rendering triangle mesh
    if (secondaryDeformableObjectRenderingMesh != NULL)
    {
        VolumetricMesh::interpolate(
            u, uSecondary,
            secondaryDeformableObjectRenderingMesh->Getn(),
            secondaryDeformableObjectRenderingMesh_interpolation_numElementVertices,
            secondaryDeformableObjectRenderingMesh_interpolation_vertices,
            secondaryDeformableObjectRenderingMesh_interpolation_weights
        );
        secondaryDeformableObjectRenderingMesh->SetVertexDeformations(uSecondary);
    }

    if (femConfig->useRealTimeNormals)
    {
        // recompute normals
        deformableObjectRenderingMesh->BuildNormals();

        if (secondaryDeformableObjectRenderingMesh != NULL)
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
                        fps, (int)(100 * cpuLoad + 0.5));

            if (femConfig->deformableObject == COROTLINFEM)
                sprintf(windowTitle, "%s:%d | %s | Elements: %d | DOFs: %d | %.1f Hz |"
                        "Defo CPU Load: %d%%", "CLFEM", femConfig->corotationalLinearFEM_warp,
                        femConfig->solverMethod, volumetricMesh->getNumElements(),
                        volumetricMesh->getNumVertices() * 3, fps, (int)(100 * cpuLoad + 0.5));

            if (femConfig->deformableObject == LINFEM)
                sprintf(windowTitle, "%s | %s | Elements: %d | DOFs: %d | %.1f Hz |"
                        "Defo CPU Load: %d%%", "LinFEM", femConfig->solverMethod,
                        volumetricMesh->getNumElements(), volumetricMesh->getNumVertices() * 3, fps,
                        (int)(100 * cpuLoad + 0.5));

            if (femConfig->deformableObject == MASSSPRING)
                sprintf(windowTitle, "%s | %s | Particles: %d | Edges: %d | %.1f Hz |"
                        "Defo CPU Load: %d%%", "mass-spring", femConfig->solverMethod,
                        massSpringSystem->GetNumParticles(), massSpringSystem->GetNumEdges(), fps,
                        (int)(100 * cpuLoad + 0.5));

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
                        fps, (int)(100 * cpuLoad + 0.5));
            }

            glutSetWindowTitle(windowTitle);
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


// Displays the fem object with primary or secondary mesh, fixed vertices, vertices interacted with, ground plane etc.
void smVegaFemSceneObject::draw(smDrawParam p_params)
{

    glEnable(GL_LIGHTING);

    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    glStencilFunc(GL_ALWAYS, 0, ~(0u));

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

    glStencilFunc(GL_ALWAYS, 1, ~(0u));
    glColor3f(0, 0, 0);

    if (femConfig->renderWireframe)
    {
        deformableObjectRenderingMesh->RenderEdges();
    }

    // disable stencil buffer modifications
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    glColor3f(0, 0, 0);

    if (femConfig->renderAxes)
    {
        glLineWidth(1.0);
        drawAxes(1.0);
    }

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
        glVertex3f(pulledVertexPos[0], pulledVertexPos[1], pulledVertexPos[2]);
        glEnd();
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    // render model fixed vertices
    if (femConfig->renderFixedVertices)
    {
        for (int i = 0; i < numFixedVertices; i++)
        {
            glColor3f(1, 0, 0);
            double fixedVertexPos[3];
            deformableObjectRenderingMesh->GetSingleVertexRestPosition(fixedVertices[i],
                    &fixedVertexPos[0], &fixedVertexPos[1], &fixedVertexPos[2]);

            glEnable(GL_POLYGON_OFFSET_POINT);
            glPolygonOffset(-1.0, -1.0);
            glPointSize(12.0);
            glBegin(GL_POINTS);
            glVertex3f(fixedVertexPos[0], fixedVertexPos[1], fixedVertexPos[2]);
            glEnd();
            glDisable(GL_POLYGON_OFFSET_FILL);
        }
    }

    // render springs for mass-spring systems
    if ((massSpringSystem != NULL) & femConfig->renderSprings)
    {
        printf("VEGA: rendering springs\n");
        glLineWidth(2.0);
        renderMassSprings->Render(massSpringSystem, u);
        glLineWidth(1.0);
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
        sprintf(s, "The integrator went unstable.");
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
        print_bitmap_string(X1, Y1, -1, GLUT_BITMAP_9_BY_15 , s);
    }

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glutSwapBuffers();
}

//font is, for example, GLUT_BITMAP_9_BY_15
void smVegaFemSceneObject::print_bitmap_string(float x, float y, float z, void * font, char * s)
{

    glRasterPos3f(x, y, z);

    if (s && strlen(s))
    {
        while (*s)
        {
            glutBitmapCharacter(font, *s);
            s++;
        }
    }
}

// draw the cartesian axes.
void smVegaFemSceneObject::drawAxes(double axisLength)
{
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);

    glBegin(GL_LINES);

    for (int i = 0; i < 3; i++)
    {
        float color[3] = { 0, 0, 0 };
        color[i] = 1.0;
        glColor3fv(color);

        float vertex[3] = {0, 0, 0};
        vertex[i] = axisLength;
        glVertex3fv(vertex);
        glVertex3f(0, 0, 0);
    }

    glEnd();
}
