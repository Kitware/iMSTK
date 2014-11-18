#include "smCore/smSimulator.h"
#include <omp.h>


///starts the tasks with the threads from thread pool
void smSimulator::beginFrame(){
	//omp_set_num_threads(this->simulators.size());
	
	//smObjectSimulator *objectSimulator;
	frameCounter++;
	
	
	//cout<<"Threads:"<<omp_get_num_threads() <<endl;

}

///waits until the frame ends
void smSimulator::endFrame(){
	//smObjectSimulator *objectSimulator;
	//threadPool->waitForDone();
	/*for(smInt i=0;i<this->simulators.size();i++){
		objectSimulator=simulators.at(i);
		objectSimulator->syncBuffers();

	
	}*/

}

void smSimulator::startAsychThreads(){
	smInt asyncThreadNbr=0;
	for(smInt i=0;i<simulators.size();i++){
		if(simulators[i]->execType==SOFMIS_SIMEXECUTION_ASYNCMODE)
			asyncThreadNbr++;
	}

	asyncPool->setMaxThreadCount(asyncThreadNbr);
	for(smInt i=0;i<simulators.size();i++){
		if(simulators[i]->execType==SOFMIS_SIMEXECUTION_ASYNCMODE)
			asyncPool->start(simulators[i],simulators[i]->getPriority());
	}


}

///the main simulation loop
void smSimulator::run(){
	smObjectSimulator *objectSimulator;
	smInt nbrSims;
	if(isInitialized==false){
		log->addError(this,"Simulator is not initialized.");
		return;
	}
	smSimulationMainParam param;
	param.sceneList=sceneList;
	
	//SetThreadAffinityMask(QThread::currentThreadId(),2);
	startAsychThreads();
	simulatorThreadPool->size_controller().resize(this->simulators.size());
	while(true&&this->terminateExecution==false){
		beginModule();
		if(main!=NULL)
			main->simulateMain(param);
		if(changedMainTimeStamp>mainTimeStamp){
			
			main=changedMain;
			changedMainTimeStamp=mainTimeStamp;
			
		}


		nbrSims=simulators.size();
		/*#pragma omp  parallel for 
		for(smInt i=0;i<nbrSims;i++){
			objectSimulator=simulators[i];
			objectSimulator->run();
			objectSimulator->syncBuffers();

	
		}
		#pragma omp barrier
		//#pragma omp  for 
		for(smInt i=0;i<this->collisionDetectors.size();i++){
			objectSimulator=collisionDetectors[i];
			objectSimulator->run();
			objectSimulator->syncBuffers();

		 }
		//#pragma omp barrier

		
		*/
		smTimer timer;
		
		//threadPool->setMaxThreadCount(this->simulators.size());
		//cout<<"Threads:"<<threadPool->activeThreadCount() <<endl;
		
		for(smInt i=0;i<this->simulators.size();i++){
				objectSimulator=simulators[i];
				if(objectSimulator->execType==SOFMIS_SIMEXECUTION_ASYNCMODE)
					continue;
				if(objectSimulator->enabled==false)
					continue;
				//threadPool->start(objectSimulator,objectSimulator->getPriority());   //old based on QT
				schedule(*simulatorThreadPool,boost::bind(&smObjectSimulator::run,objectSimulator));
				

		
		}
		simulatorThreadPool->wait();
		//threadPool->waitForDone();//based on QT thread
		
		for(smInt i=0;i<this->simulators.size();i++){
			objectSimulator=simulators[i];
			objectSimulator->syncBuffers();
		}
		
		timer.start();
		for(smInt i=0;i<this->collisionDetectors.size();i++){
			objectSimulator=collisionDetectors[i];
			//threadPool->start(objectSimulator,objectSimulator->getPriority());//old based on QT
			schedule(*simulatorThreadPool,boost::bind(&smObjectSimulator::run,objectSimulator));
			

		}
		simulatorThreadPool->wait();
		//threadPool->waitForDone();based on QT threds
		//cout<<"Coll:"<<timer.now(SOFMIS_TIMER_INMILLISECONDS)<<endl;
		//objectSimulator->syncBuffers();
		
		
		

		endModule();
	}
	asyncPool->waitForDone();

}




void smSimulator::registerObjectSimulator(smObjectSimulator *objectSimulator){
			simulators.push_back(objectSimulator);
			objectSimulator->enabled=true;
			
		
}
void  smSimulator::registerCollisionDetection(smObjectSimulator *p_collisionDetection)
{
	collisionDetectors.push_back(p_collisionDetection);	
}


void  smSimulator::registerSimulationMain(smSimulationMain*p_main)
{
	changedMain=p_main;
	this->changedMainTimeStamp++;
}




























