#ifndef VAORENDERING_H
#define VAORENDERING_H

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

class VAORendering:public smSimulationMain,public smCoreClass{

public:
	smStaticSceneObject  *object1;
	smStaticSceneObject  *object2;
	smSDK* simmedtkSDK;
	smScene *scene1;
	smDummySimulator *dummySim;
	smViewer *viewer;
	smSimulator *simulator;
	smPhantomInterface* hapticInterface;

	VAORendering();
	void initHapticCamMotion();

	virtual void simulateMain(smSimulationMainParam p_param){

		for(int i=0;i<object1->mesh->nbrVertices;i++){
			object1->mesh->vertices[i].x+=0.0000001;
			if(object1->mesh->vertices[0].x>12){
				for(smInt j=0;j<object1->mesh->nbrTriangles;j++){
					object1->mesh->triangles[j].vert[0]=0;
					object1->mesh->triangles[j].vert[1]=1;
					object1->mesh->triangles[j].vert[2]=2;
				}
			}
		}
	}

	void VAORendering::draw(smDrawParam p_params);
	~VAORendering();
};

#endif
