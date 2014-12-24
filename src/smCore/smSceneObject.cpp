#include "smCore/smSceneObject.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smSDK.h"

///attach the simulator to the  object
void smSceneObject::attachObjectSimulator(smObjectSimulator *p_objectSim)
{

    p_objectSim->addObject(this);
}

///release the simulator from the object
void smSceneObject::releaseObjectSimulator()
{

    objectSim->removeObject(this);
    objectSim = NULL;
}

///get a reference to the simulator
smObjectSimulator* smSceneObject::getObjectSimulator()
{

    return objectSim;
}

///attach the custom renderer to the scene object
void smSceneObject::attachCustomRenderer(smCustomRenderer *p_customeRenderer)
{

    customRender = p_customeRenderer;
}

///release the custom renderer from the class
void smSceneObject::releaseCustomeRenderer()
{
    customRender = NULL;
}

smSceneObject::smSceneObject()
{

    type = SIMMEDTK_SMSCENEBOJECT;
    objectSim = NULL;
    customRender = NULL;
    smSDK::registerSceneObject(this);
    memBlock = new smMemoryBlock(NULL);
    flags.isViewerInit = false;
    flags.isSimulatorInit = false;
    name = QString("SceneObject") + QString().setNum(uniqueId.ID);
}
