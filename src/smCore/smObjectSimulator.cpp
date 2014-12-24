#include "smCore/smObjectSimulator.h"
#include "smRendering/smViewer.h"
#include "smCore/smSDK.h"

void smObjectSimulator::initDraw(smDrawParam p_params)
{

    p_params.rendererObject->addText(name);
}

void smObjectSimulator::draw(smDrawParam p_params)
{

    QString fps(name + " FPS: %1");
    p_params.rendererObject->updateText(name, fps.arg((smDouble)this->FPS));
}

smObjectSimulator::smObjectSimulator(smErrorLog *p_log)
{

    this->log = p_log;
    smSDK::registerObjectSim(this);
    name = QString("objecSimulator") + QString().setNum(uniqueId.ID);

    type = SIMMEDTK_SMOBJECTSIMULATOR;
    isObjectSimInitialized = false;
    threadPriority = SIMMEDTK_THREAD_NORMALPRIORITY;

    objectsSimulated.clear();

    //this statement is very important do not delete this
    setAutoDelete(false);
    FPS = 0.0;
    frameCounter = 0;
    totalTime = 0.0;
    timer.start();
    enabled = false;
    executionTypeStatusChanged = false;
    execType = SIMMEDTK_SIMEXECUTION_SYNCMODE;
}
