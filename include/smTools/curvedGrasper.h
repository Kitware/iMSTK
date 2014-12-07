#ifndef CURVEDGRASPER_H
#define CURVEDGRASPER_H

#include "smSimulators/smStylusObject.h"
#include "smMesh/smSurfaceMesh.h"
#include "smExternalDevices/smNIUSB6008Interface.h"
/// \brief Cruver Grasper tool
class curvedGrasper:public smStylusRigidSceneObject{
public:
	/// \brief phantom device ID that will be listened
	smInt phantomID;
	/// \brief buttons states of haptic device
	smBool buttonState[2];

	/// \brief angle of the jaws
	smFloat angle;
	/// \brief maximum angle that jaws can open
	smFloat maxangle;

	/// \brief the pivto mesh container
	smMeshContainer meshContainer_pivot;
	/// \brief lower jaw container
	smMeshContainer meshContainer_lowerJaw;

	/// \brief upper jaw container
	smMeshContainer meshContainer_upperJaw;

	/// \brief stores the pivot mesh
	smSurfaceMesh *mesh_pivot;
	/// \brief stores lower jaw mesh
	smSurfaceMesh *mesh_lowerJaw;
	/// \brief stores upper mesh jaw
	smSurfaceMesh *mesh_upperJaw;

	/// \brief constrcutor that gest hatpic device ID (e.g. 0 or 1), pivot, lower and upper mesh file names
	curvedGrasper(smInt ID,
	              smChar * p_pivotModelFileName="../../resources/models/curved_pivot.3DS",
	              smChar *p_lowerModelFileName="../../resources/models/curved_upper.3DS",
	              smChar *p_upperModelFileName="../../resources/models/curved_lower.3DS");

	/// \brief for jaw interface
	//smPipeRegisteration ADUpipeReg;
	#ifdef smNIUSB6008DAQ
		smPipeRegisteration NIUSB6008pipeReg;
	#endif
	/// \brief god object position
	smVec3d godPos;
	/// \brief  god object matrix
	smFloat godMat[9];
	/// \brief  interface for DAQ
	smInt DAQdataID;
	/// \brief read  min  data value
	smFloat minValue;
	/// \brief read  max  data value
	smFloat maxValue;
	/// \brief 1/range value
	smFloat invRange;

public:
	/// \brief rendering the curved grasper
	void curvedGrasper::draw(smDrawParam p_params);
	/// \brief event handler
	void handleEvent(smEvent *p_event);
	/// \brief for open and close motion
	void updateOpenClose();
};

#endif
