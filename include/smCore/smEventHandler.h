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
    \date       08/2009
    \bug	    None yet
    \brief	    This Module is for Event handling. These have Event Declarations and event handles as well
    

*****************************************************
*/


#ifndef SMEVENTHANDLER_H
#define SMEVENTHANDLER_H
#include "smCore/smCoreClass.h"
#include "smCore/smEvent.h"
#include "smCore/smDispatcher.h"
#include <QVector>


class smEvent;
class smEventType;



struct smEventHolder{
	smEvent *myEvent;
	smBool isActive;
};





class smEventHandler:smInterface{
public:	
	virtual void handleEvent(smEvent *p_event)=0;

};


class smEventHandlerHolder{
public:
	smEventHandlerHolder(){
	  enabled=true;
	  handler=NULL;
	  registeredEventType=SOFMIS_EVENTTYPE_NONE;
   }
   smEventHandler *handler;
   smEventType	  registeredEventType;
   smBool enabled;
};



class smEventDispatcher{
	private:
		smDispatcher *dispatcher;
		vector<smEventHandlerHolder*>handlers;
		
		QAtomicInt messageId;
		smEventHolder eventHolder[SOFMIS_MAX_EVENTSBUFFER];
		//Qmutex eventSendSerialize;





		
	
	public:
		smEventDispatcher(){
			messageId=1;
			
		}
		smEventDispatcher(smDispatcher *p_dispatcher);
		
		void registerEventHandler(smEventHandler *handler, smEventType p_eventType);
		void enableEventHandler(smEventHandler *p_handler, smEventType p_eventType);
		void disableEventHandler(smEventHandler *p_handler, smEventType p_eventType);

		void asyncSendEvent(smEvent *p_event);
		void sendEventAndDelete(smEvent *p_event);
		void sendStreamEvent(smEvent *p_event);
		void fetchEvent();

		inline void callHandlers(smEvent *p_event){
			int v=handlers.size();
			for(smInt i=0;i<v;i++){
				if(handlers[i]->enabled&&p_event->eventType==handlers[i]->registeredEventType||handlers[i]->registeredEventType==SOFMIS_EVENTTYPE_ALL)
					handlers[i]->handler->handleEvent(p_event);
			}
		}

};


#endif