#ifndef COLLISIONDETECTIONEXAMPLEWITHHASH_H
#define COLLISIONDETECTIONEXAMPLEWITHHASH_H

#include "smCore/smConfig.h"
#include "smCore/smErrorLog.h"
#include "smCore/smCoreClass.h"
#include "smCore/smSimulator.h"
#include "smCore/smStaticSceneObject.h"
#include "smCore/smSceneObject.h"
#include "smSimulators/smDummySimulator.h"
#include "smCollision/smSpatialHash.h"

class CollisionDetectionExampleWithHash:public smSimulationMain, public smCoreClass{

public:
	smStaticSceneObject  *object1;
	smStaticSceneObject  *object2;
	smSDK* sofmisSDK;
	smScene *scene1;
	smDummySimulator *dummySim;
	smViewer *viewer;
	smSimulator *simulator;
	smSpatialHash *hash;
	smPipeRegisteration myCollInformation;

	CollisionDetectionExampleWithHash();
	virtual void draw(smDrawParam p_params);
	virtual void simulateMain(smSimulationMainParam p_param);
	~CollisionDetectionExampleWithHash();
};

#endif
