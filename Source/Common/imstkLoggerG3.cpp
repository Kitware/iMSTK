/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkLoggerG3.h"

#include "g3log/crashhandler.hpp"

#include <iostream>
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
        return FontColor::Yellow;
    }
    if (level.value == DEBUG.value)
    {
        return FontColor::Green;
    }
    if (level.value == FATAL.value)
    {
        return FontColor::Red;
    }
    return FontColor::White;
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
    auto color = static_cast<int>(GetColor(level));
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

imstk::LoggerG3&
LoggerG3::getInstance()
{
    // Thread safe in C++11 ("magic statics")
    static LoggerG3 instance;
    if (instance.m_g3logWorker == nullptr)
    {
        instance.initialize();
    }

    return instance;
}

LoggerG3&
LoggerG3::startLogger()
{
    // As logger is a static object we fetch and destroy
    // the logger this will reset all sinks
    {
        LoggerG3& logger = LoggerG3::getInstance();
        logger.destroy();
    }

    LoggerG3& logger = LoggerG3::getInstance();

    logger.addFileSink("simulation");
    logger.addStdoutSink();
    return logger;
}

std::unique_ptr<StdoutSinkHandle>
LoggerG3::addStdoutSink()
{
    return std::unique_ptr<StdoutSinkHandle>(std::move(m_g3logWorker->addSink(std2::make_unique<stdSink>(), &stdSink::ReceiveLogMessage)));
}

std::unique_ptr<FileSinkHandle>
LoggerG3::addFileSink(const std::string& name, const std::string& path)
{
    return std::unique_ptr<FileSinkHandle>(m_g3logWorker->addDefaultLogger(name, path, "imstk"));
}

void
LoggerG3::initialize()
{
    m_g3logWorker = g3::LogWorker::createLogWorker();
    g3::initializeLogging(m_g3logWorker.get());
}

void
LoggerG3::destroy()
{
    m_g3logWorker = nullptr;
}

LoggerG3::LoggerG3()
{
    initialize();
}
} // namespace imstk