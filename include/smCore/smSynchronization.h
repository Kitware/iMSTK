/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMSYNCHRONIZATION_H 
#define SMSYNCHRONIZATION_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include <QWaitCondition>
#include <QMutex>

///Synchronization class for sync the start/end of multiple threads
///simply set number of worker threads in the constructor
///then each worker threads should call waitTaskStart function when the taks
///is completed they should call signalTaskDone 
class smSynchronization:public smCoreClass{

	QWaitCondition taskDone;
	QWaitCondition taskStart;
	QMutex serverMutex;
	smInt totalWorkers;
	smInt finishedWorkerCounter;
	smInt startedWorkerCounter;
	smBool workerCounterUpdated;
	smInt newWorkerCounter;

public:

	/// \param p_threadsForWorkers	choose the number of worker threads
	smSynchronization(smInt p_threadsForWorkers){
		type=	SOFMIS_SMSYNCHRONIZATION;
		totalWorkers=p_threadsForWorkers;
		finishedWorkerCounter=0;
		startedWorkerCounter=0;
		workerCounterUpdated=false;
	}

	///before starting tasks worker threads should wait
	void waitTaskStart(){
		serverMutex.lock();
		startedWorkerCounter++;
		if(startedWorkerCounter==totalWorkers){
			startedWorkerCounter=0;
			taskDone.wakeAll();
		}

		taskStart.wait(&serverMutex);
		serverMutex.unlock();
	}

	///when the task ends the worker should call this function
	void signalTaskDone(){

		serverMutex.lock();
		finishedWorkerCounter++;

		if(finishedWorkerCounter==totalWorkers){
			finishedWorkerCounter=0;
		}
		serverMutex.unlock();
	}

	///You could change the number of worker threads synchronization 
	///Call this function after in the main thread where orchestration is done.
	void setWorkerCounter(smInt p_workerCounter){
		newWorkerCounter=p_workerCounter;
		workerCounterUpdated=true;
	}

	smInt getTotalWorkers(){
		return totalWorkers;
	}

	///the server thread should call this for to start exeuction of the worker threads
	void startTasks(){
		serverMutex.lock();
		if(workerCounterUpdated){
			finishedWorkerCounter=newWorkerCounter;
			workerCounterUpdated=false;
		}

		taskStart.wakeAll();
		Sleep(100);
		taskDone.wait(&serverMutex);
		serverMutex.unlock();
	}

	///this function is fore signalling the events after waking up the worker threads. 
	void startTasksandSignalEvent(smInt moduleId){

		smEvent *eventSynch;
		eventSynch=new smEvent();
		eventSynch->eventType=SOFMIS_EVENTTYPE_SYNCH;
		eventSynch->senderId=moduleId;
		eventSynch->senderType=SOFMIS_SENDERTYPE_EVENTSOURCE;
		serverMutex.lock();
		taskStart.wakeAll();
		taskDone.wait(&serverMutex);
		serverMutex.unlock();
	}
};

#endif
