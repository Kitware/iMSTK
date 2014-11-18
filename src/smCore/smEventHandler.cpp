#include "smCore/smEventHandler.h"
#include "smCore/smEvent.h"

///calls all the Event Handlers and then Delete the event

void smEventDispatcher::registerEventHandler(smEventHandler *p_handler,smEventType p_eventType){
	smEventHandlerHolder *holder;
	holder=new smEventHandlerHolder();
	holder->handler=p_handler;
	holder->registeredEventType=p_eventType;
	handlers.push_back(holder);

}

void smEventDispatcher::disableEventHandler(smEventHandler *p_handler, smEventType p_eventType){
	for(smInt i=0;i<handlers.size();i++){
		if(handlers[i]->handler==p_handler&&handlers[i]->registeredEventType==p_eventType)
			handlers[i]->enabled=false;
	}	

}
void smEventDispatcher::enableEventHandler(smEventHandler *p_handler, smEventType p_eventType){
	for(smInt i=0;i<handlers.size();i++){
		if(handlers[i]->handler==p_handler&&handlers[i]->registeredEventType==p_eventType)
			handlers[i]->enabled=true;
	}	

}

///Synchronous Event calling..Be aware that the calling of the functions are not serialized.
///The serialization should be done within the handler function.
void smEventDispatcher::sendStreamEvent(smEvent *p_event){

			  //p_event->messageId=messageId.fetchAndAddOrdered(1);	
			   callHandlers(p_event);
			   
			  

}
void smEventDispatcher::sendEventAndDelete(smEvent *p_event){

			  //p_event->messageId=messageId.fetchAndAddOrdered(1);	
			  callHandlers(p_event);
			   delete p_event;

			  

}


///asynchronous Event calling 
///not implemented yet
void smEventDispatcher::asyncSendEvent(smEvent *p_event){
			 //callHandlers(p_event)
}










