/*
****************************************************
                SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMINTERFACE_H
#define SMINTERFACE_H

#include "smCore/smConfig.h"
#include "smCore/smModule.h"
#include "smCore/smEventHandler.h"
#include "smUtilities/smVec3.h"
#include "smUtilities/smMatrix44.h"
#include "smUtilities/smMatrix33.h"

/// Declarations and constants for the device
const int	SIMMEDTK_MSG_UNKNOWN = -2;
const int 	SIMMEDTK_MSG_FAILURE = -1;
const int	SIMMEDTK_MSG_SUCCESS =  0;

/// \brief Abstract base interface class with virtual functions. 
/// Device specific implementation should be done by instantiating this class
class smDeviceInterface: public smModule, smEventHandler {

public:
	/// \brief constructor
	smDeviceInterface();

	/// \brief destructor
	virtual ~smDeviceInterface(){

	};

	/// \brief open the device
	virtual int openDevice(){ 
		return SIMMEDTK_MSG_UNKNOWN;
	}

	/// \brief close the device
	virtual int closeDevice(){
		return SIMMEDTK_MSG_UNKNOWN;
	}

	/// \brief write data (for ADU interface device)
	virtual int write(void *Interfacehandle, smInt port, void *data){
		return  SIMMEDTK_MSG_UNKNOWN;
	};

	/// \brief read data (for ADU interface device)
	virtual int read(void *Interfacehandle, smInt port, void *data){
		return SIMMEDTK_MSG_UNKNOWN;
	} ;

protected :
	smBool driverInstalled; ///< true if device driver is installed
};

#endif
