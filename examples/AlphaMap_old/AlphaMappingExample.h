#ifndef ALPHAMAPPINGEXAMPLE_H
#define ALPHAMAPPINGEXAMPLE_H

#include "smCore/smConfig.h"
#include "smCore/smErrorLog.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSimulator.h"
#include "smCore/smStaticSceneObject.h"
#include "smCore/smSceneObject.h"
#include "smSimulators/smDummySimulator.h"
#include "smExternalDevices/smPhantomInterface.h"
#include "smUtilities/smMotionTransformer.h"
#include "smShader/metalShader.h"

class AlphaMapExample:public smSimulationMain,public smCoreClass{

public:
	smStaticSceneObject  *object1;
	smSDK* sofmisSDK;
	smScene *scene1;
	smViewer *viewer;

	smPhantomInterface* hapticInterface;
	smHapticCameraTrans *motionTrans;

	AlphaMapExample();
	void initHapticCamMotion();

	virtual void simulateMain(smSimulationMainParam p_param){
	}

	void AlphaMapExample::draw(smDrawParam p_params);
	~AlphaMapExample();
};

#endif
