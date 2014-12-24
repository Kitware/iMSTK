/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMSIMULATOR_H 
#define SMSIMULATOR_H 
#include <QThread>
#include <QThreadPool>
#include "smCore/smModule.h" 
#include "smCore/smObjectSimulator.h"
#include <omp.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>
#include "smExternal/threadpool/boost/threadpool.hpp"

using namespace boost::threadpool;

struct smSimulationMainParam{
	vector<smScene*>sceneList;
};
/// \brief call back for simulator module. simulateMain is called in every simulation module frame.
class smSimulationMain{

public:
	virtual void simulateMain(smSimulationMainParam)=0;
};

class smSimulator:public smModule,QThread{
	friend class smSDK;

protected:
	vector<smObjectSimulator*> simulators;
	vector<smObjectSimulator*> collisionDetectors;

	//QThreadPool *threadPool;
	fifo_pool *simulatorThreadPool;//priority pool
	/// \brief asynchronous thread pool
	QThreadPool *asyncPool;
	/// \brief  maximum number of threads
	smInt maxThreadCount;
	/// \brief  error log
	smErrorLog *log;
	/// \brief  module keeps track of frame number
	smUInt frameCounter;

	///Simulation main registration
	smSimulationMain *main;
	/// \brief  for updating the main in real-time. The change has effect after a frame is completed
	smSimulationMain *changedMain;

	volatile smInt  changedMainTimeStamp;
	/// \brief time stampe when main callback is registered
	volatile smInt  mainTimeStamp;

public:
	///initializes all the simulators in the objects in the scene..
	void init(){

		if(isInitialized==true)
			return;

		simulatorThreadPool=new  fifo_pool(maxThreadCount);
		asyncPool=new QThreadPool(this);
		smObjectSimulator *objectSimulator;
		for(smInt i=0;i<this->simulators.size();i++){
			objectSimulator=simulators[i];
			objectSimulator->init();
		}
		isInitialized=true;
	}
	/// \brief constructor gets error log
	smSimulator(smErrorLog *p_log){

		type=SIMMEDTK_SMSIMULATOR;
		isInitialized=false;
		this->log=p_log;
		frameCounter=0;
		main=NULL;
		changedMain=NULL;
		changedMainTimeStamp=0;
		mainTimeStamp=0;
		maxThreadCount=SIMMEDTK_MAX(simulators.size(),collisionDetectors.size());
	}

	void setMaxThreadCount(smInt p_threadMaxCount){

		if(p_threadMaxCount<0)
			return;
		else
			maxThreadCount=p_threadMaxCount;
	}

	///Simualtor registers the simulator and schedules it.
	///the function is reentrant it is not thread safe.
	void registerObjectSimulator(smObjectSimulator *objectSimulator);

	void  registerCollisionDetection(smObjectSimulator *p_collisionDetection);

	///Registration of the Simulation main. It is called in each and every frame
	void registerSimulationMain(smSimulationMain*p_main);
	/// \brief launches the asynchronous threads
	void startAsychThreads();
	/// \brief the actual implementation of the simulator module resides in run function
	void run();
	/// \brief called at the beginning of  each and every frame
	virtual void beginFrame();
	/// \brief called at the end of each and every frame 
	virtual void endFrame();
	/// \brief this is called by SDK. it lanuches the simulator module
	virtual void exec(){

		if(isInitialized)
			start();
		else{
			init();
			start();
		}
	}
};

#endif
