/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SM_ATC_3DG_INTERFACE_H
#define SM_ATC_3DG_INTERFACE_H

#include <qapplication.h>
#include <QThread>
#include "smDeviceInterface.h"
#include "smCore/smEvent.h"
#include "smCore/smEventData.h"
#include "smCore/smEventHandler.h"
#include "smCore/smDoubleBuffer.h"
#include "ATC3DG.h"		// ATC3DG API


class CSystem{
public:
	SYSTEM_CONFIGURATION	m_config;
}; 

class CSensor{
public:
	SENSOR_CONFIGURATION	m_config;
};

class CXmtr{
public:
	TRANSMITTER_CONFIGURATION m_config;
};

class smATC3DGInterface: public smDeviceInterface, public QThread{

public:
	smATC3DGInterface();
	smATC3DGInterface(double dataRate);//:rate(dataRate){}
	~smATC3DGInterface();

	// Variables 
	CSystem tracker; // Configuation of the tracker system
	CSensor *pSensor; //  Configuration of sensors
	CXmtr *pXmtr; // Configuaration of the transmitter
	int errorCode; // Returned error code for debug
	double rate;  // tracker update rate -- if not set then it will be the device defualt

	DOUBLE_POSITION_MATRIX_TIME_Q_RECORD *pRecord;
	DOUBLE_POSITION_MATRIX_TIME_Q_RECORD record[32];

	//Open the 3DG device
	void openATC3DGSystem();

	//Close the 3DG device
	void closeATC3DGSystem();

	/// Function to be called in a thread for getting device data
	void runDevice();

	void errorHandler(int error);
	void setSensorDataFormat();
	void collectSensorData();
	void init();
	void exec();

	///empty functions for now
	virtual void beginFrame(){};

	///empty functions for now
	virtual void endFrame(){};
	void draw(smDrawParam p_params){};
	void run();
	void handleEvent(smEvent *p_event){};

public:
	smPipe *ATC3DGpipe;
	void sendDataToPipe();
};

#endif
