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
#include <cstring>

namespace imstk
{
Logger::Logger(std::string filename)
{
    m_filename = filename + "_" + this->getCurrentTimeFormatted() + ".log";
    m_mutex    = new std::mutex();
    m_thread   = new std::thread(Logger::eventLoop, this);
}

Logger::~Logger()
{
    delete m_mutex;
    m_thread->join();
    delete m_thread;
}

std::string
Logger::getCurrentTimeFormatted()
{
    time_t now   = time(0);
    int    year  = gmtime(&now)->tm_year + 1900;
    int    day   = gmtime(&now)->tm_mday;
    int    month = gmtime(&now)->tm_mon;
    int    hour  = gmtime(&now)->tm_hour;
    int    min   = gmtime(&now)->tm_min;
    int    sec   = gmtime(&now)->tm_sec;

    std::string year_string = std::to_string(year);
    std::string day_string  = std::to_string(day);
    if (day < 10)
    {
        day_string = "0" + day_string;
    }
    std::string month_string = std::to_string(month);
    if (month < 10)
    {
        month_string = "0" + month_string;
    }
    std::string hour_string = std::to_string(hour);
    if (hour < 10)
    {
        hour_string = "0" + hour_string;
    }
    std::string min_string = std::to_string(min);
    if (min < 10)
    {
        min_string = "0" + min_string;
    }
    std::string sec_string = std::to_string(sec);
    if (sec < 10)
    {
        sec_string = "0" + sec_string;
    }

    return year_string + day_string + month_string + "-" + hour_string + min_string + sec_string;
}

void
Logger::eventLoop(Logger* logger)
{
    std::ofstream file(logger->m_filename);
    std::string   buffer;

    while (logger->m_running) {
        std::unique_lock<std::mutex> ul(*logger->m_mutex);
        logger->m_condition.wait(ul, [logger] { return logger->m_changed; });

        if (!logger->m_running)
        {
            logger->m_changed = false;
            ul.unlock();
            break;
        }

        buffer = logger->m_message;
        logger->m_changed = false;

        ul.unlock();
        logger->m_condition.notify_one();

        file << buffer;
    }
    file.close();
    logger->m_condition.notify_one();
}

void
Logger::log(std::string message, bool prependTime /* = false */)
{
    m_message = "";
    if (prependTime)
    {
        m_message = this->getCurrentTimeFormatted() + " ";
    }
    m_message += message;

    // Safely setting the change state
    {
        std::lock_guard<std::mutex> guard(*m_mutex);
        m_changed = true;
    }

    m_condition.notify_one();
    std::unique_lock<std::mutex> ul(*m_mutex);
    m_condition.wait(ul, [this] { return !m_changed; });
    ul.unlock();
}

bool
Logger::readyForLoggingWithFrequency()
{
    long long currentMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count();
    if (currentMilliseconds - m_lastLogTime > m_period)
    {
        return true;
    }
    return false;
}

void
Logger::updateLogTime()
{
    m_lastLogTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock().now().time_since_epoch()).count();
}

void
Logger::setFrequency(int frequency)
{
    m_frequency = frequency;
    m_period    = 1000 / m_frequency;
}

int
Logger::getFrequency()
{
    return m_frequency;
}

void
Logger::shutdown()
{
    // Safely setting the running state
    {
        std::lock_guard<std::mutex> guard(*m_mutex);
        m_changed = true;
        m_running = false;
    }

    m_condition.notify_one();
    std::unique_lock<std::mutex> ul(*m_mutex);
    m_condition.wait(ul, [this] { return !m_changed; });
    ul.unlock();
}
}
