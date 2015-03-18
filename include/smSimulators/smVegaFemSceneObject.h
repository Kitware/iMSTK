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
#include "smCore/smCoreClass.h"
#include "smCore/smSceneObject.h"
#include "smCore/smErrorLog.h"
#include "smRendering/smConfigRendering.h"
#include "smRendering/smCustomRenderer.h"
#include "smSimulators/smVegaConfigFemObject.h"

// VEGA includes
#include "getopts.h"
#include "sceneObjectDeformable.h"
#include "performanceCounter.h"
#include "tetMesh.h"
#include "StVKCubeABCD.h"
#include "StVKTetABCD.h"
#include "StVKTetHighMemoryABCD.h"
#include "implicitBackwardEulerSparse.h"
#include "eulerSparse.h"
#include "centralDifferencesSparse.h"
#include "StVKInternalForces.h"
#include "StVKStiffnessMatrix.h"
#include "StVKInternalForcesMT.h"
#include "StVKStiffnessMatrixMT.h"
#include "StVKForceModel.h"
#include "massSpringSystemForceModel.h"
#include "corotationalLinearFEM.h"
#include "corotationalLinearFEMMT.h"
#include "corotationalLinearFEMForceModel.h"
#include "linearFEMForceModel.h"
#include "isotropicHyperelasticFEM.h"
#include "isotropicHyperelasticFEMMT.h"
#include "isotropicHyperelasticFEMForceModel.h"
#include "isotropicMaterial.h"
#include "StVKIsotropicMaterial.h"
#include "neoHookeanIsotropicMaterial.h"
#include "MooneyRivlinIsotropicMaterial.h"
#include "getIntegratorSolver.h"
#include "volumetricMeshLoader.h"
#include "StVKElementABCDLoader.h"
#include "generateMeshGraph.h"
#include "massSpringSystem.h"
#include "massSpringSystemMT.h"
#include "massSpringSystemFromObjMeshConfigFile.h"
#include "massSpringSystemFromTetMeshConfigFile.h"
#include "massSpringSystemFromCubicMeshConfigFile.h"
#include "graph.h"
#include "renderSprings.h"
#include "configFile.h"
#include "loadList.h"
#include "matrixIO.h"

#define VEGA_PERFORMANCE_REC_BUFFER_SIZE 50
const smString vega_string_none("__none");

/// \ Workhorse class for finite element object simulated using VEGA libraries.
///   Functionality: initialization, update of tangend stiffness, stepping in time
/// \ , solving system of equations, post processing and customized rendering
class smVegaFemSceneObject: public smSceneObject
{

public:

    /// performance counters and simulation flags. some variable names are self explainatory
    double fps; ///< fps of the simulation
    const int fpsBufferSize = 5; ///< buffer size to display fps
    int fpsHead; ///< !!
    double fpsBuffer[5]; ///< buffer to display fps
    double cpuLoad;
    double forceAssemblyTime;
    double forceAssemblyLocalTime;
    const int forceAssemblyBufferSize = VEGA_PERFORMANCE_REC_BUFFER_SIZE;
    int forceAssemblyHead; /// !!
    double forceAssemblyBuffer[VEGA_PERFORMANCE_REC_BUFFER_SIZE];
    double systemSolveTime;
    double systemSolveLocalTime;
    const int systemSolveBufferSize = VEGA_PERFORMANCE_REC_BUFFER_SIZE;
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

    /// Force models, time integrators, sparse matrices and meshes.
    /// some variable names are self explainatory
    int numFixedVertices; ///< number of fixed vertices
    int * fixedVertices; ///< fixed vertcies
    int numForceLoads; ///< number of discrete external load inputs
    double * forceLoads; ///< discrete external load inputs
    int positiveDefinite; ///< 1 if the effective matrix is positive definite
    IntegratorBase * integratorBase; ///< integrator
    ImplicitNewmarkSparse * implicitNewmarkSparse;
    IntegratorBaseSparse * integratorBaseSparse;
    ForceModel * forceModel; ///< Type of formulation driving the FEM simulation
    StVKInternalForces * stVKInternalForces;
    StVKStiffnessMatrix * stVKStiffnessMatrix;
    StVKForceModel * stVKForceModel;
    MassSpringSystemForceModel * massSpringSystemForceModel;
    CorotationalLinearFEMForceModel * corotationalLinearFEMForceModel;
    VolumetricMesh * volumetricMesh; ///< volume mesh
    TetMesh * tetMesh; ///< volume mesh
    Graph * meshGraph; ///< graph of the mesh
    MassSpringSystem * massSpringSystem;
    RenderSprings * renderMassSprings;
    SparseMatrix * massMatrix; ///< sparse mass matrix need for FEM simulation
    SparseMatrix * LaplacianDampingMatrix; ///< sparse damping matrix need for FEM simulation
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

    smVegaConfigFemObject *femConfig;

    /// interpolation to secondary rendering mesh. self explainatory names
    int secondaryDeformableObjectRenderingMesh_interpolation_numElementVertices;
    int * secondaryDeformableObjectRenderingMesh_interpolation_vertices;
    double * secondaryDeformableObjectRenderingMesh_interpolation_weights;

    SceneObjectDeformable * deformableObjectRenderingMesh;
    SceneObjectDeformable * secondaryDeformableObjectRenderingMesh;

    /// \brief Constructor
    smVegaFemSceneObject(smErrorLog *p_log = NULL, smString ConfigFile = vega_string_none);

    /// \brief Destructor
    ~smVegaFemSceneObject();

    /// \brief Initialize the parameters and properties of the simulation object
    void initSimulation();

    /// \brief Set the type of formulation used to model the deformation
    void setDeformableModel();

    /// \brief Load specified meshes
    void loadMeshes();

    /// \brief Load the rendering mesh if it is designated
    void loadRenderingMesh();

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

    /// \brief Forces that are defined by the user before the start of the simulation
    ///  is added to the external force vector here
    inline void applyScriptedExternalForces();

    /// \brief Use the computed displacemetnt update to interpolate to the secondary display mesh
    inline void updateSecondaryRenderingMesh();

    /// \brief Updates the stats related to timing, fps etc. Also updates window title with real-time information
    inline void updateStats();

    /// \brief prints a given string on the screen
    void print_bitmap_string(float x, float y, float z, void * font, char * s);

    /// \brief draws cartesian axis
    void drawAxes(double axisLength);

    /// \brief
    virtual void serialize(void */*p_memoryBlock*/)
    {
        //add code in future
    }

    /// \brief
    virtual void unSerialize(void */*p_memoryBlock*/)
    {

    }

    /// \brief not implemented yet.
    virtual smSceneObject* clone()
    {
        return this;
    }

    /// \brief  Displays the fem object with primary or secondary mesh, fixed vertices,
    ///  vertices interacted with, ground plane etc.
    virtual void draw(const smDrawParam &p_params);

    virtual void init(){}
};

#endif
