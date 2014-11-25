#include "smCore/smModule.h"

/// \brief Begin frame will be called before the cycle 
void  smModule::beginModule(){
    
	dispathcer->handle((smCoreClass*)this,SOFMIS_CALLERSTATE_BEGINFRAME);
	beginFrame();

}

/// \brief End frame will be called after the cycle 
void  smModule::endModule(){

	endFrame();
	dispathcer->handle((smCoreClass*)this,SOFMIS_CALLERSTATE_ENDFRAME);
}
