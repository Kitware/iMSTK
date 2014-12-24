/*
****************************************************
                  SIMMEDTK LICENSE
****************************************************
*/

#ifndef SMTOOLSIMULATOR_H
#define SMTOOLSIMULATOR_H
#include "smCore/smConfig.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smErrorLog.h"
#include "smCore/smEventHandler.h"
#include "smCore/smEventData.h"
#include "smSimulators/smStylusObject.h"


/// \brief Example tool simulator
class smToolSimulator: public smObjectSimulator, public smEventHandler
{

public:
    /// \brief constructor
    smToolSimulator(smErrorLog *p_errorLog): smObjectSimulator(p_errorLog)
    {
    }

protected:
    /// \brief update everything related to tool
    void updateTool(smStylusRigidSceneObject *p_tool);

    /// \brief !!
    virtual void initCustom()
    {
    }

    /// \brief run the tool simulator in a loop here
    virtual void run()
    {

        smSceneObject *sceneObj;
        smStylusRigidSceneObject *tool;

        while (true && this->enabled)
        {
            beginSim();

            for (smInt i = 0; i < this->objectsSimulated.size(); i++)
            {
                sceneObj = this->objectsSimulated[i];

                //ensure that dummy simulator will work on static scene objects only.
                if (sceneObj->getType() == SIMMEDTK_SMSTYLUSRIGIDSCENEOBJECT)
                {
                    tool = (smStylusRigidSceneObject*)sceneObj;

                    if (tool->toolEnabled)
                    {
                        updateTool(tool);
                    }
                }
            }

            endSim();
        }
    }

    /// \brief synchronize the buffers in the object..do not call by yourself.
    void syncBuffers()
    {
    }

    /// \brief handle the events such as button presses related to tool
    void handleEvent(smEvent *p_event)
    {

        smKeyboardEventData *keyBoardData;

        switch (p_event->eventType.eventTypeCode)
        {
        case SIMMEDTK_EVENTTYPE_KEYBOARD:
            keyBoardData = (smKeyboardEventData*)p_event->data;

            if (keyBoardData->keyBoardKey == Qt::Key_F1)
            {
                printf("F1 Keyboard is pressed %c\n", keyBoardData->keyBoardKey);
            }

            break;
        }
    }
};

#endif
