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

#include "g3log/logmessage.hpp"
#include "g3log/logworker.hpp"

#include "imstkLogger.h"

namespace imstk
{
///
/// \struct stdSink
///
/// \brief A standard sink that prints the message to a standard output
///
struct stdSink
{
    // Linux xterm color
    // http://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
    enum FontColor { Yellow = 33, Red = 31, Green = 32, White = 97 };

    FontColor GetColor(const LEVELS level) const;
    void ReceiveLogMessage(g3::LogMessageMover logEntry);
};

// Available through namespace
extern void Log(std::string name, std::string message);

///
/// \struct LogManager
///
/// \brief
///

struct LogManager
{
    std::map<std::string, Logger*> loggers;
    std::map<std::string, std::thread*> loggerThreads;
};

///
/// \struct LogUtility
///
/// \brief
///
class LogUtility
{
public:
    std::shared_ptr<g3::LogWorker> getLogWorker() { return m_g3logWorker; }

    void createLogger(std::string name, std::string path);

private:
    std::shared_ptr<g3::LogWorker> m_g3logWorker;
    std::unique_ptr<g3::SinkHandle<g3::FileSink>> m_fileSinkHandle;
    std::unique_ptr<g3::SinkHandle<stdSink>>      m_stdSinkHandle;
};
}
