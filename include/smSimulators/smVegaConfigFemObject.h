/***************************************************
                SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMVEGACONFIGFEMOBJECT_H
#define SMVEGACONFIGFEMOBJECT_H

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cassert>
#include <float.h>
using namespace std;

#ifdef WIN32
#include <windows.h>
#endif

#include "configFile/configFile.h"

enum massSpringSystemSourceType { OBJ, TETMESH, CUBICMESH, CHAIN, NONE };
enum deformableObjectType { STVK, COROTLINFEM, LINFEM, MASSSPRING, INVERTIBLEFEM, UNSPECIFIED };
enum invertibleMaterialType { INV_STVK, INV_NEOHOOKEAN, INV_MOONEYRIVLIN, INV_NONE };
enum solverType { IMPLICITNEWMARK, IMPLICITBACKWARDEULER, EULER, SYMPLECTICEULER, CENTRALDIFFERENCES, UNKNOWN };


/// \brief This class parses and holds the information related to various input files
/// It is separated from the smVegaFemSceneObject class in order to reduce the
/// amount of information stored while creating the fem model.
class smVegaConfigFemObject
{

public:

    // display related
    int renderWireframe; ///< render wireframe (true-1, false-0)
    int renderAxes; ///< render axis (true-1, false-0)
    int renderDeformableObject; ///< render deformable object (true-1, false-0)
    int renderSecondaryDeformableObject; ///< render wireframe (true-1, false-0)
    int useRealTimeNormals; ///< update normals realtime (true-1, false-0)
    int renderFixedVertices; ///< render fixed vertices (true-1, false-0)
    int renderSprings; ///< render springs (true-1, false-0)
    int renderVertices; ///< render vertices (true-1, false-0)
    int displayWindowTitle; ///< display title of window (true-1, false-0)

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
    char lightingConfigFilename[4096];
    float dampingMassCoef; ///< viscous damping
    float dampingStiffnessCoef; ///< structural damping
    float dampingLaplacianCoef; ///<
    float deformableObjectCompliance;
    float baseFrequency; ///< !!
    int maxIterations; ///< maximum interations
    double epsilon;  ///<
    int numInternalForceThreads; ///< max. execution threads for computing internal force
    int numSolverThreads; ///< max. solver threads for solver
    int pauseSimulation;
    int singleStepMode;
    int lockScene;

    // various file names. variable names self-explainatory
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
    solverType solver;

    /// \brief Constructor
    smVegaConfigFemObject();

    /// \brief Destructor
    ~smVegaConfigFemObject();

    /// \brief Read the confiuration file to parse all the specifications of the FEM scene
    ///  such as type of material type, input mesh and rendering files,
    ///  boundary conditions etc.
    void setFemObjConfuguration(string ConfigFile);
};

#endif
