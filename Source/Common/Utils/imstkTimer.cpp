/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkTimer.h"
#include "imstkLogger.h"

namespace imstk
{
const double
StopWatch::wcTimerConstants[] = {
    1.0,
    1 / 1000.0,
    1 / (1000.0 * 60.0),
    1 / (1000.0 * 60.0 * 60)
};

const double
CpuTimer::cpuTimerConstants[] = {
    1000.0 / CLOCKS_PER_SEC,
    1.0 / CLOCKS_PER_SEC,
    1.0 / (CLOCKS_PER_SEC * 60.0),
    1.0 / (CLOCKS_PER_SEC * 60.0 * 60.0)
};

void
StopWatch::start()
{
    wallClockTimeKeeper = std::chrono::high_resolution_clock::now();
    this->state = TimerState::started;
}

void
StopWatch::stop()
{
    this->storeLap();
    this->state = TimerState::stopped;
}

void
StopWatch::reset()
{
    lapTimes.clear();
    this->state = TimerState::stopped;
}

double
StopWatch::getTotalLapTimes()
{
    double totalTime = 0.0;
    for (const auto& timeChunk : lapTimes)
    {
        totalTime += timeChunk;
    }
    return totalTime;
}

void
StopWatch::storeLap(std::string const& lapName)
{
    if (this->lapTimes.size() > 0)
    {
        this->lapTimes.push_back(this->getTimeElapsed() - *this->lapTimes.end());
    }
    else
    {
        this->lapTimes.push_back(this->getTimeElapsed());
    }

    if (lapName == "noName")
    {
        this->lapNames.push_back("Lap " + std::to_string(lapNames.size()));
    }
    else
    {
        this->lapNames.push_back(lapName);
    }

    this->state = TimerState::stopped;
}

void
StopWatch::storeLap()
{
    this->storeLap(std::string("noName"));
}

void
StopWatch::printLapTimes()
{
    LOG(INFO) << "Lap times:";
    for (size_t i = 0; i < this->lapTimes.size(); ++i)
    {
        LOG(INFO) << "\t" << this->lapNames.at(i) << ": " << this->lapTimes.at(i) << "ms";
    }
}

std::string
StopWatch::getTimeAndDate()
{
    time_t now = time(0);
    return std::string(asctime(gmtime(&now)));
}

void
StopWatch::printTimeElapsed(std::string const& name /* = std::string("noName")*/,
                            const TimeUnitType unitType /*= TimeUnitType::milliSeconds*/)
{
    switch (unitType)
    {
    case TimeUnitType::milliSeconds:
        LOG(INFO) << name << ": " << this->getTimeElapsed(unitType) << " ms.";
        break;

    case TimeUnitType::seconds:
        LOG(INFO) << name << ": " << this->getTimeElapsed(unitType) << " sec.";
        break;

    case TimeUnitType::minutes:
        LOG(INFO) << name << ": " << this->getTimeElapsed(unitType) << " min.";
        break;

    case TimeUnitType::hours:
        LOG(INFO) << name << ": " << this->getTimeElapsed(unitType) << " hrs.";
        break;

    default:
        LOG(WARNING) << "Type of the time integrator not identified!";
        break;
    }
}

double
StopWatch::getTimeElapsed(const TimeUnitType unitType /*= TimeUnitType::milliSeconds*/)
{
    return std::chrono::duration<double, std::milli>
           (std::chrono::high_resolution_clock::now() - wallClockTimeKeeper).count() *
           wcTimerConstants[(int)unitType];
}

double
CpuTimer::getTimeElapsed(const TimeUnitType unitType /*= TimeUnitType::milliSeconds*/)
{
    return (std::clock() - cpuTimeKeeper) * cpuTimerConstants[(int)unitType];
}

void
UPSCounter::reset()
{
    m_timer->reset();
    m_accumulatedTimer = 0.;
    m_ups = 0;
    m_updateCount = 0;
}

void
UPSCounter::setEndPointOfUpdate()
{
    m_accumulatedTimer += m_timer->getTimeElapsed(StopWatch::TimeUnitType::milliSeconds);
    m_updateCount++;

    if (m_accumulatedTimer > 1000.)
    {
        m_ups = m_updateCount;
        m_updateCount      = 0;
        m_accumulatedTimer = 0.;
    }
}
} // namespace imstk
