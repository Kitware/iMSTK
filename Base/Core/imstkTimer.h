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

#ifndef imstkTimerUtility_h
#define imstkTimerUtility_h

#include "g3log/g3log.hpp"

#include <chrono>
#include <ctime>
#include <string>
#include <vector>

namespace imstk
{

///
/// \brief Stop Watch utility class
///
class StopWatch
{
public:
    static const double wcTimerConstants[4];

    enum class TimerState
    {
        started,
        stopped
    };

    enum class TimeUnitType
    {
        milliSeconds,
        seconds,
        minutes,
        hours
    };

    ///
    /// \brief Constructor
    ///
    StopWatch(){};

    ///
    /// \brief Destructor
    ///
    ~StopWatch() = default;

    ///
    /// \brief Start the appropriate timer
    ///
    virtual void start();

    ///
    /// \brief Start the appropriate timer
    ///
    virtual void stop();

    ///
    /// \brief Start the appropriate timer
    ///
    void storeLap(std::string const& lapName);
    void storeLap();

    ///
    /// \brief Print all the lap times
    ///
    void printLapTimes();

    ///
    /// \brief Clears all the laps
    ///
    virtual void reset();

    ///
    /// \brief Returns the total time from all
    ///
    double getTotalLapTimes();

    ///
    /// \brief Returns the time elapsed since calling start
    ///
    virtual double getTimeElapsed(const TimeUnitType unitType = TimeUnitType::milliSeconds);

    ///
    /// \brief Returns a string with current date and time in UTC
    ///
    static std::string getTimeAndDate();

    ///
    /// \brief Print the elapsed time
    ///
    void printTimeElapsed(std::string const& name = std::string("noName"), const TimeUnitType unitType = TimeUnitType::milliSeconds);
private:
    TimerState state;
    std::vector<double> lapTimes;
    std::vector<std::string> lapNames;
    std::chrono::high_resolution_clock::time_point wallClockTimeKeeper; ///> time keeper for wall clock time
};

///
/// \brief CPU timer
///
class CpuTimer : public StopWatch
{
public:
    static const double cpuTimerConstants[4];

    ///
    /// \brief Constructor
    ///
    CpuTimer() : StopWatch(){};

    ///
    /// \brief Destructor
    ///
    ~CpuTimer() = default;

    ///
    /// \brief Start the appropriate timer
    ///
    virtual void start() override {cpuTimeKeeper = std::clock();};

    ///
    /// \brief Returns the CPU time elapsed since calling start
    /// NOTE: This measurement is not entirely accurate especially if one tries to measure
    /// CPU time used by certain calls in a multi-threaded application.
    ///
    double getTimeElapsed(const TimeUnitType unitType = TimeUnitType::milliSeconds) override;

private:
    std::clock_t cpuTimeKeeper; ///> time keeper for cpu time
};

}

#endif // ifndef imstkLogUtility_h
