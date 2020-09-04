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

#include "imstkLoopThreadObject.h"

namespace imstk
{
class KeyboardDeviceClient;

///
/// \class ConsoleThread
/// 
/// \brief The console thread can run separately or in
/// sync to provide keyboard events from the console
/// they should be handled on another thread
///
class ConsoleThread : public LoopThreadObject
{
public:
    ConsoleThread(std::string name = "ConsoleThread");
    ~ConsoleThread() override = default;

public:
    std::shared_ptr<KeyboardDeviceClient> getKeyboardDevice() const { return m_keyboardDeviceClient; }

protected:
    ///
    /// \brief Awaits input
    /// 
    void updateThread() override;

protected:
    std::shared_ptr<KeyboardDeviceClient> m_keyboardDeviceClient;
};
}