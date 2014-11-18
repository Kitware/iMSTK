#include "smCore/smModule.h"
//QAtomicInt smModule::moduleIdCounter(1);

///Begin frame will be called before the cycle 
void  smModule::beginModule(){
    dispathcer->handle((smCoreClass*)this,SOFMIS_CALLERSTATE_BEGINFRAME);
	beginFrame();

	
}

///End frame will be called after the cycle 
void  smModule::endModule(){
    
	endFrame();
	dispathcer->handle((smCoreClass*)this,SOFMIS_CALLERSTATE_ENDFRAME);

}