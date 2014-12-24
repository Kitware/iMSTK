#include "smCore/smEvent.h"
#include "smCore/smEventHandler.h"

smEvent::smEvent()
{

    priority = SIMMEDTK_EVENTPRIORITY_NORMAL;
}

smEvent::~smEvent()
{

    delete []data;
}

smEventType::smEventType()
{

    eventTypeCode = SIMMEDTK_EVENTTYPE_NONE;
}

smEventType::smEventType(smInt p_eventType)
{

    eventTypeCode = p_eventType;
}

smEventType & smEventType::operator=(smInt p_eventTypeCode)
{

    eventTypeCode = p_eventTypeCode;
    return *this;
}

smBool smEventType::operator ==(smEventType &p_event)
{

    return (eventTypeCode == p_event.eventTypeCode ? true : false);
}

smBool smEventType::operator ==(smInt p_eventTypeCode)
{

    return (eventTypeCode == p_eventTypeCode ? true : false);
}
