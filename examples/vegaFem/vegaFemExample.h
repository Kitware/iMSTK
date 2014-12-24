/*****************************************************
                SimMedTK LICENSE
****************************************************

*****************************************************
*/

#include <GL/glew.h>
#include "smCore/smConfig.h"
#include "smCore/smErrorLog.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSDK.h"
#include "smRendering/smViewer.h"
#include "smSimulators/smVegaFemSimulator.h"
#include "smSimulators/smVegaFemSceneObject.h"

class vegaFemExample : public smSimulationMain, public smCoreClass
{

public:
    vegaFemExample();
    void simulateMain(smSimulationMainParam p_param) { };
private:
    smSDK *sofmisSDK;
    smVegaFemSceneObject *femobj;
    smVegaFemSimulator *femSim;
    smMatrix33<smFloat> mat;
    smSimulator *simulator;
    smViewer *viewer;
    smScene *scene1;
};

void VegaFemExample();
