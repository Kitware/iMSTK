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

#include "imstkConsoleModule.h"
#include "imstkKeyboardDeviceClient.h"

namespace imstk
{
ConsoleModule::ConsoleModule(std::string name) :
    m_keyboardDeviceClient(KeyboardDeviceClient::New())
{
}

void
ConsoleModule::updateModule()
{
    const char c = static_cast<char>(getchar());

    // Simulate key press/release
    m_keyboardDeviceClient->emitKeyDown(c);
    m_keyboardDeviceClient->emitKeyUp(c);
}
} // namespace imstk