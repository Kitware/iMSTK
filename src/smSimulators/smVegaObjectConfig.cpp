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
#include "smSimulators/smVegaObjectConfig.h"

smVegaObjectConfig::smVegaObjectConfig()
{
    syncTimestepWithGraphics = 1;
    timeStep = 1.0 / 30;
    newmarkBeta = 0.25;
    newmarkGamma = 0.5;
    use1DNewmarkParameterFamily = 1;
    substepsPerTimeStep = 1;
    lockAt30Hz = 0;
    forceNeighborhoodSize = 5;
    enableCompressionResistance = 1;
    compressionResistance = 500;
    centralDifferencesTangentialDampingUpdateMode = 1;
    addGravity = 1;
    g = -9.81;
    dampingLaplacianCoef = 0.0;
    deformableObjectCompliance = 1.0;
    baseFrequency = 1.0;
    corotationalLinearFEM_warp = 1;
    singleStepMode = 0;

    massSpringSystemSource = NONE;
    deformableObject = UNSPECIFIED;
    invertibleMaterial = INV_NONE;
    solver = UNKNOWN;
}

smVegaObjectConfig::~smVegaObjectConfig()
{
}

void smVegaObjectConfig::setFemObjConfuguration(const std::string &ConfigFilename)
{

    printf("VEGA: Parsing configuration file %s...\n", ConfigFilename.c_str());
    ConfigFile configFile;

    // specify the entries of the config file
    // at least one of the following must be present:
    configFile.addOptionOptional("volumetricMeshFilename", volumetricMeshFilename, "__none");
    configFile.addOptionOptional("customMassSpringSystem", customMassSpringSystem, "__none");
    configFile.addOptionOptional("deformableObjectMethod", deformableObjectMethod, "StVK");
    
    configFile.addOptionOptional("massSpringSystemObjConfigFilename",
                                  massSpringSystemObjConfigFilename, "__none");

    configFile.addOptionOptional("massSpringSystemTetMeshConfigFilename",
                                  massSpringSystemTetMeshConfigFilename, "__none");

    configFile.addOptionOptional("massSpringSystemCubicMeshConfigFilename",
                                  massSpringSystemCubicMeshConfigFilename, "__none");

    // option for corotational linear FEM: if warp is disabled, one gets purely linear FEM
    configFile.addOptionOptional("corotationalLinearFEM_warp",
                                &corotationalLinearFEM_warp, corotationalLinearFEM_warp);

    // this is now obsolete, but preserved for backward compatibility, use "solver" below
    configFile.addOptionOptional("implicitSolverMethod", implicitSolverMethod, "none");
    configFile.addOptionOptional("solver", solverMethod, "implicitNewmark");
    
    configFile.addOptionOptional("centralDifferencesTangentialDampingUpdateMode", 
                                &centralDifferencesTangentialDampingUpdateMode,
                                centralDifferencesTangentialDampingUpdateMode);

    configFile.addOptionOptional("initialPositionFilename", initialPositionFilename, "__none");
    configFile.addOptionOptional("initialVelocityFilename", initialVelocityFilename, "__none");
    configFile.addOptionOptional("outputFilename", outputFilename, "__none");
    configFile.addOptionOptional("addGravity", &addGravity, addGravity);
    configFile.addOptionOptional("g", &g, g);
    configFile.addOptionOptional("renderingMeshFilename", renderingMeshFilename, "__none");
    
    configFile.addOptionOptional("secondaryRenderingMeshFilename",
                                  secondaryRenderingMeshFilename, "__none");
    
    configFile.addOptionOptional("secondaryRenderingMeshInterpolationFilename",
                                  secondaryRenderingMeshInterpolationFilename, "__none");

    //configFile.addOptionOptional("useRealTimeNormals", &useRealTimeNormals, 0);
    configFile.addOptionOptional("fixedVerticesFilename", fixedVerticesFilename, "__none");
    configFile.addOptionOptional("massMatrixFilename", massMatrixFilename, "__none");
    configFile.addOptionOptional("enableCompressionResistance",
                                  &enableCompressionResistance, enableCompressionResistance);

    configFile.addOptionOptional("compressionResistance",
                                 &compressionResistance, compressionResistance);

    configFile.addOption("timestep", &timeStep);

    configFile.addOptionOptional("substepsPerTimeStep",
                                 &substepsPerTimeStep, substepsPerTimeStep);
    
    configFile.addOptionOptional("syncTimestepWithGraphics",
                                  &syncTimestepWithGraphics, syncTimestepWithGraphics);

    configFile.addOption("dampingMassCoef", &dampingMassCoef);
    configFile.addOption("dampingStiffnessCoef", &dampingStiffnessCoef);
    
    configFile.addOptionOptional("dampingLaplacianCoef",
                                 &dampingLaplacianCoef, dampingLaplacianCoef);
    
    configFile.addOptionOptional("newmarkBeta", &newmarkBeta, newmarkBeta);
    configFile.addOptionOptional("newmarkGamma", &newmarkGamma, newmarkGamma);
    configFile.addOption("deformableObjectCompliance", &deformableObjectCompliance);
    configFile.addOption("baseFrequency", &baseFrequency);
    
    configFile.addOptionOptional("forceNeighborhoodSize",
                                 &forceNeighborhoodSize, forceNeighborhoodSize);
    
    configFile.addOptionOptional("maxIterations", &maxIterations, 1);
    configFile.addOptionOptional("epsilon", &epsilon, 1E-6);
    configFile.addOptionOptional("numInternalForceThreads", &numInternalForceThreads, 0);
    configFile.addOptionOptional("numSolverThreads", &numSolverThreads, 1);
    
    configFile.addOptionOptional("inversionThreshold",
                                 &inversionThreshold, -std::numeric_limits< double >::max());

    configFile.addOptionOptional("forceLoadsFilename", forceLoadsFilename, "__none");
    configFile.addOptionOptional("singleStepMode", &singleStepMode, singleStepMode);
    //configFile.addOptionOptional("pauseSimulation", &pauseSimulation, pauseSimulation);
    configFile.addOptionOptional("lockAt30Hz", &lockAt30Hz, lockAt30Hz);
    
    configFile.addOptionOptional("invertibleMaterial",
                                  invertibleMaterialString, invertibleMaterialString);

    // parse the configuration file
    if (configFile.parseOptions((char *)ConfigFilename.c_str()) != 0)
    {
        printf("VEGA: Error parsing options.\n");
        exit(1);
    }

    // the config variables have now been loaded with their specified values
    // informatively print the variables (with assigned values) that were just parsed
    configFile.printOptions();

    // set the solver based on config file input
    solver = UNKNOWN;

    if (strcmp(implicitSolverMethod, "implicitNewmark") == 0)
    {
        solver = IMPLICITNEWMARK;
    }

    if (strcmp(implicitSolverMethod, "implicitBackwardEuler") == 0)
    {
        solver = IMPLICITBACKWARDEULER;
    }

    if (strcmp(solverMethod, "implicitNewmark") == 0)
    {
        solver = IMPLICITNEWMARK;
    }

    if (strcmp(solverMethod, "implicitBackwardEuler") == 0)
    {
        solver = IMPLICITBACKWARDEULER;
    }

    if (strcmp(solverMethod, "Euler") == 0)
    {
        solver = EULER;
    }

    if (strcmp(solverMethod, "symplecticEuler") == 0)
    {
        solver = SYMPLECTICEULER;
    }

    if (strcmp(solverMethod, "centralDifferences") == 0)
    {
        solver = CENTRALDIFFERENCES;
    }

    if (solver == UNKNOWN)
    {
        printf("VEGA Error: unknown implicit solver specified.\n");
        exit(1);
    }
}

smVegaPerformanceCounter::smVegaPerformanceCounter()
{
}

smVegaPerformanceCounter::~smVegaPerformanceCounter()
{
}

void smVegaPerformanceCounter::initialize()
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

    fpsBufferSize = 5; ///< buffer size to display fps
    forceAssemblyBufferSize = VEGA_PERFORMANCE_REC_BUFFER_SIZE;
    systemSolveBufferSize = VEGA_PERFORMANCE_REC_BUFFER_SIZE;
}

void smVegaPerformanceCounter::clearFpsBuffer()
{
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