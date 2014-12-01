/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMPHANTOMINTERFACE_H
#define SMPHANTOMINTERFACE_H

#include "smHapticInterface.h"
#include <HD/hd.h>
#include <HDU/hduVector.h>
#include <HDU/hduError.h>
#include "smCore/smEventHandler.h"

const int SM_MAX_PHANTOM_DEVICES = 4;

class smPhantomInterface: public smHapticInterface, public smEventHandler{

protected:
	smEvent *hapticEvent[SM_MAX_PHANTOM_DEVICES];
	smHapticOutEventData *hapticEventData[SM_MAX_PHANTOM_DEVICES];

public:
	smBool forceEnabled;

	smPhantomInterface();
	~smPhantomInterface();
	smInt openDevice();
	smInt closeDevice();
	smInt openDevice(smInt phantomNumber) ;
	smInt openDevice(smString phantomName);
	smInt startDevice();
	smInt getPosition(smVec3 <smDouble>& d_pos);
	smInt getOreintation(smMatrix33 <smDouble> *d_rot);
	smInt getDeviceTransform(smMatrix44 <smDouble> *d_transform);
	HHD dHandle[SM_MAX_PHANTOM_DEVICES];

	smInt numPhantomDevices;
	hduVector3Dd  position[SM_MAX_PHANTOM_DEVICES];
	hduVector3Dd  velocity[SM_MAX_PHANTOM_DEVICES];
	hduVector3Dd  angles[SM_MAX_PHANTOM_DEVICES];
	hduVector3Dd  force[SM_MAX_PHANTOM_DEVICES];
	hduVector3Dd  torque[SM_MAX_PHANTOM_DEVICES]; 
	smDouble transform[SM_MAX_PHANTOM_DEVICES][16];
	hapticDeviceData_t hapticDeviceData[SM_MAX_PHANTOM_DEVICES];
	HDSchedulerHandle hapticCallbackHandle;

	smString phantomDeviceNames[SM_MAX_PHANTOM_DEVICES];

	///empty functions for now
	virtual void beginFrame(){};

	///empty functions for now
	virtual void endFrame(){};

	friend HDCallbackCode HDCALLBACK hapticCallback(void *pData);

	void handleEvent(smEvent *p_event);
	void init();
	void exec();
	void draw(smDrawParam p_params);

};

#endif
