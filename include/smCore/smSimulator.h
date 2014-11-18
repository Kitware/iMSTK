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
    \brief	    This Module is for Main Simulator Module which orchestrates the object simulators.
    \brief      Module is in test now

*****************************************************
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
		QThreadPool *asyncPool;
		smInt maxThreadCount;
		
		smErrorLog *log;
		smUInt frameCounter;

		///Simulation main registration
		smSimulationMain *main;
		smSimulationMain *changedMain;
		volatile smInt  changedMainTimeStamp;
		volatile smInt  mainTimeStamp;

		
	
	public:	
		///initializes all the simulators in the objects in the scene..
		void init(){
			if(isInitialized==true)
				return;
			//threadPool=new QThreadPool(this);
			simulatorThreadPool=new  fifo_pool(maxThreadCount);


			 asyncPool=new QThreadPool(this);
			smObjectSimulator *objectSimulator;
			for(smInt i=0;i<this->simulators.size();i++){
				objectSimulator=simulators[i];
				objectSimulator->init();

		
			}
			isInitialized=true;

		
		
		}


		smSimulator(smErrorLog *p_log){
			type=SOFMIS_SMSIMULATOR;
			//maxThreadCount=QThread::QThread::ide
			isInitialized=false;
			this->log=p_log;
			frameCounter=0;
			main=NULL;
			changedMain=NULL;
			changedMainTimeStamp=0;
			mainTimeStamp=0;
			maxThreadCount=SOFMIS_MAX(simulators.size(),collisionDetectors.size());


		
		}

		void setMaxThreadCount(smInt p_threadMaxCount){
			if(p_threadMaxCount<0)
				return;
			else
				maxThreadCount=p_threadMaxCount;

			//correct this statement.
			//threadPool->setMaxThreadCount(p_threadMaxCount); 

		
		}
		

		///Simualtor registers the simulator and schedules it.
		///the function is reentrant it is not thread safe.
		void registerObjectSimulator(smObjectSimulator *objectSimulator);
		/*{
			simulators.push_back(objectSimulator);
		
		}*/

		void  registerCollisionDetection(smObjectSimulator *p_collisionDetection);

		///Registration of the Simulation main
		void registerSimulationMain(smSimulationMain*p_main);

		void startAsychThreads();

		void run();
		 virtual void beginFrame();
		 virtual void endFrame();
		 virtual void exec(){
			 if(isInitialized)
				 start();
			 else{
				init();
				start();

			 }

		 }


		 
		
	

	private:




};




#endif
