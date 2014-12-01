#include "smCore/smDispatcher.h"


void smDispatcher::handleViewer(smCoreClass *p_caller, smCallerState p_callerState){
	
	switch(p_callerState){
	}
}

void smDispatcher::handleSimulator(smCoreClass *p_caller, smCallerState p_callerState){

}

void smDispatcher::handleCollisionDetection(smCoreClass *p_caller, smCallerState p_callerState){

}

void smDispatcher::handleAll(){

}

//// \brief this function takes care of handling of system level tasks before or after every frame
///  \param p_caller        p_caller has pointer to the object which the function is called
///  \param p_callerState   p_callerState is stores the state of the at the moment of the function called.
///                         such as whether it is the beginning of the simulator frameor end of the simulator frame for now etc..
smDispathcerResult smDispatcher::handle(smCoreClass *p_caller, smCallerState p_callerState){
	
	smClassType classType;
	
	//get the type of the class which is calling
	classType=p_caller->getType();
	
	switch(classType){
		//handle for viewer
		SOFMIS_SMVIEWER:
		handleViewer(p_caller,p_callerState);
		break;
		//handle for simulator
		SOFMIS_SMSIMULATOR:
		handleSimulator(p_caller,p_callerState);

		break;
		//handle for collision detection
		SOFMIS_SMCOLLISIONDETECTION:
		handleCollisionDetection(p_caller,p_callerState);
		
		break;
	}

	handleAll();
	return SOFMIS_DISPATCHER_OK;
}
