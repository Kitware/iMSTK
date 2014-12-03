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

/// \brief !!
class CSystem{
public:
	SYSTEM_CONFIGURATION	m_config;
}; 

/// \brief !!
class CSensor{
public:
	SENSOR_CONFIGURATION	m_config;
};

/// \brief !!
class CXmtr{
public:
	TRANSMITTER_CONFIGURATION m_config;
};

/// \brief interface class for ATC device
class smATC3DGInterface: public smDeviceInterface, public QThread{

public:
	/// \brief constructor
	smATC3DGInterface();

	/// \brief constructor
	smATC3DGInterface(double dataRate);//:rate(dataRate){}

	/// \brief destructor
	~smATC3DGInterface();

	CSystem tracker; ///< Configuation of the tracker system
	CSensor *pSensor; ///<  Configuration of sensors
	CXmtr *pXmtr; ///< Configuaration of the transmitter
	int errorCode; ///< Returned error code for debug
	double rate;  ///< tracker update rate -- if not set then it will be the device defualt

	DOUBLE_POSITION_MATRIX_TIME_Q_RECORD *pRecord; ///< !!
	DOUBLE_POSITION_MATRIX_TIME_Q_RECORD record[32]; ///< !!

	/// \brief Open the 3DG device
	void openATC3DGSystem();

	/// \brief Close the 3DG device
	void closeATC3DGSystem();

	/// \brief Function to be called in a thread for getting device data
	void runDevice();

	/// \brief get errors if any
	void errorHandler(int error);

	/// \brief set the sensor data format
	void setSensorDataFormat();

	/// \brief get the data from sensor
	void collectSensorData();

	/// \brief !! nothing happens here
	void init();

	/// \brief !! start the ATC interface
	void exec();

	/// \brief empty functions for now
	virtual void beginFrame(){};

	/// \brief empty functions for now
	virtual void endFrame(){};

	/// \brief !! not used
	void draw(smDrawParam p_params){};

	/// \brief run the interface in a loop
	void run();

	/// \brief 
	void handleEvent(smEvent *p_event){};

public:
	smPipe *ATC3DGpipe; ///< !!

	/// \brief !!
	void sendDataToPipe();
};

#endif
