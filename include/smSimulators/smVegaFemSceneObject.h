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

#ifndef SMVEGAFEMSCENEOBJECT_H
#define SMVEGAFEMSCENEOBJECT_H

// SimMedTK includes
#include "smCore/smConfig.h"
#include "smMesh/smVolumeMesh.h"
#include "smMesh/smSurfaceMesh.h"
#include "smMesh/smVegaSceneObjectDeformable.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"
#include "smCore/smErrorLog.h"
#include "smRendering/smConfigRendering.h"
#include "smRendering/smCustomRenderer.h"
#include "smSimulators/smVegaConfigFemObject.h"

// VEGA includes
#include "centralDifferencesSparse.h"
#include "configFile.h"
#include "corotationalLinearFEMForceModel.h"
#include "corotationalLinearFEM.h"
#include "corotationalLinearFEMMT.h"
#include "eulerSparse.h"
#include "generateMeshGraph.h"
#include "getIntegratorSolver.h"
#include "getopts.h"
#include "graph.h"
#include "implicitBackwardEulerSparse.h"
#include "isotropicHyperelasticFEMForceModel.h"
#include "isotropicHyperelasticFEM.h"
#include "isotropicHyperelasticFEMMT.h"
#include "isotropicMaterial.h"
#include "linearFEMForceModel.h"
#include "loadList.h"
#include "massSpringSystemForceModel.h"
#include "massSpringSystemFromCubicMeshConfigFile.h"
#include "massSpringSystemFromObjMeshConfigFile.h"
#include "massSpringSystemFromTetMeshConfigFile.h"
#include "massSpringSystem.h"
#include "massSpringSystemMT.h"
#include "matrixIO.h"
#include "MooneyRivlinIsotropicMaterial.h"
#include "neoHookeanIsotropicMaterial.h"
#include "objMesh.h"
#include "performanceCounter.h"
#include "renderSprings.h"
#include "StVKCubeABCD.h"
#include "StVKElementABCDLoader.h"
#include "StVKForceModel.h"
#include "StVKInternalForces.h"
#include "StVKInternalForcesMT.h"
#include "StVKIsotropicMaterial.h"
#include "StVKStiffnessMatrix.h"
#include "StVKStiffnessMatrixMT.h"
#include "StVKTetABCD.h"
#include "StVKTetHighMemoryABCD.h"
#include "tetMesh.h"
#include "volumetricMesh.h"
#include "volumetricMeshLoader.h"

#define VEGA_PERFORMANCE_REC_BUFFER_SIZE 50
const smString vega_string_none("__none");

/// \ Workhorse class for finite element object simulated using VEGA libraries.
///   Functionality: initialization, update of tangend stiffness, stepping in time
/// \ , solving system of equations, post processing and customized rendering
class smVegaFemSceneObject: public smSceneObject
{
public:

    /// \brief Constructor
	smVegaFemSceneObject();

    /// \brief Constructor
    smVegaFemSceneObject(std::shared_ptr<smErrorLog> p_log = nullptr, smString ConfigFile = vega_string_none);

    /// \brief Destructor
    ~smVegaFemSceneObject();

	bool configure(smString ConfigFile);

    /// \brief Initialize the parameters and properties of the simulation object
    void initSimulation();

    /// \brief Set the type of formulation used to model the deformation
    void setDeformableModel();

    /// \brief Load specified meshes
    void loadVolumeMesh();

    /// \brief Load the rendering mesh if it is designated
    void loadSurfaceMesh();

    /// \brief Load the data related to the vertices that will be fixed
    void loadFixedBC();

    /// \brief load initial displacements and velocities of the nodes
    void loadInitialStates();

    /// \brief load the scripted externalloads
    void loadScriptedExternalFroces();

    /// \brief Create the force model (underlying formulation)
    void createForceModel();

    /// \brief Inititialize the time integrator
    void initializeTimeIntegrator();

    /// \brief Update the deformations by time stepping
    void advanceDynamics();

    /// \brief Advance in time by a specificed amount and a chosen time stepping scheme
    inline void advanceOneTimeStep();

    /// \brief Forces as a result of user interaction
    /// (through an interface such as mouse or haptic device)
    /// with the scene during runtime are added here
    inline void applyUserInteractionForces();

    /// \brief Append the contact forces (if any)
    void applyContactForces();

    /// \brief Set all contact forces to zero (if any)
    void setContactForcesToZero();

    /// \brief Forces that are defined by the user before the start of the simulation
    ///  is added to the external force vector here
    inline void applyScriptedExternalForces();

    /// \brief Use the computed displacemetnt update to interpolate to the secondary display mesh
    inline void updateSecondaryRenderingMesh();

    /// \brief Updates the stats related to timing, fps etc. Also updates window title with real-time information
    inline void updateStats();

    /// \brief prints a given string on the screen
    void print_bitmap_string(float x, float y, float z, void * font, char * s);

    /// \brief not implemented yet.
    virtual std::shared_ptr<smSceneObject> clone() override
    {
        return safeDownCast<smSceneObject>();
    }

    void setRenderUsingVega(const bool vegaRender);

    /// \brief check all the surface nodes for the closest node within
    /// certain threshold and set it to be the pulled vertex
    void setPulledVertex(const smVec3d &userPos);

    /// \brief  Displays the fem object with primary or secondary mesh, fixed vertices,
    ///  vertices interacted with, ground plane etc.
    virtual void draw() override;

    void renderWithVega();

	///serialize function explicity writes the object to the memory block
	///each scene object should know how to write itself to a memory block
	virtual void serialize(void *p_memoryBlock) override {};

	///Unserialize function can recover the object from the memory location
	virtual void unSerialize(void *p_memoryBlock) override {};

	///this function may not be used
	///every Scene Object should know how to clone itself. Since the data structures will be
	///in the beginning of the modules(such as simulator, viewer, collision etc.)
	//virtual std::shared_ptr<smSceneObject> clone() override { return nullptr; };

	virtual void init() override {};

public:
    /// performance counters and simulation flags. some variable names are self explainatory
    double fps; ///< fps of the simulation
	int fpsBufferSize;///< buffer size to display fps
    int fpsHead; ///< !!
    double fpsBuffer[5]; ///< buffer to display fps
    double cpuLoad;
    double forceAssemblyTime;
    double forceAssemblyLocalTime;
	int forceAssemblyBufferSize;
    int forceAssemblyHead;
    double forceAssemblyBuffer[VEGA_PERFORMANCE_REC_BUFFER_SIZE];
    double systemSolveTime;
    double systemSolveLocalTime;
	int systemSolveBufferSize;
    int systemSolveHead;
    double systemSolveBuffer[VEGA_PERFORMANCE_REC_BUFFER_SIZE];
    int enableTextures;
    int staticSolver;
    int graphicFrame;
    int pulledVertex; ///< vertex that is pulled by user using external force
    int explosionFlag; ///< 1 if the simulation goes unstable
    PerformanceCounter titleBarCounter;
    PerformanceCounter explosionCounter;
    PerformanceCounter cpuLoadCounter;
    int timestepCounter;
    int subTimestepCounter;
    bool renderUsingVega;
    bool importAndUpdateVolumeMeshToSmtk;

    /// Force models, time integrators, sparse matrices and meshes.
    /// some variable names are self explainatory
    std::vector<int> fixedVertices; ///< fixed vertcies
    std::vector<double> forceLoads; ///< discrete external load inputs
    int positiveDefinite; ///< 1 if the effective matrix is positive definite
    std::shared_ptr<IntegratorBase> integratorBase; ///< integrator
    std::shared_ptr<ImplicitNewmarkSparse> implicitNewmarkSparse;
    std::shared_ptr<IntegratorBaseSparse> integratorBaseSparse;
    std::shared_ptr<ForceModel> forceModel; ///< Type of formulation driving the FEM simulation
    std::shared_ptr<StVKInternalForces> stVKInternalForces;
    std::shared_ptr<StVKStiffnessMatrix> stVKStiffnessMatrix;
    std::shared_ptr<StVKForceModel> stVKForceModel;
    std::shared_ptr<MassSpringSystemForceModel> massSpringSystemForceModel;
    std::shared_ptr<CorotationalLinearFEMForceModel> corotationalLinearFEMForceModel;
    std::shared_ptr<VolumetricMesh> volumetricMesh; ///< volume mesh
    std::shared_ptr<TetMesh> tetMesh; ///< volume mesh
    std::shared_ptr<Graph> meshGraph; ///< graph of the mesh
    std::shared_ptr<MassSpringSystem> massSpringSystem;
    std::shared_ptr<RenderSprings> renderMassSprings;
    std::shared_ptr<SparseMatrix> massMatrix; ///< sparse mass matrix need for FEM simulation
    std::shared_ptr<SparseMatrix> LaplacianDampingMatrix; ///< sparse damping matrix need for FEM simulation
    int n;

    // body states
    double * u;                 ///< displacement
    double * uvel;              ///< derivative of displacement in time
    double * uaccel;            ///< double derivative of displacement in time
    double * f_ext;             ///< external forces
    double * f_extBase;         ///< non-varying external forces
    double * uSecondary;        ///< interpolated displacement for secondary mesh
    double * uInitial;          ///< initial displacement
    double * velInitial;        ///< initial velocity

    std::vector<double> f_contact; ///< contact forces (if any)

    std::shared_ptr<smVegaConfigFemObject> femConfig;

    /// interpolation to secondary rendering mesh. self explainatory names
    int secondaryDeformableObjectRenderingMesh_interpolation_numElementVertices;
    int * secondaryDeformableObjectRenderingMesh_interpolation_vertices;
    double * secondaryDeformableObjectRenderingMesh_interpolation_weights;

    std::shared_ptr<smVegaSceneObjectDeformable> deformableObjectRenderingMesh;
    std::shared_ptr<smVegaSceneObjectDeformable> secondaryDeformableObjectRenderingMesh;

    std::shared_ptr<smVolumeMesh> smtkVolumeMesh;
    std::shared_ptr<smSurfaceMesh> smtkSurfaceMesh;
};

#endif
