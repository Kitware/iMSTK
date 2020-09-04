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

#include "imstkThreadObject.h"

namespace imstk
{
class UPSCounter;

///
/// \class LoopThreadObject
///
/// \brief A thread object that defines its own basic loop and update function
/// Subclasses can define how to update
///
class LoopThreadObject : public ThreadObject
{
public:
    LoopThreadObject(std::string name, int loopDelay = 0);
    virtual ~LoopThreadObject() = default;

public:
    ///
    /// \brief Get loop delay
    ///
    double getLoopDelay() const { return m_loopDelay; }

    ///
    /// \brief Set the loop delay
    ///
    void setLoopDelay(const double milliseconds);

    ///
    /// \brief Get loop delay
    ///
    double getFrequency() const;

    ///
    /// \brief Set the loop delay
    ///
    void setFrequency(const double f);

    ///
    /// \brief Enable frame count
    ///
    void enableFrameCount() { m_trackFPS = true; };
    ///
    /// \brief Disable frame count
    ///
    void disableFrameCount() { m_trackFPS = false; };

    /// \return true is frame count is already enabled
    bool isFrameCountEnabled() const { return m_trackFPS; };

    unsigned int getUPS() const;

public:
    void update()
    {
        emit(Event(EventType::PostUpdate));
        updateThread();
        emit(Event(EventType::PreUpdate));
    }

protected:
    ///
    /// \brief Start the thread
    ///
    virtual void startThread() override;

    ///
    /// \brief Called right before thread is started
    ///
    virtual void initThread() { }

    ///
    /// \brief Update the thread
    ///
    virtual void updateThread() = 0;

protected:
    std::shared_ptr<UPSCounter> m_frameCounter;
    double m_loopDelay;      ///> Loop delay
    bool   m_trackFPS;       ///> Flag to enable/diable FPS tracking
};
}