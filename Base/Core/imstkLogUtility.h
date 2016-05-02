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

#ifndef imstkLogUtility_h
#define imstkLogUtility_h

#include <string>
#include <iostream>

#include "g3log/logmessage.hpp"
#include "g3log/logworker.hpp"

namespace imstk {
struct stdSink {
    // Linux xterm color
    // http://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
    enum FG_Color { YELLOW = 33, RED = 31, GREEN = 32, WHITE = 97 };

    FG_Color GetColor(const LEVELS level) const;
    void ReceiveLogMessage(g3::LogMessageMover logEntry);
};

struct LogUtility {
    void createLogger(std::string name, std::string path);

    std::unique_ptr<g3::LogWorker>                m_g3logWorker;
    std::unique_ptr<g3::SinkHandle<g3::FileSink> >m_fileSinkHandle;
    std::unique_ptr<g3::SinkHandle<stdSink> >     m_stdSinkHandle;
};
}

#endif // ifndef imstkLogUtility_h
