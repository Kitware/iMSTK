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


#ifndef SMINTERFACE_H
#define SMINTERFACE_H

#include "smCore/smConfig.h"
#include "smCore/smModule.h"
#include "smCore/smEventHandler.h"
#include "smUtilities/smVec3.h"
#include "smUtilities/smMatrix44.h"
#include "smUtilities/smMatrix33.h"

/// Declarations and constants for the device



const int	SOFMIS_MSG_UNKNOWN = -2;
const int 	SOFMIS_MSG_FAILURE = -1;
const int	SOFMIS_MSG_SUCCESS =  0;
	


/// Abstract interface class with virtual functions. Device specific implementation should be done by instantiating this class
class smDeviceInterface: public smModule, smEventHandler {
public:
	smDeviceInterface();

	virtual ~smDeviceInterface(){};

	


	virtual int openDevice(){ return SOFMIS_MSG_UNKNOWN;}

	virtual int closeDevice() { return SOFMIS_MSG_UNKNOWN;}

	/// For ADU Interface Device
	virtual int write(void *Interfacehandle, smInt port, void *data) {return  SOFMIS_MSG_UNKNOWN; } ;

	/// FOR ADU Interface Device 
	virtual int read(void *Interfacehandle, smInt port, void *data) { return SOFMIS_MSG_UNKNOWN; } ;


protected :

	smBool driverInstalled;

	

};



#endif