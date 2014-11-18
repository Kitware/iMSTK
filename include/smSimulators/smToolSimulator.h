/*
****************************************************
                  SOFMIS LICENSE

****************************************************

    \author:    <http:\\acor.rpi.edu>
                SOFMIS TEAM IN ALPHABATIC ORDER
                Anderson Maciel, Ph.D.
                Ganesh Sankaranarayanan, Ph.D.
                Sreekanth A Venkata
                Suvranu De, Ph.D.
                Tansel Halic
                Zhonghua Lu

    \author:    Module by Tansel Halic
                
                
    \version    1.0
    \date       05/2009
    \bug	    None yet
    \brief	    This class is the simulator object. Each simulator should derive this.
                
    

*****************************************************
*/

#ifndef SMTOOLSIMULATOR_H
#define SMTOOLSIMULATOR_H
#include "smCore/smConfig.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smErrorLog.h"
#include "smCore/smEventHandler.h"
#include "smCore/smEventData.h"
#include "smSimulators/smStylusObject.h"


///example simulator..the dummy simulator works on static scene objects for now..
class smToolSimulator:public smObjectSimulator,public smEventHandler{

private:
	  

public:
	smToolSimulator(smErrorLog *p_errorLog):smObjectSimulator(p_errorLog){
		

	
	
	}



protected:
	void updateTool(smStylusRigidSceneObject *p_tool);
	


	virtual void initCustom(){
	
	}


	//test
	virtual void run(){
		smSceneObject *sceneObj;
		smStylusRigidSceneObject *tool;
		
		
		while(true &&this->enabled){
		beginSim();
		for(smInt i=0;i<this->objectsSimulated.size();i++){
			  sceneObj=this->objectsSimulated[i];

			  //ensure that dummy simulator will work on static scene objects only.
			  if(sceneObj->getType()==SOFMIS_SMSTYLUSRIGIDSCENEOBJECT){
				  tool=(smStylusRigidSceneObject*)sceneObj;
				  if(tool->toolEnabled)
						updateTool(tool);
	  
			  }

			
			
		  }
		  //wait
		  //do the job
		  //write the result
		//Sleep(20);
		 endSim();
		}
		
			

	
	
	}

	

	///synchronize the buffers in the object..do not call by yourself.
	void syncBuffers(){
	

	  
	}

	void handleEvent(smEvent *p_event){
		smKeyboardEventData *keyBoardData;
				
		switch(p_event->eventType.eventTypeCode){

		case SOFMIS_EVENTTYPE_KEYBOARD:
			keyBoardData=(smKeyboardEventData*)p_event->data;
			if(keyBoardData->keyBoardKey==Qt::Key_F1)
				printf("F1 Keyboard is pressed %c\n",keyBoardData->keyBoardKey);
			break;

		
		
		}
	
	}

};





#endif