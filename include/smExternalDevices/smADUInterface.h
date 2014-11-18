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
    \date       12/2010
    \bug	    None yet
    \brief	    This Module is the ADU Interface from Ontrak Control Systems Inc.  for interaction in SoFMIS
    

*****************************************************

*/
#ifdef _WIN32 //right now adutux(aduhid on linux) requires a different interface
#ifndef SM_ADU_INTERFACE_H
#define SM_ADU_INTERFACE_H

#include "smDeviceInterface.h"
#include "smCore/smEvent.h"
#include "smCore/smEventData.h"
#include "smCore/smEventHandler.h"
#include <qapplication.h>
#include <QThread>
#include "smCore/smDoubleBuffer.h"
#include "smUtilities/smTimer.h"



// Comment / uncomment for using interface
//#define USING_INTERFACE

//class smPipe;

/// Container for holding the device data
//struct ADUDeviceData{
//
//
//	smUint anValue[3];
//};


struct ADUDeviceCalibrationData{

	smUInt maxValue1;
	smUInt minValue1;
	smUInt maxValue2;
	smUInt minValue2;

};

/// Container for holding the device data
struct ADUDeviceData{

	smBool deviceOpen;
	smUInt calibration[4];
	smUInt anValue[3];	
	
};



class smADUInterface: public smDeviceInterface, public QThread
{
public:
	smADUInterface();
	/// Open the ADU device with all necessary data recorded in a file
	smADUInterface(char *calibrationFile);
	~smADUInterface();

	/// Open the ADU device specified by the serial number
	smInt openDevice(char *serialNumber);

	/// Close the ADU device
	smInt closeDevice();

	/// Function to be called in a thread for getting device data
	void runDevice();

	smInt readAnalogInput(smInt channel);
	smInt *readAnalogInputs();

	smInt sw;
	smBool updateFlag;
	// smInt write(void *Interfacehandle, void *data); 
	//smInt read(void *Interfacehandle, void *data); 
	//void readData(void *InterfaceHandle, smInt port);
	void *deviceHandle;
	ADUDeviceCalibrationData *calibrationData;
	ADUDeviceData *deviceData;
	smChar *serialNumber;
	smBool isOpened;

	void init();
	void exec();
	//empty functions for now
	virtual void beginFrame(){};
	///empty functions for now
	virtual void endFrame(){};
	void draw(smDrawParam p_params){};

	void run();

	void handleEvent(smEvent *p_event){};

public:
	smPipe *ADUpipe;
	void sendDataToPipe();
};

#endif
#endif //WIN32 include
