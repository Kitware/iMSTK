// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "smCore/smEvent.h"
#include "smCore/smEventHandler.h"

smEvent::smEvent()
{
    priority = SIMMEDTK_EVENTPRIORITY_NORMAL;
    data = nullptr;
}

smEvent::~smEvent()
{
    if (data != nullptr)
    {
        delete data;
    }
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
