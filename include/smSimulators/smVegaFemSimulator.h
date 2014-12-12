/***************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMVEGAFEMSIMULATOR_H
#define SMVEGAFEMSIMULATOR_H

#include "smCore/smConfig.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smErrorLog.h"
#include "smCore/smEventHandler.h"
#include "smCore/smEventData.h"
#include "smCore/smEventData.h"
#include "smSimulators/smVegaFemSceneObject.h"

/// \brief Interface class between Vega and SimMedTK core
class smVegaFemSimulator:public smObjectSimulator,public smEventHandler{

private:
	smVec3<smFloat> hapticPosition; ///< position of the haptic device end effector
	smBool hapticButtonPressed; ///< true of the button is pressed
	smEventDispatcher *eventDispatcher; ///< !!

public:
	/// \brief constructor
	smVegaFemSimulator(smErrorLog *p_errorLog):smObjectSimulator(p_errorLog){
		hapticButtonPressed=false;
	}

	/// \brief !!
	void setDispatcher(smEventDispatcher *p_eventDispatcher){
		eventDispatcher=p_eventDispatcher;	
	}


protected:

	/// \brief start the job
	virtual void beginSim(){		
	}

	/// \brief !!
	virtual void initCustom(){

		smSceneObject *object;
		//do nothing for now
		for(smInt i=0;i<objectsSimulated.size();i++){
			object=objectsSimulated[i];
			switch(type){ 
				case SOFMIS_SMVEGAFEMSCENEOBJECT:
				object->flags.isSimulatorInit=true;
				break;
			}
		}
	}


	/// \brief implement the deformation computation through fem here
	virtual void run(){

		smSceneObject *sceneObj;
		smVegaFemSceneObject *femSceneObject;

		beginSim();
		for(smInt i=0;i<this->objectsSimulated.size();i++){
			sceneObj=this->objectsSimulated[i];

			//ensure that dummy simulator will work on static scene objects only.
			if(sceneObj->getType()==SOFMIS_SMVEGAFEMSCENEOBJECT){

				femSceneObject=(smVegaFemSceneObject*)sceneObj;
				
				femSceneObject->advanceDynamics();

			}
		}
		endSim();

	}

	/// \breif end the job
	void endSim(){
	}

	/// \brief synchronize the buffers in the object..do not call by yourself.
	void syncBuffers(){
	}

	/// \brief handle the keyboard and haptic button press events
	void handleEvent(smEvent *p_event){
		
		smKeyboardEventData *keyBoardData;
		smHapticOutEventData *hapticEventData;

		switch(p_event->eventType.eventTypeCode){
			case SOFMIS_EVENTTYPE_KEYBOARD:
				keyBoardData=(smKeyboardEventData*)p_event->data;
				if(keyBoardData->keyBoardKey==Qt::Key_F1)
					printf("F1 Keyboard is pressed %c\n",keyBoardData->keyBoardKey);
			break;


			case SOFMIS_EVENTTYPE_HAPTICOUT:
				hapticEventData=(smHapticOutEventData *)p_event->data;
				if(hapticEventData->deviceId==1){
					hapticPosition.x=hapticEventData->position.x;
					hapticPosition.y=hapticEventData->position.y;
					hapticPosition.z=hapticEventData->position.z;
					hapticButtonPressed=hapticEventData->buttonState[0];
				}
			break;
		}
	}
};

#endif
