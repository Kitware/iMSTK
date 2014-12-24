/*
****************************************************
                SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMEVENT_H
#define SMEVENT_H

#include "smCore/smEventData.h"

class smEventDispatcher;
class smMemoryBlock;

///for asycnronous call
enum smEventPriority
{
    SIMMEDTK_EVENTPRIORITY_HIGHEST,
    SIMMEDTK_EVENTPRIORITY_HIGH,
    SIMMEDTK_EVENTPRIORITY_NORMAL,
    SIMMEDTK_EVENTPRIORITY_LOW,
    SIMMEDTK_EVENTPRIORITY_LOWEST,
};

///We need to add more events.
#define SIMMEDTK_EVENTTYPE_ALL 0
#define SIMMEDTK_EVENTTYPE_TERMINATE  1
#define SIMMEDTK_EVENTTYPE_OBJECTCLICKED 2
#define SIMMEDTK_EVENTTYPE_OBJECT2OBJECTHIT 3
#define SIMMEDTK_EVENTTYPE_OBJECT2CAMERAHIT 4
#define SIMMEDTK_EVENTTYPE_KEYBOARD 5
#define SIMMEDTK_EVENTTYPE_SYNCH 6
#define SIMMEDTK_EVENTTYPE_HAPTICOUT 7
#define SIMMEDTK_EVENTTYPE_HAPTICIN 8
#define SIMMEDTK_EVENTTYPE_CAMERA_UPDATE 9
#define SIMMEDTK_EVENTTYPE_LIGHTPOS_UPDATE 10
#define SIMMEDTK_EVENTTYPE_AUDIO 11
#define SIMMEDTK_EVENTTYPE_RESERVE 1000
#define SIMMEDTK_EVENTTYPE_NONE 1001

class smEventType
{
public:
    smInt eventTypeCode;
    smEventType();
    smEventType(smInt p_eventType);
    smEventType & operator=(smInt p_eventTypeCode);
    smBool operator ==(smEventType &p_event);
    smBool operator ==(smInt p_eventTypeCode);
};

enum smSenderType
{
    SIMMEDTK_SENDERTYPE_MODULE,
    SIMMEDTK_SENDERTYPE_SCENEOBJECT,
    SIMMEDTK_SENDERTYPE_EVENTSOURCE,
};

///this class is for particular entity that  that sends event
class smEventSource
{

};

class smEventBase
{

};
/// \brief  event class. One method of message sending among the framework
class smEvent
{

public:
    /// \brief  points to the data
    void *data;
    /// \brief priority of event
    smEventPriority priority;
    /// \brief  event type
    smEventType eventType;
    /// \brief  sender type
    smSenderType senderType;
    /// \brief  sender id
    smInt senderId;
    /// \brief constrcutor
    smEvent();
    /// \brief  destructor
    ~smEvent();

private:
    /// \brief  message ID
    smInt messageId;
    friend smEventDispatcher;

};
/// \brief stream event
class smStreamEvent: public smEvent
{

public:
    /// \brief time stamp
    smInt timeStamp;
    smStreamEvent()
    {
    }

    virtual smBool allocate(smMemoryBlock *p_block) = 0;
};

#endif
