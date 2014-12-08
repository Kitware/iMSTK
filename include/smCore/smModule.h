/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMMODULE_H
#define SMMODULE_H
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smDispatcher.h"
#include "smCore/smScene.h"
#include "smCore/smSceneObject.h"
#include "smCore/smEventHandler.h"


///this class is module major. Every other thread should derive this class
class smModule:public smCoreClass{

private:
	friend class smSDK;

protected:
	///initialization flag
	smBool isInitialized;

	///execution termination..if it is true exit from the thread
	smBool terminateExecution;

	///When the terminatation is done by the module, this will be true
	smBool terminationCompleted;

	///scene list in the environment
	vector<smScene*>sceneList;
	/// \brief call are made for begin module and end module before and after each frame
	virtual void beginModule();
	virtual void   endModule();
	/// \brief  dispatcher reference
	smDispatcher *dispathcer;
	/// \brief  event dispatcher reference
	smEventDispatcher *eventDispatcher;

public:
	/// \brief  constructor initializes the module
	smModule(){
		terminateExecution=false;
		eventDispatcher=NULL;
		isInitialized=false;
			name="Module";
	}

	void list(){

	}
	/// \brief virtual functions
	virtual void init()=0;
	virtual void beginFrame()=0;
	virtual void endFrame()=0;
	virtual void exec()=0;
	/// \brief flags for termination
	void terminate(){
		terminateExecution=true;
	}
	/// \brief  to check if the termination of the module is completed
	smBool isTerminationDone(){
		return terminationCompleted;
	}
	/// \brief  wait for termination
	void waitTermination(){
		while(1){
			if(terminationCompleted==true)
				break;
		}
	}
	/// \brief set the event dispatcher
	void setEventDispatcher(smEventDispatcher *p_dispathcer){
		eventDispatcher=p_dispathcer;
	}
	/// \brief  get module id
	inline smInt getModuleId(){
		return uniqueId.ID;
	}

};

#endif
