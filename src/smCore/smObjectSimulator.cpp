#include "smCore/smObjectSimulator.h"
#include "smRendering/smViewer.h"
#include "smCore/smSDK.h"

//QAtomicInt smObjectSimulator::objectSimulatorIdCounter(1);

void smObjectSimulator::initDraw(smDrawParam p_params){
	p_params.rendererObject->addText(name);

}
void smObjectSimulator::draw(smDrawParam p_params){
	QString fps(name+" FPS: %1");
	p_params.rendererObject->updateText(name,fps.arg((smDouble)this->FPS));


}

smObjectSimulator::smObjectSimulator(smErrorLog *p_log){
			this->log=p_log;
			smSDK::registerObjectSim(this);
			name=QString("objecSimulator")+QString().setNum(uniqueId.ID);
		
			type=SOFMIS_SMOBJECTSIMULATOR;
		    isObjectSimInitialized=false;
			threadPriority=SOFMIS_THREAD_NORMALPRIORITY;
			
			//objectSimulatorId=objectSimulatorIdCounter.fetchAndAddOrdered(1);
			objectsSimulated.clear();
			//this statement is very important do not delete this
		    setAutoDelete(false);
			FPS=0.0;
			frameCounter=0;
			totalTime=0.0;
			timer.start();
			enabled=false;
			executionTypeStatusChanged=false;
			execType=SOFMIS_SIMEXECUTION_SYNCMODE;
}