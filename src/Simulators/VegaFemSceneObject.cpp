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
#include "VegaFemSceneObject.h"
#include "Core/Factory.h"
#include "Core/RenderDelegate.h"

#include <exception>

smVegaFemSceneObject::smVegaFemSceneObject() :
    staticSolver(0),
    graphicFrame(0),
    explosionFlag(0),
    positiveDefinite(0),
    importAndUpdateVolumeMeshToSmtk(false)
{
    performaceTracker.initialize();

    this->type = SIMMEDTK_SMVEGAFEMSCENEOBJECT;

    this->name = "VegaFem_SceneObject_" + std::to_string(this->getUniqueId()->getId());

    this->setRenderDelegate(
      smFactory<smRenderDelegate>::createConcreteClass(
        "SceneObjectDeformableRenderDelegate"));
}

smVegaFemSceneObject::smVegaFemSceneObject(const std::shared_ptr<smErrorLog> p_log,
                                           const smString ConfigFile):
                                           staticSolver(0),
                                           graphicFrame(0),
                                           explosionFlag(0),
                                           positiveDefinite(0),
                                           importAndUpdateVolumeMeshToSmtk(false)
{
    performaceTracker.initialize();

    this->type = SIMMEDTK_SMVEGAFEMSCENEOBJECT;

    this->name = "VegaFem_SceneObject_" + std::to_string(this->getUniqueId()->getId());

    // Create the default delegate before configuration so it
    // can be overridden.
    this->setRenderDelegate(
      smFactory<smRenderDelegate>::createConcreteClass(
        "SceneObjectDeformableRenderDelegate"));

    ConfigFileName = ConfigFile;

    configure(ConfigFileName);

    initialize();
}

smVegaFemSceneObject::~smVegaFemSceneObject()
{
}

std::shared_ptr<smSceneObject> smVegaFemSceneObject::clone()
{
    return safeDownCast<smSceneObject>();
}

bool smVegaFemSceneObject::configure(const smString ConfigFile)
{
    femConfig = std::make_shared<smVegaObjectConfig>();

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
        vegaPrimarySurfaceMesh->ResetDeformationToRest();

        performaceTracker.clearFpsBuffer();
    }
    else
    {
        PRINT_ERROR_LOCATION
        std::cout << "VEGA: error! Initial state undetermined as the topology is altered.\n";
    }
}

void smVegaFemSceneObject::initialize()
{
    setDeformableModelType();
    loadVolumeMesh();
    loadSurfaceMesh();

    if (importAndUpdateVolumeMeshToSmtk)
    {
        this->volumeMesh = std::make_shared<smVolumeMesh>();
        this->volumeMesh->importVolumeMeshFromVegaFormat(this->volumetricMesh, true);
    }

    this->primarySurfaceMesh = std::make_shared<smSurfaceMesh>();

    this->primarySurfaceMesh->importSurfaceMeshFromVegaFormat(
        this->vegaPrimarySurfaceMesh->GetMesh(), true);

    if (
      femConfig->secondaryRenderingMeshFilename[0] &&
      (strcmp(femConfig->secondaryRenderingMeshFilename, "__none") != 0))
    {
        this->secondarySurfaceMesh = std::make_shared<smSurfaceMesh>();

        this->secondarySurfaceMesh->importSurfaceMeshFromVegaFormat(
            this->vegaSecondarySurfaceMesh->GetMesh(), true);

        this->setRenderSecondaryMesh();
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
    performaceTracker.objectPerformanceCounter.StartCounter();

    std::cout << "Inititializing fem object done. \n";

    primarySurfaceMesh->printPrimitiveDetails();

    primarySurfaceMesh->initVertexNeighbors();
    primarySurfaceMesh->updateTriangleNormals();
    primarySurfaceMesh->updateVertexNormals();

    if (strcmp(femConfig->secondaryRenderingMeshFilename, "__none") != 0)
    {
        secondarySurfaceMesh->printPrimitiveDetails();

        secondarySurfaceMesh->initVertexNeighbors();
        secondarySurfaceMesh->updateTriangleNormals();
        secondarySurfaceMesh->updateVertexNormals();
    }
}

void smVegaFemSceneObject::setDeformableModelType()
{

    // set deformable material type
    if (strcmp(femConfig->volumetricMeshFilename, "__none") != 0)
    {
        if (strcmp(femConfig->deformableObjectMethod, "StVK") == 0)
        {
            femConfig->deformableObject = smVegaObjectConfig::STVK;
        }

        if (strcmp(femConfig->deformableObjectMethod, "CLFEM") == 0)
        {
            femConfig->deformableObject = smVegaObjectConfig::COROTLINFEM;
        }

        if (strcmp(femConfig->deformableObjectMethod, "LinearFEM") == 0)
        {
            femConfig->deformableObject = smVegaObjectConfig::LINFEM;
        }

        if (strcmp(femConfig->deformableObjectMethod, "InvertibleFEM") == 0)
        {
            femConfig->deformableObject = smVegaObjectConfig::INVERTIBLEFEM;
        }
    }

    if (femConfig->deformableObject == smVegaObjectConfig::UNSPECIFIED)
    {
        PRINT_ERROR_LOCATION
        std::cout << "VEGA: error! no deformable model specified." << std::endl;
    }
}

void smVegaFemSceneObject::loadVolumeMesh()
{

    // load mesh
    if ((femConfig->deformableObject == smVegaObjectConfig::STVK) ||
        (femConfig->deformableObject == smVegaObjectConfig::COROTLINFEM) ||
        (femConfig->deformableObject == smVegaObjectConfig::LINFEM) ||
        (femConfig->deformableObject == smVegaObjectConfig::INVERTIBLEFEM))
    {

        std::cout << "VEGA: Loading volumetric mesh from file "
                  << femConfig->volumetricMeshFilename << "..." << std::endl;

        volumetricMesh = std::shared_ptr<VolumetricMesh>(
            VolumetricMeshLoader::load(femConfig->volumetricMeshFilename));

        if (volumetricMesh == nullptr)
        {
            PRINT_ERROR_LOCATION
            std::cout << "VEGA: error! unable to load the volumetric mesh from"
                                        << femConfig->volumetricMeshFilename << std::endl;
        }

        numNodes = volumetricMesh->getNumVertices();
        std::cout << "VEGA: Num vertices: " << numNodes <<
                         ". Num elements: " << volumetricMesh->getNumElements() << std::endl;

        meshGraph = std::shared_ptr<Graph>(GenerateMeshGraph::Generate(volumetricMesh.get()));

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
                GenerateMassMatrix::computeMassMatrix(volumetricMesh.get(), &tempMassMatrix, false);

                std::cout << femConfig->massMatrixFilename << std::endl;
                if (tempMassMatrix->Save(femConfig->massMatrixFilename)!=0)
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
        std::shared_ptr<SparseMatrixOutline> massMatrixOutline;

        try
        {
            // 3 is expansion flag to indicate this is a mass matrix; and does 3x3 identity block expansion
            massMatrixOutline = std::make_shared<SparseMatrixOutline>(femConfig->massMatrixFilename, 3);
        }
        catch (int exceptionCode)
        {
            PRINT_ERROR_LOCATION
            std::cout << "VEGA:  error! loading mass matrix"
                        << femConfig->massMatrixFilename << std::endl;
        }

        massMatrix = std::make_shared<SparseMatrix>(massMatrixOutline.get());

        if (femConfig->deformableObject == smVegaObjectConfig::STVK ||
            femConfig->deformableObject == smVegaObjectConfig::LINFEM)
        {

            unsigned int loadingFlag = 0; // 0 = use low-memory version, 1 = use high-memory version
            std::shared_ptr<StVKElementABCD> precomputedIntegrals =
                std::shared_ptr<StVKElementABCD>(StVKElementABCDLoader::load(volumetricMesh.get(), loadingFlag));

            if (precomputedIntegrals == nullptr)
            {
                PRINT_ERROR_LOCATION
                std::cout << "VEGA: error! unable to load the StVK integrals.\n";
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

    if (meshGraph)
    {
        int scaleRows = 1;
        SparseMatrix *sm;
        meshGraph->GetLaplacian(&sm, scaleRows);
        LaplacianDampingMatrix.reset(sm);
        LaplacianDampingMatrix->ScalarMultiply(femConfig->dampingLaplacianCoef);
    }
}

void smVegaFemSceneObject::loadSurfaceMesh()
{
    // initialize the rendering mesh for the volumetric mesh
    if (strcmp(femConfig->renderingMeshFilename, "__none") == 0)
    {
        std::cout << "VEGA: rendering mesh was not specified!\n";

        std::string rendFilenameStr(femConfig->volumetricMeshFilename);
        rendFilenameStr += ".mass";
        std::ifstream renderingFileName(rendFilenameStr);

        if (!renderingFileName.good())
        {
            std::cout << "VEGA: Generating primary rendering mesh.\n";

            std::shared_ptr<ObjMesh> objMesh(GenerateSurfaceMesh::ComputeMesh(volumetricMesh.get(), false));

            objMesh->save(femConfig->renderingMeshFilename);

        }
        else
        {
            std::cout << "VEGA: Loading primary rendering mesh: "<< femConfig->renderingMeshFilename <<" \n";
        }
    }

    vegaPrimarySurfaceMesh = std::make_shared<smVegaSceneObjectDeformable>(femConfig->renderingMeshFilename);
    if (!vegaPrimarySurfaceMesh->GetMesh())
      return; // Go no further if given an invalid filename

    vegaPrimarySurfaceMesh->ResetDeformationToRest();
    vegaPrimarySurfaceMesh->BuildNeighboringStructure();
    vegaPrimarySurfaceMesh->BuildNormals();
    //vegaPrimarySurfaceMesh->SetMaterialAlpha(0.5);

    // initialize the embedded triangle rendering mesh
    vegaSecondarySurfaceMesh = nullptr;

    if (strcmp(femConfig->secondaryRenderingMeshFilename, "__none") != 0)
    {
        vegaSecondarySurfaceMesh = std::make_shared<smVegaSceneObjectDeformable>(
                                                    femConfig->secondaryRenderingMeshFilename);

        if (vegaSecondarySurfaceMesh == nullptr)
        {
            std::cout << "VEGA: Secondary rendering mesh is not initialized!\n";
            throw std::runtime_error("No secondary rendering mesh provided.");
        }
        else
        {
            std::cout << "VEGA: Secondary rendering mesh is initialized:\n\t\t"
                << vegaSecondarySurfaceMesh->GetNumVertices() << " vertices\n\t\t"
                << vegaSecondarySurfaceMesh->GetNumFaces() << " faces\n";
        }

        vegaSecondarySurfaceMesh->ResetDeformationToRest();
        vegaSecondarySurfaceMesh->BuildNeighboringStructure();
        vegaSecondarySurfaceMesh->BuildNormals();

        uSecondary.resize(3 * vegaSecondarySurfaceMesh->Getn());

        // load interpolation structure
        if (strcmp(femConfig->secondaryRenderingMeshInterpolationFilename, "__none") == 0)
        {
            PRINT_ERROR_LOCATION
            std::cout << "VEGA:  error! no secondary rendering mesh interpolation filename specified.\n";
        }

        numInterpolationElementVerts =
            VolumetricMesh::getNumInterpolationElementVertices(
            femConfig->secondaryRenderingMeshInterpolationFilename);

        if (numInterpolationElementVerts < 0)
        {
            PRINT_ERROR_LOCATION
            std::cout << "VEGA: error! unable to open file " <<
                femConfig->secondaryRenderingMeshInterpolationFilename << "." << std::endl;
        }

        std::cout << "VEGA: Num interpolation element vertices:" <<
            numInterpolationElementVerts << std::endl;

        int *vertices = interpolationVertices.data();
        double *weights = interpolationWeights.data();
        VolumetricMesh::loadInterpolationWeights(
            femConfig->secondaryRenderingMeshInterpolationFilename,
            vegaSecondarySurfaceMesh->Getn(),
            numInterpolationElementVerts,
            &vertices,
            &weights
            );
    }
}

int smVegaFemSceneObject::readBcFromFile(const char* filename, const int offset)
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

void smVegaFemSceneObject::loadFixedBC()
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
            PRINT_ERROR_LOCATION
            std::cout << "VEGA: error! Mismatch in the dimension of the force load matrix.\n";
        }
    }
}

void smVegaFemSceneObject::initializeTimeIntegrator()
{

    int numFixedDOFs = 3 * numFixedNodes;
    std::vector<int> fixedDOFs(numFixedDOFs, 0);

    // initialize the integrator
    std::cout << "VEGA: Initializing the integrator, n = " << numNodes << "...\n";
    std::cout << "VEGA: Solver type - " << femConfig->solverMethod << std::endl;

    integratorBaseSparse = nullptr;

    if (femConfig->solver == smVegaObjectConfig::IMPLICITNEWMARK)
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
    else if (femConfig->solver == smVegaObjectConfig::IMPLICITBACKWARDEULER)
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
    else if (femConfig->solver == smVegaObjectConfig::EULER)
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
    else if (femConfig->solver == smVegaObjectConfig::SYMPLECTICEULER)
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
    else if (femConfig->solver == smVegaObjectConfig::CENTRALDIFFERENCES)
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

void smVegaFemSceneObject::createForceModel()
{

    // create force models, to be used by the integrator
    std::cout << "VEGA: Creating force models..." << std::endl;

    switch (femConfig->deformableObject)
    {
    case smVegaObjectConfig::STVK:
    {

        stVKForceModel = std::make_shared<StVKForceModel>(stVKInternalForces.get(),
                                                          stVKStiffnessMatrix.get());
        forceModel = stVKForceModel;
        stVKForceModel->GetInternalForce(uInitial.data(), u.data());

        break;
    }
    case smVegaObjectConfig::COROTLINFEM:
    {
        TetMesh * tetMesh = dynamic_cast<TetMesh*>(volumetricMesh.get());

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

        corotationalLinearFEMForceModel = std::make_shared<CorotationalLinearFEMForceModel>(
                                            corotationalLinearFEM, femConfig->corotationalLinearFEM_warp);
        forceModel = corotationalLinearFEMForceModel;

        break;
    }
    case smVegaObjectConfig::LINFEM:
    {
        auto linearFEMForceModel =
                    std::make_shared<LinearFEMForceModel>(stVKInternalForces.get());
        forceModel = linearFEMForceModel;

        break;
    }
    case smVegaObjectConfig::INVERTIBLEFEM:
    {
        TetMesh * tetMesh = dynamic_cast<TetMesh*>(volumetricMesh.get());

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
        case smVegaObjectConfig::INV_STVK:
        {

            isotropicMaterial = new StVKIsotropicMaterial(
                tetMesh,
                femConfig->enableCompressionResistance,
                femConfig->compressionResistance);
            std::cout << "VEGA: Invertible material: StVK.\n";
            break;
        }

        case smVegaObjectConfig::INV_NEOHOOKEAN:
            isotropicMaterial = new NeoHookeanIsotropicMaterial(
                tetMesh,
                femConfig->enableCompressionResistance,
                femConfig->compressionResistance);
            std::cout << "VEGA: Invertible material: neo-Hookean.\n";
            break;

        case smVegaObjectConfig::INV_MOONEYRIVLIN:
            isotropicMaterial = new MooneyRivlinIsotropicMaterial(
                tetMesh,
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
                tetMesh, isotropicMaterial,
                femConfig->inversionThreshold,
                femConfig->addGravity, femConfig->g);
        }
        else
        {
            isotropicHyperelasticFEM = new IsotropicHyperelasticFEMMT(
                tetMesh,
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

void smVegaFemSceneObject::advanceDynamics()
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

        if (importAndUpdateVolumeMeshToSmtk)
        {
            this->volumeMesh->updateVolumeMeshFromVegaFormat(this->volumetricMesh);
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
                performaceTracker.objectPerformanceCounter.StopCounter();
                elapsedTime = performaceTracker.objectPerformanceCounter.GetElapsedTime();
            } while (1.0 * graphicFrame / elapsedTime >= 30.0);
        }
    }

    vegaPrimarySurfaceMesh->SetVertexDeformations(u.data());

    // update the secondary mesh
    updateSecondaryRenderingMesh();

    if (strcmp(femConfig->secondaryRenderingMeshFilename, "__none") == 0)
    {
        this->primarySurfaceMesh->updateSurfaceMeshFromVegaFormat(
            this->vegaPrimarySurfaceMesh->GetMesh());

        primarySurfaceMesh->updateTriangleNormals();
        primarySurfaceMesh->updateVertexNormals();
    }
    else
    {
        this->primarySurfaceMesh->updateSurfaceMeshFromVegaFormat(
            this->vegaPrimarySurfaceMesh->GetMesh());

        if (!renderSecondaryMesh)
        {
            primarySurfaceMesh->updateTriangleNormals();
            primarySurfaceMesh->updateVertexNormals();
        }
        else
        {
            this->secondarySurfaceMesh->updateSurfaceMeshFromVegaFormat(
                this->vegaSecondarySurfaceMesh->GetMesh());

            secondarySurfaceMesh->updateTriangleNormals();
            secondarySurfaceMesh->updateVertexNormals();
        }
    }
    // update stasts
    updatePerformanceMetrics();
}

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
            std::cout << "VEGA: The integrator went unstable."
                <<"Reduce the timestep, or increase the number of substeps per timestep.\n";
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

void smVegaFemSceneObject::setPulledVertex(const smVec3d &userPos)
{
    pulledVertex = this->volumetricMesh->getClosestVertex(Vec3d(userPos(0), userPos(1), userPos(2)));
}

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

inline void smVegaFemSceneObject::applyScriptedExternalForces()
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

inline void smVegaFemSceneObject::updateSecondaryRenderingMesh()
{
    // interpolate deformations from volumetric mesh to rendering triangle mesh
    if (vegaSecondarySurfaceMesh != nullptr)
    {
        VolumetricMesh::interpolate(
            u.data(), uSecondary.data(),
            vegaSecondarySurfaceMesh->Getn(),
            numInterpolationElementVerts,
            interpolationVertices.data(),
            interpolationWeights.data()
            );
        vegaSecondarySurfaceMesh->SetVertexDeformations(uSecondary.data());
    }

    // recompute normals
    if (vegaSecondarySurfaceMesh != nullptr)
    {
        vegaSecondarySurfaceMesh->BuildNormals();
    }
    else
    {
        vegaPrimarySurfaceMesh->BuildNormals();
    }

    if (explosionFlag)
    {
        performaceTracker.explosionCounter.StopCounter();
        // the message will appear on screen for 4 seconds
        if (performaceTracker.explosionCounter.GetElapsedTime() > 4.0)
        {
            explosionFlag = 0;
        }
    }
}

inline void smVegaFemSceneObject::updatePerformanceMetrics()
{
    smVegaPerformanceCounter *pt = &performaceTracker;
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

void smVegaFemSceneObject::printInfo() const
{
    std::cout << "\t-------------------------------------\n";
    std::cout << "\t Name        : " << this->getName() << std::endl;
    std::cout << "\t Type        : " << this->getType() << std::endl;
    std::cout << "\t Model       : " << "--"  << std::endl;
    std::cout << "\t # Nodes     : " << this->getNumNodes() << std::endl;
    std::cout << "\t # TotalDOF  : " << this->getNumDof() << std::endl;
    std::cout << "\t-------------------------------------\n";
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
