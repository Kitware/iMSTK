#ifndef SMEFFECTSIM_H
#define SMEFFECTSIM_H

#include "smCore/smConfig.h"
#include "smCore/smObjectSimulator.h"
#include "smCore/smErrorLog.h"
#include "smCore/smEventHandler.h"
#include "smCore/smEventData.h"
#include "smRenderEffects/SmokeEffect.h"

class smEffectSim:public smStaticSceneObject, public smEventHandler{

public:
	SmokeVideo *smoke;
	bool enabled;
	smBool buttonState;
	double left[16];
	double normalizedToolTipX,normalizedToolTipY;

	///for local mesh based effects..
	void addSparkMesh(smSurfaceMesh *p_mesh){
		smoke->spark->addSparkMesh(p_mesh);
	}

	smEffectSim(smErrorLog *p_errorLog):smStaticSceneObject(p_errorLog){
		buttonState=false;
		normalizedToolTipX=0;
		normalizedToolTipY=0;
		smoke=new SmokeVideo();
	}

protected:
	void initDraw(smDrawParam p_params){

	smoke->Initialize();
	}

	virtual void draw(smDrawParam p_params);

	virtual void initCustom(){
	}

	//test
	virtual void run();

	///synchronize the buffers in the object..do not call by yourself.
	void syncBuffers(){
	}

	void handleEvent(smEvent *p_event);
};

#endif