// This file is part of the SimMedTK project.
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
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#ifndef SMTIMER_H
#define SMTIMER_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"

#include <chrono>
using namespace std::chrono;
/// \brief timer class
class smTimer: public smCoreClass
{
public:
    /// \brief constructor
    smTimer()
    {
        start();
    }
    /// \brief start the timer
    inline  void start()
    {
        begin = high_resolution_clock::now();
    }

    /// \brief Gets the time passed between this call and start()
    ///
    /// \return Returns the time in seconds
    inline smLongDouble elapsed()
    {
        high_resolution_clock::time_point now = high_resolution_clock::now();
        high_resolution_clock::duration delta = now - begin;
        return (smLongDouble) (((smLongDouble)delta.count() *
                                high_resolution_clock::period::num) /
                               high_resolution_clock::period::den);
    }

private:
    high_resolution_clock::time_point begin;
};

#endif
