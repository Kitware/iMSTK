/*
 * // This file is part of the SimMedTK project.
 * // Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 * //                        Rensselaer Polytechnic Institute
 * //
 * // Licensed under the Apache License, Version 2.0 (the "License");
 * // you may not use this file except in compliance with the License.
 * // You may obtain a copy of the License at
 * //
 * //     http://www.apache.org/licenses/LICENSE-2.0
 * //
 * // Unless required by applicable law or agreed to in writing, software
 * // distributed under the License is distributed on an "AS IS" BASIS,
 * // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * // See the License for the specific language governing permissions and
 * // limitations under the License.
 * //
 * //---------------------------------------------------------------------------
 * //
 * // Authors:
 * //
 * // Contact:
 * //---------------------------------------------------------------------------
 */

#include "smEvent/smAudioEvent.h"

namespace smtk
{
namespace Event
{

constexpr EventType smAudioEvent::EventName;

smAudioEvent::smAudioEvent(): state(AudioState::Stop), sound(""), volume(-1.0)
{
}
void smAudioEvent::setState(const smtk::Event::AudioState& audioState)
{
    this->state = audioState;
}
const smtk::Event::AudioState& smAudioEvent::getState()
{
    return this->state;
}
void smAudioEvent::setSound(const std::string& soundStream)
{
    this->sound = soundStream;
}
const std::string& smAudioEvent::getSound()
{
    return this->sound;
}
void smAudioEvent::setVolume(const float& volumeScale)
{
    this->volume = volumeScale;
}
const float& smAudioEvent::getVolume()
{
    return this->volume;
}
}
}

