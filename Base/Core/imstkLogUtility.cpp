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
#include "imstkLogUtility.h"

namespace imstk
{

stdSink::FG_Color
stdSink::GetColor(const LEVELS level) const
{
    if (level.value == WARNING.value)   return YELLOW;
    if (level.value == DEBUG.value)   return GREEN;
    if (level.value == FATAL.value)   return RED;
    return WHITE;
}

void
stdSink::ReceiveLogMessage(g3::LogMessageMover logEntry)
{
    auto level = logEntry.get()._level;
    auto message = logEntry.get().message();

#ifndef WIN32
    auto color = GetColor(level);
    std::cout << "\033[" << color << "m"
        << message
        << "\033[m" << std::endl;
#else
    if (level.value == WARNING.value || level.value == FATAL.value)
    {
        std::cerr << message << std::endl;
    }
    else
    {
        std::cout << message << std::endl;
    }
#endif
}

void
LogUtility::createLogger(std::string name, std::string path)
{
    m_g3logWorker = g3::LogWorker::createLogWorker();
    m_fileSinkHandle = m_g3logWorker->addDefaultLogger(name, path);
    m_stdSinkHandle = m_g3logWorker->addSink(
        std2::make_unique<stdSink>(), &stdSink::ReceiveLogMessage);
        g3::initializeLogging(m_g3logWorker.get());
}

}