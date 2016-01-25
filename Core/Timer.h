// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef CORE_TIMER_H
#define CORE_TIMER_H

// STL includes
#include <chrono>

namespace imstk {

/// \brief timer class
class Timer
{
public:
    using ClockType = std::chrono::high_resolution_clock;
    using TimePointType = ClockType::time_point;
    using DurationType = ClockType::duration;
    using PeriodType = ClockType::period;

    ///
    /// \brief Constructor
    ///
    Timer();

    ///
    /// \brief Start the timer
    ///
    void start();

    ///
    /// \brief Gets the time passed between start() and this call
    ///
    /// \return Returns the time in seconds
    ///
    long double elapsed();

private:
    TimePointType begin; //!< Beginning of time point
};

} //core

#endif
