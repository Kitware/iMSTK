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
#include "imstkLogger.h"

#ifdef WIN32
#include <windows.h>
#endif

namespace imstk
{
stdSink::FontColor
stdSink::GetColor(const LEVELS level) const
{
    if (level.value == WARNING.value)
    {
        return Yellow;
    }
    if (level.value == DEBUG.value)
    {
        return Green;
    }
    if (level.value == FATAL.value)
    {
        return Red;
    }
    return White;
}

#ifdef WIN32

#define WIN_CONSOLE_RED 4
#define WIN_CONSOLE_LIGHT_GRAY 7
#define WIN_CONSOLE_YELLOW 14

void
setColorWin(const int colCode)
{
    WORD wColor = ((0 & 0x0F) << 4) + (colCode & 0x0F); // black background
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wColor);
}

#endif

void
stdSink::ReceiveLogMessage(g3::LogMessageMover logEntry)
{
    const auto level   = logEntry.get()._level;
    const auto message = logEntry.get().message();

#ifndef WIN32
    auto color = GetColor(level);
    std::cout << "\033[" << color << "m"
              << message
              << "\033[m" << std::endl;
#else

    if (level.value == WARNING.value)
    {
        setColorWin(WIN_CONSOLE_YELLOW);
        std::cerr << message << std::endl;
        setColorWin(WIN_CONSOLE_LIGHT_GRAY);
    }
    else if (level.value == FATAL.value)
    {
        setColorWin(WIN_CONSOLE_RED);
        std::cerr << message << std::endl;
        setColorWin(WIN_CONSOLE_LIGHT_GRAY);
    }
    else
    {
        std::cout << message << std::endl;
    }
#endif
}

std::unique_ptr<StdoutSinkHandle>
Logger::addStdoutSink()
{
    return std::unique_ptr<StdoutSinkHandle>(std::move(m_g3logWorker->addSink(std2::make_unique<stdSink>(), &stdSink::ReceiveLogMessage)));
}

std::unique_ptr<FileSinkHandle>
Logger::addFileSink(const std::string& name, const std::string& path)
{
    return std::unique_ptr<FileSinkHandle>(m_g3logWorker->addDefaultLogger(name, path, "imstk"));
}

void
Logger::initialize()
{
    m_g3logWorker = g3::LogWorker::createLogWorker();
    g3::initializeLogging(m_g3logWorker.get());
}
}