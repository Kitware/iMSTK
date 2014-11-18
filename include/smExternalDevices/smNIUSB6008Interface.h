#include "smCore/smConfig.h"

#ifndef SOFMIS_OPERATINGSYSTEM_LINUX

#ifndef SM_NIUSB6008_INTERFACE_H
#define SM_NIUSB6008_INTERFACE_H

#include <qapplication.h>
#include <QThread>

#include <NIDAQmx.h>
//#define smNIUSB6008DAQ 1

#include "smDeviceInterface.h"
#include "smCore/smEvent.h"
#include "smCore/smEventData.h"
#include "smCore/smEventHandler.h"
#include "smCore/smDoubleBuffer.h"
#include "smUtilities/smTimer.h"

struct NIUSB6008Data{
	smBool on;
	smFloat value[3];	
};

struct toolData{
	char type[5];
	float32 min;
	float32 max; 
	float32 invRange;
};

class smNIUSB6008Interface: public smDeviceInterface, public QThread
{

public:
	smNIUSB6008Interface(int VBLaST_Task_ID);
	~smNIUSB6008Interface();
	
	smPipe *NIUSB6008pipe;
	
	int32 NI_error;
	char NI_errBuff[2048];
	smInt initCount;
	smBool NI_on;

	float32 minValue[2];
	float32 maxValue[2];
	float32 invRange[2];
		
	int32 count;
	float64     sampdata[30];
	TaskHandle  taskHandle;
	float64		aveData[3];

	smInt nbrRegTool;
	toolData *regTool;

	smInt nbrTotalChannel; 
	toolData installedTool[8];		// total number of AI channels of the device = 8

	smInt nbrActiveChannel;
	smInt activeChannel[3];			// maximum number of active channel is 3

	smInt taskID;
	
public:
	void init(){};
	void exec(){this->start();};
	void run();
	void handleEvent(smEvent *p_event){};
	
	///empty functions for now
	virtual void beginFrame(){};
	///empty functions for now
	virtual void endFrame(){};

	void sendDataToPipe();
	void initNI_Error(int32 error);

	void getToolCalibrationData();
	void setTool();
	void getToolData(smInt nc, smInt *ac);

	friend int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle, int32 everyNsamplesEventType, uInt32 nSamples, void *callbackData);
	friend int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, void *callbackData);	
};

#endif

#endif //SOFMIS_OPERATINGSYSTEM_LINUX
