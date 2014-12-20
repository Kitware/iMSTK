#include "smCore/smConfig.h"

#ifndef SIMMEDTK_OPERATINGSYSTEM_LINUX

#ifndef SM_NIUSB6008_INTERFACE_H

#define SM_NIUSB6008_INTERFACE_H

#include <qapplication.h>
#include <QThread>
#include <NIDAQmx.h>
#include "smDeviceInterface.h"
#include "smCore/smEvent.h"
#include "smCore/smEventData.h"
#include "smCore/smEventHandler.h"
#include "smCore/smDoubleBuffer.h"
#include "smUtilities/smTimer.h"

/// \brief National Instruments USB6008 related data
struct NIUSB6008Data{
	smBool on;
	smFloat value[3];
};

/// \brief data related to tools used for laparoscopic surgery simulators
struct toolData{
	char type[5];
	float32 min;
	float32 max; 
	float32 invRange;
};

/// \brief National Instruments USB6008 interface class
class smNIUSB6008Interface: public smDeviceInterface, public QThread{

public:
	smNIUSB6008Interface(int VBLaST_Task_ID);
	~smNIUSB6008Interface();

	smPipe *NIUSB6008pipe; ///<
	int32 NI_error; ///<
	char NI_errBuff[2048]; ///<
	smInt initCount; ///<
	smBool NI_on; ///<
	float32 minValue[2]; ///<
	float32 maxValue[2]; ///<
	float32 invRange[2]; ///<
	int32 count; ///<
	float64 sampdata[30]; ///<
	TaskHandle taskHandle; ///<
	float64 aveData[3]; ///<
	smInt nbrRegTool; ///<
	toolData *regTool; ///<
	smInt nbrTotalChannel; ///<
	toolData installedTool[8]; ///< total number of AI channels of the device = 8
	smInt nbrActiveChannel; ///<
	smInt activeChannel[3]; ///< maximum number of active channel is 3s
	smInt taskID;

public:

	/// \brief Initialize NIUSB6008 device
	void init(){		
	};

	/// \brief start the device
	void exec(){
		this->start();
	};

	/// \brief !!
	void run();

	/// \brief handle event related to NIUSB6008 device
	void handleEvent(smEvent *p_event){		
	};

	/// \brief empty functions for now
	virtual void beginFrame(){		
	};

	/// \brief empty functions for now
	virtual void endFrame(){		
	};

	/// \brief !!
	void sendDataToPipe();

	/// \brief !!
	void initNI_Error(int32 error);

	/// \brief get tool calibration data from external file (hard-coded file name)
	void getToolCalibrationData();

	/// \brief !!
	void setTool();

	/// \brief get the data form registered tools
	void getToolData(smInt nc, smInt *ac);

	/// \brief !!
	friend int32 CVICALLBACK EveryNCallback(TaskHandle taskHandle,
	                                        int32 everyNsamplesEventType,
	                                        uInt32 nSamples, void *callbackData);
	/// \brief !!
	friend int32 CVICALLBACK DoneCallback(TaskHandle taskHandle, int32 status, 
                                          void *callbackData);
};

#endif

#endif //SIMMEDTK_OPERATINGSYSTEM_LINUX
