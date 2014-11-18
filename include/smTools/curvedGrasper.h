#ifndef CURVEDGRASPER_H
#define CURVEDGRASPER_H

#include "smSimulators/smStylusObject.h"
#include "smMesh/smSurfaceMesh.h"

//#include "smADUInterface.h"
#include "smExternalDevices/smNIUSB6008Interface.h"


class curvedGrasper:public smStylusRigidSceneObject{

public:
	smInt phantomID;
	smBool buttonState[2];
	smFloat angle;
	smFloat maxangle;

	smMeshContainer meshContainer_pivot;
	smMeshContainer meshContainer_lowerJaw;
	smMeshContainer meshContainer_upperJaw;

	smSurfaceMesh *mesh_pivot;
	smSurfaceMesh *mesh_lowerJaw;
	smSurfaceMesh *mesh_upperJaw;

	curvedGrasper(smInt ID,
		smChar * p_pivotModelFileName="../../resources/models/curved_pivot.3DS",
		smChar *p_lowerModelFileName="../../resources/models/curved_upper.3DS",
		smChar *p_upperModelFileName="../../resources/models/curved_lower.3DS"
		
		);

	// for jaw interface
	//smPipeRegisteration ADUpipeReg;
	#ifdef smNIUSB6008DAQ
	smPipeRegisteration NIUSB6008pipeReg;
	#endif
	smVec3d godPos;
	smFloat godMat[9];

	
	smInt DAQdataID;

	smFloat minValue;
	smFloat maxValue;
	smFloat invRange;

public:
	void curvedGrasper::draw(smDrawParam p_params);
	void handleEvent(smEvent *p_event);

	 void updateOpenClose();
};


#endif
