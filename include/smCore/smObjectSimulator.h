/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMOBJECTSIMULATOR_H
#define SMOBJECTSIMULATOR_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smMemoryBlock.h"
#include <QVector>
#include <QThread>
#include <QRunnable>
#include "smCore/smSceneObject.h"
#include "smUtilities/smTimer.h"
#include "smCore/smScheduler.h"

enum smThreadPriority{
	SOFMIS_THREAD_IDLE=QThread::IdlePriority,
	SOFMIS_THREAD_LOWPRIORITY=QThread::LowPriority,
	SOFMIS_THREAD_NORMALPRIORITY=QThread::NormalPriority,
	SOFMIS_THREAD_HIGHESTPRIORITY=QThread::HighestPriority,
	SOFMIS_THREAD_TIMECRITICAL=QThread::TimeCriticalPriority,
};

enum smSimulatorExecutionType{
	SOFMIS_SIMEXECUTION_SYNCMODE,
	SOFMIS_SIMEXECUTION_ASYNCMODE
};

//forward declarations
class smSceneObject;

struct smObjectSimulatorParam{
	smShort threadIndex;
};

///This is the major object simulator. Each object simulator should derive this class.
///you want particular object simualtor to work over an object just set pointer of the object. the rest will be taken care of the simulator and object simulator.
class smObjectSimulator:public smCoreClass,QRunnable{

	///friend class since smSimulator is the encapsulates the other simulators.
	friend class smSimulator;

protected:	
	 ///log of the object
	smErrorLog *log;
	smBool isObjectSimInitialized;
	smThreadPriority threadPriority;
	smTimer timer;
	smLongDouble timerPerFrame;
	smLongDouble FPS;
	smUInt frameCounter;
	smLongDouble totalTime;
	smSimulatorExecutionType execType;
	smBool executionTypeStatusChanged;

public:
	///This is for scheduler 
	smScheduleGroup scheduleGroup;

	//smUnifiedID objectSimulatorId;
	smBool enabled;

	///the function is reentrant it is not thread safe.
	virtual void addObject(smSceneObject *p_object){
		p_object->objectSim=this;
		objectsSimulated.push_back(p_object);
	}

	virtual void removeObject(smSceneObject *p_object){
	}

	smObjectSimulator(smErrorLog *p_log);

	void setPriority(smThreadPriority p_priority){
		threadPriority=p_priority;
	};

	void setExecutionType(smSimulatorExecutionType p_type){
		if(execType!=p_type)
			executionTypeStatusChanged=true;
		execType=p_type;
	}

	smThreadPriority getPriority(){
		return threadPriority;
	}

protected:
	///objects that are simulated by this will be added to the list
	vector <smSceneObject*> objectsSimulated;

	virtual void initCustom()=0;

	void init(){
		if(isObjectSimInitialized==false){
			initCustom();
			//make the simulator true..it is initialized
			isObjectSimInitialized=true;
		}
	};

	virtual void run()=0; 
	virtual void beginSim(){
		frameCounter++;
		timer.start();
	};

	virtual void syncBuffers()=0;

	virtual void endSim(){
		timerPerFrame=timer.now(SOFMIS_TIMER_INMILLISECONDS);
		totalTime+=timerPerFrame;

		if(SMTIMER_FRAME_MILLISEC2SECONDS(totalTime)>1.0){
			FPS=frameCounter;
			frameCounter=0.0;
			totalTime=0.0;
		}
	}

	virtual void updateSceneList(){
	};

	virtual void initDraw(smDrawParam p_params);
	virtual void draw(smDrawParam p_params);

	struct smObjectSimulatorObjectIter{

	private:
		smShort beginIndex;
		smShort endIndex;
		smShort currentIndex;
		smShort	threadIndex;
	public:
		smObjectSimulatorObjectIter(smScheduleGroup &p_group, vector <smSceneObject*> &p_objectsSimulated,smInt p_threadIndex){

			smInt objectsPerThread;
			smInt leap;
			threadIndex=p_threadIndex;
			smInt totalObjects=p_objectsSimulated.size();
			leap=(totalObjects%p_group.totalThreads);
			objectsPerThread=p_objectsSimulated.size()/(p_group.totalThreads);

			if(threadIndex==0){
				beginIndex=0;
				endIndex=objectsPerThread+(leap!=0?1:0);

			}
			else{
				beginIndex=objectsPerThread*threadIndex;

				if(threadIndex<leap&&leap!=0)
					beginIndex+=threadIndex;
				else
					beginIndex+=leap;

				endIndex=beginIndex+objectsPerThread;
				if(endIndex<leap&&leap!=0) 
					endIndex++;
			}
		}

		inline void setThreadIndex(smShort p_threadIndex){
			threadIndex=p_threadIndex;
		}

		inline smInt begin(){
			return beginIndex;
		}

		inline smInt end(){
			return endIndex;
		}
	};
};

#endif
