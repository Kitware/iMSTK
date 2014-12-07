#ifndef SMEFFECTSIM_H
#define SMEFFECTSIM_H

#include "smCore/smConfig.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smErrorLog.h"
#include "smCore/smEventHandler.h"
#include "smCore/smEventData.h"
#include "smRenderEffects/SmokeEffect.h"
/// \brief  effect simulation  such as smoke, spark etc.
class smEffectSim:public smStaticSceneObject, public smEventHandler{

public:
	SmokeVideo *smoke;
	bool enabled;
	smBool buttonState;
	double left[16];
	double normalizedToolTipX,normalizedToolTipY;

	/// \brief add spark effect for a mesh ..
	void addSparkMesh(smSurfaceMesh *p_mesh){
		smoke->spark->addSparkMesh(p_mesh);
	}
	/// \brief constructor for effect simulation
	smEffectSim(smErrorLog *p_errorLog):smStaticSceneObject(p_errorLog){
		buttonState=false;
		normalizedToolTipX=0;
		normalizedToolTipY=0;
		smoke=new SmokeVideo();
	}

protected:
	/// \brief initialization routines. 
	void initDraw(smDrawParam p_params){

	smoke->Initialize();
	}

	/// \brief effect called by viewer rendering 
	virtual void draw(smDrawParam p_params);

	/// \brief custome initialization  routines
	virtual void initCustom(){
	}

	//// \brief legacy code
	virtual void run();

	/// \brief synchronize the buffers in the object..do not call by yourself.
	void syncBuffers(){
	}
	/// \brief event handler
	void handleEvent(smEvent *p_event);
};

#endif