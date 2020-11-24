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

#pragma once

#include "imstkEventObject.h"

#include <string>
#include <atomic>
#include <thread>
#include <vector>

namespace imstk
{
///
/// \brief Enumerations for the status of the thread
///
enum class ThreadStatus
{
    Running,
    Paused,
    Inactive
};

///
/// \class ThreadObject
///
/// \brief Wraps Thread with an object oriented base class suited for real time,
/// persistent threads ??
///
class ThreadObject : public EventObject
{
public:
    virtual ~ThreadObject() override;

protected:
    ///
    /// \brief Set status to launch into paused state
    /// \param: Name of the thread
    /// \param: Desired status when this starts
    ///
    ThreadObject(std::string name, ThreadStatus requestedStatus = ThreadStatus::Running);

public:
    ///
    /// \brief Get the status of the thread
    ///
    ThreadStatus getStatus() const { return m_status; }

    ///
    /// \brief Get the name of the thread
    ///
    const std::string& getName() const { return m_name; }

    ///
    /// \brief An agglomeration of the start/stop/resume/pause calls
    /// \param: Desired status
    /// \param: When true, returns after status has changed to desired. When false, returns immediately
    ///
    void requestStatus(ThreadStatus status);

public:
    ///
    /// \brief Start the thread and all its children
    /// children are started before parents
    /// \param sync: When true, returns after thread is complete. When false, returns immediately
    ///
    virtual void start(const bool sync = true);

    ///
    /// \brief End/terminate the thread
    /// \param sync: When true, returns after stop complete (like a thread join).
    /// When false, returns immediately
    ///
    virtual void stop(const bool sync = true);

    ///
    /// \brief Resume the thread
    /// \param sync: When true, returns after resume. When false, returns immediately
    ///
    virtual void resume(const bool sync = false);

    ///
    /// \brief Pause the thread
    /// \param sync: When true, returns after thread paused. When false, returns immediately
    ///
    virtual void pause(const bool sync = true);

    ///
    /// \brief Adds a child, this thread will start when *this* does
    /// If *this* is already running, it will start immediately
    /// If *this* is paused, it will start paused
    ///
    void addChildThread(std::shared_ptr<ThreadObject> childThreadObject);

    ///
    /// \brief Stops and then removes a child
    ///
    void removeChildThread(std::shared_ptr<ThreadObject> childThreadObject);

protected:
    ///
    /// \brief Called when the thread starts
    ///
    virtual void startThread() { }

    ///
    /// \brief Called when the thread stops
    ///
    virtual void stopThread() { }

    ///
    /// \brief Called before the thread resumes
    ///
    virtual void resumeThread() { }

    ///
    /// \brief Called before the thread pauses
    ///
    virtual void pauseThread() { }

protected:
    std::atomic<ThreadStatus> m_status{ ThreadStatus::Inactive };         ///> Thread status
    std::atomic<ThreadStatus> m_requestedStatus{ ThreadStatus::Running }; ///> Requested status

    std::vector<std::shared_ptr<ThreadObject>> m_children;                ///> Child threads

    std::string   m_name;                                                 ///> Name of the thread
    ThreadObject* m_parent;                                               ///> Parent thread, may only have one (or none)
    std::thread   m_stlThread;                                            ///> Used for the thread
};
}