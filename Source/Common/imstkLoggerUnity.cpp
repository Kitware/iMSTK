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

#include "imstkLoggerUnity.h"

#include <ctime>
#include <iomanip>

#if defined(_WIN32)
#include <windows.h>
#else // not defined(_WIN32)
#include <stdlib.h>
#endif // not defined(_WIN32)
#include <unordered_map>

namespace imstk
{
StreamOutput::StreamOutput(std::ostream& ostream) : m_stream(ostream)
{
}

bool
StreamOutput::writeMessage(const std::string& message)
{
    bool result = false;
    if (!m_stream.fail())
    {
        m_mutex.lock();
        m_stream << message << std::endl;
        result = true;
        m_mutex.unlock();
    }
    else
    {
        throw("Default logging not implemented yet");
    }
    return result;
}

bool
CacheOutput::writeMessage(const std::string& message)
{
    m_mutex.lock();
    m_messages.push_back(message);
    m_mutex.unlock();
    return true;
}

bool
CacheOutput::hasMessages() const
{
    bool result;
    m_mutex.lock();
    result = m_messages.empty();
    m_mutex.unlock();
    return result;
}

std::string
CacheOutput::popLastMessage()
{
    std::string result;
    m_mutex.lock();
    if (!m_messages.empty())
    {
        result = m_messages.front();
        m_messages.pop_front();
    }
    m_mutex.unlock();
    return result;
}

LogMessageBase::LogMessageBase(LoggerUnity* logger, int level)
    : m_stream(), m_logger(logger)
{
    CHECK(logger) << "logger should not be a null pointer";
    static std::unordered_map<int, std::string> levelNames = {
        { DEBUG.value, "DEBUG   " },
        { INFO.value, "INFO    " },
        { WARNING.value, "WARNING " },
        { FATAL.value, "FATAL   " }
    };
    std::time_t                                 timeStamp;
    std::time(&timeStamp);
    ::tm tm;
#ifdef _MSC_VER
    localtime_s(&tm, &timeStamp);
#else
    localtime_r(&timeStamp, &tm);
#endif
    std::string levelName("NONE    ");
    if (level >= 0 && level <= FATAL.value)
    {
        levelName = levelNames[level];
    }
    char fillChar = m_stream.fill();
    m_stream << std::setfill('0') <<
        std::setw(2) << 1 + tm.tm_mon << "." <<
        std::setw(2) << tm.tm_mday << ' ' <<
        std::setw(2) << tm.tm_hour << ':' <<
        std::setw(2) << tm.tm_min << ':' <<
        std::setw(2) << tm.tm_sec << ' ' <<
        std::setfill(fillChar) <<
        levelName << " " <<
        m_logger->getName() << " ";
}

void
AssertMessage::setFailureCallback(AssertMessage::DeathCallback callback)
{
    m_killMeNow = callback;
}

AssertMessage::DeathCallback
AssertMessage::getFailureCallback()
{
    return m_killMeNow;
}

void
AssertMessage::throwException(const std::string& errorMessage)
{
    //throw AssertionFailure(errorMessage);
    throw std::runtime_error(errorMessage);
}

void
AssertMessage::killApplication(const std::string&)
{
#if defined(_WIN32)
    DebugBreak();
#else  // not defined(_WIN32)
    abort();
#endif // not defined(_WIN32)
}

AssertMessage::DeathCallback AssertMessage::m_killMeNow = AssertMessage::throwException;
} // namespace imstk