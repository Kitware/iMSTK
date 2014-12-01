
#ifndef SMMYSTYLUS_H
#define SMMYSTYLUS_H

#include "smSimulators/smStylusObject.h"
class smMeshContainer;

class MyStylus:public smStylusRigidSceneObject{
public:
	smInt phantomID;
	smBool buttonState[2];
	smFloat angle;
	smFloat maxangle;
	smMeshContainer meshContainer,meshContainerLower,meshContainerUpper;

	void handleEvent(smEvent *p_event);
	MyStylus(smChar *p_shaft="../../resources/models/blunt_diss_pivot.3DS",
	         smChar *p_lower="../../resources/models/blunt_diss_lower.3DS",
	         smChar *p_upper="../../resources/models/blunt_diss_upper.3DS");
	void updateOpenClose();
};

class HookCautery:public smStylusRigidSceneObject{
public:
	smInt phantomID;
	smBool buttonState[2];
	smMeshContainer meshContainer;

	void handleEvent(smEvent *p_event);
	void HookCautery::draw(smDrawParam p_param);
	HookCautery(smChar *p_pivot="../../resources/models/hook_cautery_new.3DS");
};

#endif
