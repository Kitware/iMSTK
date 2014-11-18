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

    \author:    Module by Tansel Halic
                
                
    \version    1.0
    \date       04/2009
    \bug	    None yet
    \brief	    This file contains the Events and Event Types  

*****************************************************
*/

#ifndef SMEVENT_H
#define SMEVENT_H

//#include "smEventHandler.h"
#include "smCore/smEventData.h"



class smEventDispatcher;
class smMemoryBlock;

 ///for asycnronous call
enum smEventPriority{
	SOFMIS_EVENTPRIORITY_HIGHEST,
	SOFMIS_EVENTPRIORITY_HIGH,
	SOFMIS_EVENTPRIORITY_NORMAL,
	SOFMIS_EVENTPRIORITY_LOW,
	SOFMIS_EVENTPRIORITY_LOWEST,
};

///We need to add more events. 
//enum smEventType{
#define	SOFMIS_EVENTTYPE_ALL 0 
#define	SOFMIS_EVENTTYPE_TERMINATE  1
#define SOFMIS_EVENTTYPE_OBJECTCLICKED 2
#define SOFMIS_EVENTTYPE_OBJECT2OBJECTHIT 3 
#define SOFMIS_EVENTTYPE_OBJECT2CAMERAHIT 4
#define SOFMIS_EVENTTYPE_KEYBOARD 5
#define SOFMIS_EVENTTYPE_SYNCH 6
#define SOFMIS_EVENTTYPE_HAPTICOUT 7
#define SOFMIS_EVENTTYPE_HAPTICIN 8
#define SOFMIS_EVENTTYPE_CAMERA_UPDATE 9
#define SOFMIS_EVENTTYPE_LIGHTPOS_UPDATE 10
#define SOFMIS_EVENTTYPE_AUDIO 11 


#define SOFMIS_EVENTTYPE_RESERVE 1000
#define SOFMIS_EVENTTYPE_NONE 1001

//};
class smEventType{
public:
	smInt eventTypeCode;
	smEventType();
	smEventType(smInt p_eventType);
	smEventType & operator=(smInt p_eventTypeCode);
	smBool operator ==(smEventType &p_event);
	smBool operator ==(smInt p_eventTypeCode);
};



enum smSenderType{
	SOFMIS_SENDERTYPE_MODULE,
	SOFMIS_SENDERTYPE_SCENEOBJECT,
	SOFMIS_SENDERTYPE_EVENTSOURCE,
};



///this class is for particular entity that  that sends event 
class smEventSource{


};



class smEventBase{

};

class smEvent{
public:
	 void *data;
	 smEventPriority priority;
	 smEventType eventType;
	 smSenderType senderType;
	 smInt senderId;
	 smEvent();/*{
		priority=SOFMIS_EVENTPRIORITY_NORMAL;

	 }*/


private:
	 smInt messageId;
	 
	 friend smEventDispatcher;
public:
	 ~smEvent();/*{
		delete []data;
	 }*/

};


class smStreamEvent:public smEvent{
	public:
		smInt timeStamp;
		smStreamEvent(){
		
		}
		virtual smBool allocate(smMemoryBlock *p_block)=0;
		
};









#endif
