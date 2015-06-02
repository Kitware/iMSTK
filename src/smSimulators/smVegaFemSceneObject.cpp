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
	type = SIMMEDTK_SMVEGAFEMSCENEOBJECT;
}

smVegaFemSceneObject::smVegaFemSceneObject(std::shared_ptr<smErrorLog> p_log, smString ConfigFile)
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
    massSpringSystemForceModel = nullptr;
    corotationalLinearFEMForceModel = nullptr;
    positiveDefinite = 0;

	fpsBufferSize = 5; ///< buffer size to display fps
	forceAssemblyBufferSize = VEGA_PERFORMANCE_REC_BUFFER_SIZE;
	systemSolveBufferSize = VEGA_PERFORMANCE_REC_BUFFER_SIZE;

    volumetricMesh = nullptr;
    tetMesh = nullptr;
    meshGraph = nullptr;

    massSpringSystem = nullptr;
    renderMassSprings = nullptr;
    massMatrix = nullptr;
    LaplacianDampingMatrix = nullptr;

    //states
    u = nullptr;
    uvel = nullptr;
    uaccel = nullptr;
    f_ext = nullptr;
    f_extBase = nullptr;
    uSecondary = nullptr;
    uInitial = nullptr;
    velInitial = nullptr;

    deformableObjectRenderingMesh = nullptr;
    secondaryDeformableObjectRenderingMesh = nullptr;

    secondaryDeformableObjectRenderingMesh_interpolation_vertices = nullptr;
    secondaryDeformableObjectRenderingMesh_interpolation_weights = nullptr;

    renderUsingVega = true;
    importAndUpdateVolumeMeshToSmtk = false;

	type = SIMMEDTK_SMVEGAFEMSCENEOBJECT;

    if (ConfigFile.compare(vega_string_none) != 0)
    {
		femConfig = std::make_shared<smVegaConfigFemObject>();
        femConfig->setFemObjConfuguration(ConfigFile);
		std::cout << "VEGA: Initialized the VegaFemSceneObject and configured using file: " <<
			ConfigFile.c_str() << std::endl;

		initSimulation();
    }
    else
    {
		std::cout << "VEGA: NOTE: Initialized the VegaFemSceneObject, but not configured yet!" << std::endl;
    }
}

bool smVegaFemSceneObject::configure(smString ConfigFile)
{
	femConfig = std::make_shared<smVegaConfigFemObject>();

	if (ConfigFile.compare(vega_string_none) != 0)
	{
		femConfig->setFemObjConfuguration(ConfigFile);
		std::cout << "VEGA: Initialized the VegaFemSceneObject and configured using file!" <<
			ConfigFile.c_str() << std::endl;

		initSimulation();

		return 1;
	}
	else
	{
		std::cout<<"VEGA: Configured file invalid!"<< ConfigFile.c_str() << std::endl;
		return 0;
	}
}

smVegaFemSceneObject::~smVegaFemSceneObject()
{

    if (u != nullptr)
    {
        free(u);
    }

    if (uvel != nullptr)
    {
        free(uvel);
    }

    if (uaccel != nullptr)
    {
        free(uaccel);
    }

    if (f_ext != nullptr)
    {
        free(f_ext);
    }

    if (f_extBase != nullptr)
    {
        free(f_extBase);
    }

    if (uSecondary != nullptr)
    {
        free(uSecondary);
    }

    if (uInitial != nullptr)
    {
        free(uInitial);
    }

    if (velInitial != nullptr)
    {
        free(velInitial);
    }

    if (secondaryDeformableObjectRenderingMesh_interpolation_vertices != nullptr)
    {
        delete secondaryDeformableObjectRenderingMesh_interpolation_vertices;
    }

    if (secondaryDeformableObjectRenderingMesh_interpolation_weights != nullptr)
    {
        delete secondaryDeformableObjectRenderingMesh_interpolation_weights;
    }
}

// Initialize the parameters and properties of the simulation object
void smVegaFemSceneObject::initSimulation()
{

    volumetricMesh = nullptr;
    massSpringSystem = nullptr;

    setDeformableModel();
    loadVolumeMesh();
    loadSurfaceMesh();

    if(importAndUpdateVolumeMeshToSmtk)
    {
        this->smtkVolumeMesh = std::make_shared<smVolumeMesh>();
        this->smtkVolumeMesh->importVolumeMeshDataFromVEGA_Format(this->volumetricMesh, true);
    }

    if(!renderUsingVega)
    {
        this->smtkSurfaceMesh = std::make_shared<smSurfaceMesh>();
        if(strcmp(femConfig->secondaryRenderingMeshFilename, "__none") == 0)
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
    u = new double[3*n]();
    uvel = new double[3*n]();
    uaccel = new double[3*n]();
    f_ext = new double[3*n]();
    f_extBase = new double[3*n]();

    f_contact.resize(3*n);

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
void smVegaFemSceneObject::loadVolumeMesh()
{

    // load mesh
    if ((femConfig->deformableObject == STVK) || (femConfig->deformableObject == COROTLINFEM)
            || (femConfig->deformableObject == LINFEM)
            || (femConfig->deformableObject == INVERTIBLEFEM))
    {

        printf("VEGA: Loading volumetric mesh from file %s...\n", femConfig->volumetricMeshFilename);

        volumetricMesh = std::shared_ptr<VolumetricMesh>(VolumetricMeshLoader::load(femConfig->volumetricMeshFilename));

        if (volumetricMesh == nullptr)
        {
            printf("VEGA Error: unable to load the volumetric mesh from %s.\n",
                   femConfig->volumetricMeshFilename);
            exit(1);
        }

        n = volumetricMesh->getNumVertices();
        printf("VEGA: Num vertices: %d. Num elements: %d\n", n, volumetricMesh->getNumElements());
        meshGraph = std::shared_ptr<Graph>(GenerateMeshGraph::Generate(volumetricMesh.get()));

        // load mass matrix
        if (strcmp(femConfig->massMatrixFilename, "__none") == 0)
        {
            printf("VEGA Error: mass matrix for the StVK deformable model not specified (%s).\n",
                   femConfig->massMatrixFilename);
            exit(1);
        }

		printf("VEGA: Loading the mass matrix from file %s...\n", femConfig->massMatrixFilename);
        // get the mass matrix
		std::shared_ptr<SparseMatrixOutline> massMatrixOutline;

        try
        {
            // 3 is expansion flag to indicate this is a mass matrix; and does 3x3 identity block expansion
            massMatrixOutline = std::make_shared<SparseMatrixOutline>(femConfig->massMatrixFilename, 3);
        }
        catch (int exceptionCode)
        {
            printf("VEGA: Error loading mass matrix %s.\n", femConfig->massMatrixFilename);
            exit(1);
        }

        massMatrix = std::make_shared<SparseMatrix>(massMatrixOutline.get());

        if (femConfig->deformableObject == STVK || femConfig->deformableObject == LINFEM)  //LINFEM constructed from stVKInternalForces{
        {

            unsigned int loadingFlag = 0; // 0 = use low-memory version, 1 = use high-memory version
            std::shared_ptr<StVKElementABCD> precomputedIntegrals = std::shared_ptr<StVKElementABCD>(StVKElementABCDLoader::load(volumetricMesh.get(), loadingFlag));

            if (precomputedIntegrals == nullptr)
            {
                printf("VEGA Error: unable to load the StVK integrals.\n");
                exit(1);
            }

            printf("VEGA: Generating internal forces and stiffness matrix models...\n");
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

    if (femConfig->deformableObject == MASSSPRING)
    {

        switch (femConfig->massSpringSystemSource)
        {
        case OBJ:
        {
            printf("VEGA: Loading mass spring system from an obj file...\n");
            MassSpringSystemFromObjMeshConfigFile massSpringSystemFromObjMeshConfigFile;
            MassSpringSystemObjMeshConfiguration massSpringSystemObjMeshConfiguration;

			MassSpringSystem *mss = massSpringSystem.get();
            if (massSpringSystemFromObjMeshConfigFile.GenerateMassSpringSystem(
                        femConfig->massSpringSystemObjConfigFilename, &mss,
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

			MassSpringSystem *mss = massSpringSystem.get();
            if (massSpringSystemFromTetMeshConfigFile.GenerateMassSpringSystem(
                        femConfig->massSpringSystemTetMeshConfigFilename,
                        &mss,
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

			MassSpringSystem *mss = massSpringSystem.get();
            if (massSpringSystemFromCubicMeshConfigFile.GenerateMassSpringSystem(
                        femConfig->massSpringSystemCubicMeshConfigFilename,
                        &mss,
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

            std::vector<double> masses(numParticles);

            for (int i = 0; i < numParticles; i++)
            {
                masses[i] = 1.0;
            }

            std::vector<double> restPositions(3 * numParticles);

            for (int i = 0; i < numParticles; i++)
            {
                restPositions[3 * i + 0] = 0;
                restPositions[3 * i + 1] = (numParticles == 1) ? 0.0 : 1.0 * i / (numParticles - 1);
                restPositions[3 * i + 2] = 0;
            }

            std::vector<int> edges(2 * (numParticles - 1));

            for (int i = 0; i < numParticles - 1; i++)
            {
                edges[2 * i + 0] = i;
                edges[2 * i + 1] = i + 1;
            }

            std::vector<int> edgeGroups(numParticles - 1);

            for (int i = 0; i < numParticles - 1; i++)
            {
                edgeGroups[i] = 0;
            }

            double groupDamping = 0;

            massSpringSystem = std::make_shared<MassSpringSystem>(numParticles, masses.data(), restPositions.data(),
                                                    numParticles - 1, edges.data(), edgeGroups.data(),
                                                    1, &groupStiffness, &groupDamping,
                                                    femConfig->addGravity);

            char s[96];
            sprintf(s, "chain-%d.obj", numParticles);
            massSpringSystem->CreateObjMesh(s);
            strcpy(femConfig->renderingMeshFilename, s);
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
            auto massSpringSystemMT = std::make_shared<MassSpringSystemMT>(*massSpringSystem.get(),
                    femConfig->numInternalForceThreads);
            massSpringSystem = massSpringSystemMT;
        }

        n = massSpringSystem->GetNumParticles();

        // create the mass matrix
		SparseMatrix *sm = massMatrix.get();
		massSpringSystem->GenerateMassMatrix(&sm, 1);
        massSpringSystem->GenerateMassMatrix(&sm);

        // create the mesh graph (used only for the distribution of user forces over neighboring vertices)
        meshGraph = std::make_shared<Graph>(massSpringSystem->GetNumParticles(),
                              massSpringSystem->GetNumEdges(), massSpringSystem->GetEdges());
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
        printf("VEGA Error: rendering mesh was not specified.\n");
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

        secondaryDeformableObjectRenderingMesh->ResetDeformationToRest();
        secondaryDeformableObjectRenderingMesh->BuildNeighboringStructure();
        secondaryDeformableObjectRenderingMesh->BuildNormals();

        uSecondary = new double[3 * secondaryDeformableObjectRenderingMesh->Getn()]();

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
    int numFixedVertices = fixedVertices.size();
    if (!((femConfig->deformableObject == MASSSPRING) && (femConfig->massSpringSystemSource == CHAIN)))
    {
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
    }
    else
    {
        fixedVertices.resize(1);
        fixedVertices[0] = massSpringSystem->GetNumParticles();
    }

    printf("VEGA: Loaded %d fixed vertices. They are:\n", numFixedVertices);
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
        uInitial = new double[3*n]();
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
        uInitial = new double[3*n]();
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
    int numForceLoads = forceLoads.size();
    if (strcmp(femConfig->forceLoadsFilename, "__none") != 0)
    {
        int m1;
        double *data = forceLoads.data();
        ReadMatrixFromDisk_(femConfig->forceLoadsFilename, &m1, &numForceLoads, &data);

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

    int numFixedDOFs = 3 * this->fixedVertices.size();
    std::vector<int> fixedDOFs(numFixedDOFs,0);

    // initialize the integrator
    printf("VEGA: Initializing the integrator, n = %d...\n", n);
    printf("VEGA: Solver type: %s\n", femConfig->solverMethod);

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
        printf("VEGA Error: failed to initialize numerical integrator.\n");
        exit(1);
    }

    // set integration parameters
    integratorBaseSparse->SetDampingMatrix(LaplacianDampingMatrix.get());
    integratorBase->ResetToRest();
    integratorBase->SetState(uInitial, velInitial);
    integratorBase->SetTimestep(femConfig->timeStep / femConfig->substepsPerTimeStep);

    if (implicitNewmarkSparse != nullptr)
    {
        implicitNewmarkSparse->UseStaticSolver(staticSolver);

        if (velInitial != nullptr)
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
		stVKForceModel = std::make_shared<StVKForceModel>(stVKInternalForces.get(), stVKStiffnessMatrix.get());
        forceModel = stVKForceModel;
        stVKForceModel->GetInternalForce(uInitial, u);
    }

    if (femConfig->deformableObject == COROTLINFEM)
    {
        TetMesh * tetMesh = dynamic_cast<TetMesh*>(volumetricMesh.get());

        if (tetMesh == nullptr)
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

		corotationalLinearFEMForceModel = std::make_shared<CorotationalLinearFEMForceModel>(
            corotationalLinearFEM, femConfig->corotationalLinearFEM_warp);
        forceModel = corotationalLinearFEMForceModel;
    }

    if (femConfig->deformableObject == LINFEM)
    {
		std::shared_ptr<LinearFEMForceModel> linearFEMForceModel = std::make_shared<LinearFEMForceModel>(stVKInternalForces.get());
        forceModel = linearFEMForceModel;
    }

    if (femConfig->deformableObject == INVERTIBLEFEM)
    {
        TetMesh * tetMesh = dynamic_cast<TetMesh*>(volumetricMesh.get());

        if (tetMesh == nullptr)
        {
            printf("VEGA Error: the input mesh is not a tet mesh (Invertible FEM deformable model).\n");
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
        std::shared_ptr<IsotropicHyperelasticFEMForceModel> isotropicHyperelasticFEMForceModel =
			std::make_shared<IsotropicHyperelasticFEMForceModel>(isotropicHyperelasticFEM);
        forceModel = isotropicHyperelasticFEMForceModel;
    }

    if (femConfig->deformableObject == MASSSPRING)
    {
		massSpringSystemForceModel = std::make_shared<MassSpringSystemForceModel>(massSpringSystem.get());
        forceModel = massSpringSystemForceModel;
		renderMassSprings = std::make_shared<RenderSprings>();
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

        // apply external forces arising from contact
        applyContactForces();

        // set forces to the integrator
        integratorBaseSparse->SetExternalForces(f_ext);

        // timestep the dynamics
        advanceOneTimeStep();

        timestepCounter++;

        memcpy(u, integratorBase->Getq(), sizeof(double) * 3 * n);

        if(importAndUpdateVolumeMeshToSmtk)
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
            }
            while (1.0 * graphicFrame / elapsedTime >= 30.0);
        }
    }

    deformableObjectRenderingMesh->SetVertexDeformations(u);

    // update the secondary mesh
    updateSecondaryRenderingMesh();

    if(!renderUsingVega)
    {

        if(strcmp(femConfig->secondaryRenderingMeshFilename, "__none") == 0)
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
        //printf("."); fflush(nullptr);

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


void smVegaFemSceneObject::applyContactForces()
{
    if(f_contact.size() != 0)
    {
        for(int i=0; i<f_contact.size(); i++)
        {
            f_ext[i] += f_contact[i];
        }
    }
}


void smVegaFemSceneObject::setContactForcesToZero()
{
    f_contact.assign(f_contact.size(),0.0);
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

            for (std::set<int> :: iterator iter = lastLayerVertices.begin(); iter != lastLayerVertices.end(); iter++)
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

            for (std::set<int> :: iterator iter = newAffectedVertices.begin(); iter != newAffectedVertices.end(); iter++)
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
        printf("  External forces read from the binary input file.\n");

        size_t offset = 3*n*timestepCounter;
        for (size_t i = 0, end = 3u*n; i < end; i++)
        {
            f_ext[i] += forceLoads[offset+i];
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

        graphicFrame = 0;

        if ((femConfig->syncTimestepWithGraphics) && ((!femConfig->lockScene) &&
                (!femConfig->pauseSimulation) && (femConfig->singleStepMode == 0)))
        {
            femConfig->timeStep = 1.0 / fps;
            integratorBase->SetTimestep(femConfig->timeStep / femConfig->substepsPerTimeStep);
        }
    }
}

void smVegaFemSceneObject::draw(const smDrawParam &p_params)
{
    smtkSurfaceMesh->draw(p_params);
}
