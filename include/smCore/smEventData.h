													   /*
****************************************************
                  SOFMIS LICENSE

****************************************************

    \author:    <http:\\acor.rpi.edu>
                SOFMIS TEAM IN ALPHABATIC ORDER
                Anderson Maciel, Ph.D.
                Ganesh Sankaranarayanan, Ph.D.
                Sreekanth A Venkata
                Suvranu De, Ph.D.
                Tansel Halic
                Zhonghua Lu

    \author:    Module by Tansel Halic
                
                
    \version    1.0
    \date       04/2009
    \bug	    None yet
    \brief	    This file contains the Event Data  

*****************************************************
*/

#ifndef SMEVENTDATA_H
#define SMEVENTDATA_H
#include "smCore/smConfig.h"
#include "smUtilities/smVec3.h"
#include "smUtilities/smMatrix44.h"

#include <QKeyEvent>
using namespace std;





struct smKeyboardEventData{
   // Key keyBoardKey;
	smInt keyBoardKey;
};

struct smMouseEventData{
	//smMouseButtonType mouseType;
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

