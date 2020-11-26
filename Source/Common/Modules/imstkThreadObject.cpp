/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkThreadObject.h"
#include "imstkLogger.h"

namespace imstk
{
ThreadObject::ThreadObject(std::string name, ThreadStatus status) :
    m_requestedStatus(status), m_name(name), m_parent(nullptr)
{
}

ThreadObject::~ThreadObject()
{
    // Stop if not yet
    if (m_status != ThreadStatus::Inactive)
    {
        stop(true);
    }
}

void
ThreadObject::requestStatus(ThreadStatus status)
{
    // Copy the atomic
    if (status == ThreadStatus::Inactive)
    {
        m_requestedStatus = ThreadStatus::Inactive;
    }
    else if (status == ThreadStatus::Paused)
    {
        m_requestedStatus = ThreadStatus::Paused;
    }
    else if (status == ThreadStatus::Running)
    {
        m_requestedStatus = ThreadStatus::Running;
    }
    // Children recieve their parents requests of status
    for (size_t i = 0; i < m_children.size(); i++)
    {
        m_children[i]->requestStatus(status);
    }
}

void
ThreadObject::start(const bool sync)
{
    if (m_status != ThreadStatus::Inactive)
    {
        LOG(WARNING) << "Can not start '" << m_name << "'.\n"
                     << "Thread already/still active.";
        return;
    }

    this->postEvent(Event(EventType::Start, 5));

    m_status = ThreadStatus::Running;

    // Start all of it's children first (does not wait)
    for (size_t i = 0; i < m_children.size(); i++)
    {
        //m_children[i]->requestStatus(m_status);
        m_children[i]->start(false);
    }

    if (sync)
    {
        startThread();

        m_status = ThreadStatus::Inactive;

        // Stop all of it's children
        for (size_t i = 0; i < m_children.size(); i++)
        {
            m_children[i]->stop(true);
        }
        this->postEvent(Event(EventType::End, 5));
        m_requestedStatus = ThreadStatus::Running;
    }
    else
    {
        m_stlThread = std::thread(
            [&]()
            {
                startThread();

                m_status = ThreadStatus::Inactive;

                // Stop all of it's children
                for (size_t i = 0; i < m_children.size(); i++)
                {
                    m_children[i]->stop(true);
                }
                this->postEvent(Event(EventType::End, 5));
                m_requestedStatus = ThreadStatus::Running;
            });
    }
}

void
ThreadObject::stop(const bool sync)
{
    // Stop all of it's children first
    for (size_t i = 0; i < m_children.size(); i++)
    {
        m_children[i]->stop(true);
    }

    this->postEvent(Event(EventType::End, 5));

    stopThread();
    if ((m_status == ThreadStatus::Inactive))
    {
        LOG(WARNING) << "Can not end '" << m_name << "'.\n" << "Thread already inactive.";
        return;
    }

    m_requestedStatus = ThreadStatus::Inactive;

    // If sync, wait until the module actually stops before returning from function
    if (sync)
    {
        while (m_status != ThreadStatus::Inactive) {}

        if (m_stlThread.joinable())
        {
            m_stlThread.join();
        }
    }
}

void
ThreadObject::resume(const bool sync)
{
    if (m_status == ThreadStatus::Inactive)
    {
        return;
    }

    this->postEvent(Event(EventType::Resume, 5));

    resumeThread();
    if (m_status == ThreadStatus::Paused)
    {
        m_requestedStatus = ThreadStatus::Running;

        // If sync, wait until the module actually pauses before returning from function
        if (sync)
        {
            while (m_status != ThreadStatus::Running) {}
        }
    }
}

void
ThreadObject::pause(const bool sync)
{
    if (m_status == ThreadStatus::Inactive)
    {
        return;
    }

    this->postEvent(Event(EventType::Pause, 5));

    pauseThread();
    if (m_status == ThreadStatus::Running)
    {
        m_requestedStatus = ThreadStatus::Paused;

        // If sync, wait until the module actually pauses before returning from function
        if (sync)
        {
            while (m_status != ThreadStatus::Paused) {}
        }
    }
}

void
ThreadObject::addChildThread(std::shared_ptr<ThreadObject> childThreadObject)
{
    // If the module belongs to another parent
    if (childThreadObject->m_parent != nullptr)
    {
        // Remove child, which should sync stop it
        childThreadObject->m_parent->removeChildThread(childThreadObject);
    }

    m_children.push_back(childThreadObject);
    childThreadObject->m_parent = this;

    // Request the child have the parents status
    // This may start it, pause it, start in pause, etc
    //childThreadObject->requestStatus(m_status);
}

void
ThreadObject::removeChildThread(std::shared_ptr<ThreadObject> childThreadObject)
{
    auto i = std::find(m_children.begin(), m_children.end(), childThreadObject);
    if (i != m_children.end())
    {
        // Stop in sync
        (*i)->stop(true);
        m_children.erase(i);
        childThreadObject->m_parent = nullptr;
    }
}
}
