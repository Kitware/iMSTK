/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMEVENTDATA_H
#define SMEVENTDATA_H

#include "smCore/smConfig.h"
#include "smUtilities/smVec3.h"
#include "smUtilities/smMatrix44.h"
#include <QKeyEvent>

using namespace std;

struct smKeyboardEventData{
	smInt keyBoardKey;
};

struct smMouseEventData{
	smInt windowX;
	smInt windowY;
};

struct smObjectClickedData{
	smInt objectId;
	smVec3<smFloat> clickedPosition;
};

struct smSynchEventData{
};

struct smHapticOutEventData{
	smInt deviceId;
	smString deviceName;
	smVec3 <smDouble> position;
	smVec3 <smDouble> velocity;
	smVec3 <smDouble> angles;
	smMatrix44<smDouble> transform;
	smBool buttonState[4];//will be chnage later on
};

struct smHapticInEventData{
	smInt deviceId;
	smVec3 <smDouble> force;
	smVec3 <smDouble> torque;

};

template <class smCollisionData>
struct smObjectHitData{
	smInt objectSourceId;
	smInt objectTargetId;
	smCollisionData data;
};

struct smCameraEventData{
	smVec3<smDouble> pos;
	smVec3<smDouble> direction;
	smVec3<smDouble> upDirection;
};

struct smLightMotionEventData{
	smInt lightIndex;
	smVec3<smDouble> pos;
	smVec3<smDouble> direction;
};

#endif
