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

#ifndef SMAUDIOEVENT_H
#define SMAUDIOEVENT_H

// STL includes
#include <string>

// SimMedTK includes
#include "Core/Event.h"

namespace event {

class AudioEvent: public core::Event
{
public:
    /// \brief contains state of the audio driver
    enum class AudioState
    {
        Unknown = -1,
        Play = 0,
        Stop,
    };
    static core::EventType EventName;

public:
    AudioEvent();

    void setState(const AudioState &audioState);

    const AudioState &getState();

    void setSound(const std::string &soundStream);

    const std::string &getSound();

    void setVolume(const float &volumeScale);

    const float &getVolume();

private:
    AudioState state;
    std::string sound;
    float volume;
};

} // event namespace

#endif // SMAUDIOEVENT_H
