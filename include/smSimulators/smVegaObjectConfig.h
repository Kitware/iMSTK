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

#ifndef SM_VEGA_OBJECT_CONFIG_H
#define SM_VEGA_OBJECT_CONFIG_H

// SimMedTK includes
#include "smCore/smConfig.h"

// VEGA includes
#include "configFile.h"
#include "performanceCounter.h"

#define VEGA_PERFORMANCE_REC_BUFFER_SIZE 50

enum massSpringSystemSourceType
{
    OBJ,
    TETMESH,
    CUBICMESH,
    CHAIN,
    NONE
};

enum deformableObjectType
{
    STVK,
    COROTLINFEM,
    LINFEM,
    MASSSPRING,
    INVERTIBLEFEM,
    UNSPECIFIED
};

enum invertibleMaterialType
{
    INV_STVK,
    INV_NEOHOOKEAN,
    INV_MOONEYRIVLIN,
    INV_NONE
};

enum solverType
{
    IMPLICITNEWMARK,
    IMPLICITBACKWARDEULER,
    EULER,
    SYMPLECTICEULER,
    CENTRALDIFFERENCES,
    UNKNOWN
};

/// \brief This class parses and holds the information related to various input files
/// It is separated from the smVegaFemSceneObject class in order to reduce the
/// amount of information stored while creating the fem model.
class smVegaObjectConfig
{
public:

    // simulation. Some variable names self-explainatory
    smInt syncTimestepWithGraphics; ///< !!
    smFloat timeStep; ///< time step
    smFloat newmarkBeta; ///> beta in NewmarkBeta time smIntegration
    smFloat newmarkGamma; ///> gamma in NewmarkBeta time smIntegration
    smInt use1DNewmarkParameterFamily; ///< !!
    smInt substepsPerTimeStep;
    smDouble inversionThreshold; ///<
    smInt lockAt30Hz; ///< update at 30Hz
    smInt forceNeighborhoodSize; ///< !!
    smInt enableCompressionResistance;
    smDouble compressionResistance;
    smInt centralDifferencesTangentialDampingUpdateMode; ///<
    smInt addGravity; ///< include gravity
    smDouble g;///> gravity; unit: m/s^2
    smInt corotationalLinearFEM_warp;///> type of co-rotation formulation to be used
    const smInt max_corotationalLinearFEM_warp = 2;
    smChar implicitSolverMethod[4096];
    smChar solverMethod[4096];

    smFloat dampingMassCoef; ///< viscous damping
    smFloat dampingStiffnessCoef; ///< structural damping
    smFloat dampingLaplacianCoef; ///<
    smFloat deformableObjectCompliance;

    smFloat baseFrequency; ///< !!
    smInt maxIterations; ///< maximum smInterations
    smDouble epsilon;  ///<
    smInt numInternalForceThreads; ///< max. execution threads for computing smInternal force
    smInt numSolverThreads; ///< max. solver threads for solver
    smInt singleStepMode;

    // various file names. variable names self-explainatory
    smChar renderingMeshFilename[4096];
    smChar secondaryRenderingMeshFilename[4096];
    smChar secondaryRenderingMeshInterpolationFilename[4096];
    smChar volumetricMeshFilename[4096];
    smChar customMassSpringSystem[4096];
    smChar deformableObjectMethod[4096];
    smChar fixedVerticesFilename[4096];
    smChar massMatrixFilename[4096];
    smChar massSpringSystemObjConfigFilename[4096];
    smChar massSpringSystemTetMeshConfigFilename[4096];
    smChar massSpringSystemCubicMeshConfigFilename[4096];
    smChar invertibleMaterialString[4096];
    smChar initialPositionFilename[4096];
    smChar initialVelocityFilename[4096];
    smChar forceLoadsFilename[4096];
    smChar outputFilename[4096];

    massSpringSystemSourceType massSpringSystemSource;
    deformableObjectType deformableObject;
    invertibleMaterialType invertibleMaterial;
    solverType solver;

    /// \brief Constructor
    smVegaObjectConfig();

    /// \brief Destructor
    ~smVegaObjectConfig();

    /// \brief Read the confiuration file to 
    ///  parse all the specifications of the FEM scene
    ///  such as type of material type, input mesh and rendering files,
    ///  boundary conditions etc.
    void setFemObjConfuguration(const std::string &ConfigFile);
};

class smVegaPerformanceCounter
{
public:
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
    PerformanceCounter titleBarCounter;
    PerformanceCounter explosionCounter;
    PerformanceCounter cpuLoadCounter;

    smVegaPerformanceCounter();
    ~smVegaPerformanceCounter();

    void initialize();

    void clearFpsBuffer();
};

#endif