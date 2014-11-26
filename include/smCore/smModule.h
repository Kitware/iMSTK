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

	virtual void beginModule();
	virtual void   endModule();
	smDispatcher *dispathcer;
	smEventDispatcher *eventDispatcher;

public:

	smModule(){
		terminateExecution=false;
		eventDispatcher=NULL;
		isInitialized=false;
			name="Module";
	}

	void list(){

	}

	virtual void init()=0;
	virtual void beginFrame()=0;
	virtual void endFrame()=0;
	virtual void exec()=0;

	void terminate(){
		terminateExecution=true;
	}

	smBool isTerminationDone(){
		return terminationCompleted;
	}

	void waitTermination(){
		while(1){
			if(terminationCompleted==true)
				break;
		}
	}

	void setEventDispatcher(smEventDispatcher *p_dispathcer){
		eventDispatcher=p_dispathcer;
	}

	inline smInt getModuleId(){
		return uniqueId.ID;
	}

};

#endif
