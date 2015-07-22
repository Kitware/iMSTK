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
#include "Core/Config.h"
#include "Core/ErrorLog.h"

// VEGA includes
#include "configFile.h"
#include "performanceCounter.h"

/// \brief This class parses and holds the information related to various input files
/// It is separated from the VegaFemSceneObject class in order to reduce the
/// amount of information stored while creating the fem model.
class VegaObjectConfig
{
public:

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

    enum timeIntegrationType
    {
        IMPLICITNEWMARK,
        IMPLICITBACKWARDEULER,
        EULER,
        SYMPLECTICEULER,
        CENTRALDIFFERENCES,
        UNKNOWN
    };

    // simulation. Some variable names self-explainatory
    int syncTimestepWithGraphics; ///< !!
    float timeStep; ///< time step
    float newmarkBeta; ///> beta in NewmarkBeta time integration
    float newmarkGamma; ///> gamma in NewmarkBeta time integration
    int use1DNewmarkParameterFamily; ///< !!
    int substepsPerTimeStep;
    double inversionThreshold; ///<
    int lockAt30Hz; ///< update at 30Hz
    int forceNeighborhoodSize; ///< !!
    int enableCompressionResistance;
    double compressionResistance;
    int centralDifferencesTangentialDampingUpdateMode; ///<
    int addGravity; ///< include gravity
    double g;///> gravity; unit: m/s^2
    int corotationalLinearFEM_warp;///> type of co-rotation formulation to be used
    const int max_corotationalLinearFEM_warp = 2;
    char implicitSolverMethod[4096];
    char solverMethod[4096];

    float dampingMassCoef; ///< viscous damping
    float dampingStiffnessCoef; ///< structural damping
    float dampingLaplacianCoef; ///<
    float deformableObjectCompliance;

    float baseFrequency; ///< !!
    int maxIterations; ///< maximum interations
    double epsilon;  ///<
    int numInternalForceThreads; ///< max. execution threads for computing internal force
    int numSolverThreads; ///< max. solver threads for solver
    int singleStepMode;

    // various file names. variable names self-explainatory
    //string renderingMeshFilename;
    char renderingMeshFilename[4096];
    char secondaryRenderingMeshFilename[4096];
    char secondaryRenderingMeshInterpolationFilename[4096];
    char volumetricMeshFilename[4096];
    char customMassSpringSystem[4096];
    char deformableObjectMethod[4096];
    char fixedVerticesFilename[4096];
    char massMatrixFilename[4096];
    char massSpringSystemObjConfigFilename[4096];
    char massSpringSystemTetMeshConfigFilename[4096];
    char massSpringSystemCubicMeshConfigFilename[4096];
    char invertibleMaterialString[4096];
    char initialPositionFilename[4096];
    char initialVelocityFilename[4096];
    char forceLoadsFilename[4096];
    char outputFilename[4096];

    massSpringSystemSourceType massSpringSystemSource;
    deformableObjectType deformableObject;
    invertibleMaterialType invertibleMaterial;
    timeIntegrationType solver;

    /// \brief Constructor
    VegaObjectConfig();

    /// \brief Destructor
    ~VegaObjectConfig();

    /// \brief Read the confiuration file to
    ///  parse all the specifications of the FEM scene
    ///  such as type of material type, input mesh and rendering files,
    ///  boundary conditions etc.
    void setFemObjConfuguration(const std::string ConfigFile, const bool printVerbose);

    /// enable/disable update of scene object sync with graphics
    void setSyncTimeStepWithGraphics(const bool syncOrNot);
};

class VegaPerformanceCounter
{
public:
    double fps; ///< fps of the simulation
    int fpsBufferSize;///< buffer size to display fps
    int fpsHead; ///< !!
    double fpsBuffer[5]; ///< buffer to display fps

    // force assembly metrics recording
    double forceAssemblyTime;
    double forceAssemblyLocalTime;
    int forceAssemblyBufferSize;
    int forceAssemblyHead;
    double forceAssemblyBuffer[50];

    // solver metrics recording
    double systemSolveTime;
    double systemSolveLocalTime;
    int systemSolveBufferSize;
    int systemSolveHead;
    double systemSolveBuffer[50];

    PerformanceCounter objectPerformanceCounter;///< keeps track of overall performance
    PerformanceCounter explosionCounter;///< keeps track of instability

    /// \brief constructor
    VegaPerformanceCounter();

    /// \brief destructor
    ~VegaPerformanceCounter();

    /// \brief Initialization
    void initialize();

    /// \brief clear the buffers that record the fps
    void clearFpsBuffer();
};

#endif
