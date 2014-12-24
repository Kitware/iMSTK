#ifndef COLLISIONDETECTIONEXAMPLE_H
#define COLLISIONDETECTIONEXAMPLE_H

#include "smCore/smConfig.h"
#include "smCore/smErrorLog.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSimulator.h"
#include "smCore/smStaticSceneObject.h"
#include "smCore/smSceneObject.h"
#include "smSimulators/smDummySimulator.h"
#include "smExternalDevices/smPhantomInterface.h"
#include "smUtilities/smMotionTransformer.h"
#include "smCollision/smSpatialGrid.h"


class CollisionDetectionExample:public smSimulationMain,public smCoreClass{

public:
	smStaticSceneObject  *object1;
	smStaticSceneObject  *object2;
	smSDK* simmedtkSDK;
	smScene *scene1;
	smDummySimulator *dummySim;
	smViewer *viewer;
	smSimulator *simulator;
	smPipeRegisteration myCollInformation;
	smPhantomInterface* hapticInterface;
	smHapticCameraTrans *motionTrans;
	smSpatialGrid *spatGrid;
	smLattice *lat;
	smLattice *lat2;

	CollisionDetectionExample();
	void initHapticCamMotion();

	virtual void simulateMain(smSimulationMainParam p_param){
	}

	void CollisionDetectionExample::draw(smDrawParam p_params);
	~CollisionDetectionExample();
};

void collisionDetectionExample();

#endif
