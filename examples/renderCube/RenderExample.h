#ifndef RENDER_EXAMPLE_H
#define RENDER_EXAMPLE_H

#include "smCore/smSDK.h"

class RenderExample : public smSimulationMain, public smCoreClass
{
public:
    RenderExample();
    void simulateMain(smSimulationMainParam p_param) { };
private:
    smSDK* simmedtkSDK;
    smScene *scene1;
    smStaticSceneObject cube;
    smViewer *viewer;
};

void renderExample();

#endif
