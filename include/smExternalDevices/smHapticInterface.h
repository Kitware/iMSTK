/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SM_HAPTIC_INTERFACE_H
#define SM_HAPTIC_INTERFACE_H

#include "smDeviceInterface.h"
#include "smCore/smEvent.h"
#include "smCore/smEventData.h"
#include "smCore/smEventHandler.h"

#define SM_MAX_BUTTONS 4

/// \brief holds haptic device data
struct hapticDeviceData_t {
	smInt deviceID;
	smString deviceName;
	smVec3 <smDouble> position;
	smVec3 <smDouble> velocity;
	smVec3 <smDouble> angles;
	smMatrix44 <smDouble> transform;
	smBool buttonState[SM_MAX_BUTTONS];
};

/// \brief Absract base class for haptic device
class smHapticInterface: public smDeviceInterface{

public:

	/// \brief constructor
	smHapticInterface();

	/// \brief destructor
	virtual ~smHapticInterface(){};

	/// \brief open haptic device
	virtual int openDevice(){
		return SOFMIS_MSG_UNKNOWN;
	}

	/// \brief close haptic device
	virtual int closeDevice(){
		return SOFMIS_MSG_UNKNOWN;
	}

	/// \brief start the haptic device 
	virtual int startDevice(){
		return SOFMIS_MSG_UNKNOWN;
	}

	/// \brief get the position of the end effector the haptic device
	virtual int  getPosition(smVec3<smDouble> & d_pos){
		return SOFMIS_MSG_UNKNOWN;
	}

	/// \brief get the orientation of the end effector the haptic device
	virtual int getOreintation(smMatrix33 <smDouble> *d_rot){
		return SOFMIS_MSG_UNKNOWN;
	}

	/// \brief get the transform (position + orientation) of the end effector the haptic device
	virtual int getDeviceTransform(smMatrix44 <smDouble> *d_transform){
		return SOFMIS_MSG_UNKNOWN;
	}

	/// \brief set force to the haptic device
	virtual int setForce (smVec3<smDouble> & force){
		return SOFMIS_MSG_UNKNOWN;
	}

	/// \brief set torque to the haptic device
	virtual int setForceandTorque(smVec3 <smDouble>& force, smVec3 <smDouble> & torque){
		return SOFMIS_MSG_UNKNOWN;
	}

};

#endif
