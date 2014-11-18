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

    \author:    Module by Ganesh Sankaranarayanan
                
                
    \version    1.0
    \date       09/2009
    \bug	    None yet
    \brief	    This Module is the basic Interface for interaction in SoFMIS
    

*****************************************************

*/


#ifndef SM_HAPTIC_INTERFACE_H
#define SM_HAPTIC_INTERFACE_H

#include "smDeviceInterface.h"
#include "smCore/smEvent.h"
#include "smCore/smEventData.h"
#include "smCore/smEventHandler.h"

#define SM_MAX_BUTTONS 4

struct hapticDeviceData_t {

	smInt deviceID;
	
	smString deviceName;

	smVec3 <smDouble> position;

	smVec3 <smDouble> velocity;

	smVec3 <smDouble> angles;

	smMatrix44 <smDouble> transform;

	smBool buttonState[SM_MAX_BUTTONS];

};
 



class smHapticInterface: public smDeviceInterface    
{

public:

	smHapticInterface();

	virtual ~smHapticInterface(){};


	virtual int openDevice(){ return SOFMIS_MSG_UNKNOWN;}

	

	virtual int closeDevice() {return SOFMIS_MSG_UNKNOWN;}

	virtual int startDevice(){ return SOFMIS_MSG_UNKNOWN;}
	

	virtual int  getPosition(smVec3<smDouble> & d_pos) {return SOFMIS_MSG_UNKNOWN;}
	

	virtual int getOreintation(smMatrix33 <smDouble> *d_rot) {return SOFMIS_MSG_UNKNOWN;}

	virtual int getDeviceTransform(smMatrix44 <smDouble> *d_transform) {return SOFMIS_MSG_UNKNOWN;}


	virtual int setForce (smVec3<smDouble> & force) {return SOFMIS_MSG_UNKNOWN;}

	virtual int setForceandTorque(smVec3 <smDouble>& force, smVec3 <smDouble> & torque) {return SOFMIS_MSG_UNKNOWN;}


protected:

	//smHapticData  hapticDeviceData;

};


#endif