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

#ifndef SMAUDIO_H
#define SMAUDIO_H

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smErrorLog.h"
#include "audiere.h"
#include "smCore/smSDK.h"
#include "smEvent/smAudioEvent.h"

using namespace audiere;

/// \brief contains audio query states
enum smAudioReturnType
{
    SIMMEDTK_AUDIO_PLAYING,
    SIMMEDTK_AUDIO_STOPPED,
    SIMMEDTK_AUDIO_SOUNDERROR,
    SIMMEDTK_AUDIO_SOUNDNOOP,
};

/// \brief class to enable audio rendering
class smAudio: public smCoreClass
{

    AudioDevicePtr device; ///< audio device
    OutputStreamPtr sound; ///< !!

    smErrorLog *log; ///< log for errors rendering audio
    smString referenceName; ///< !!
    smtk::Event::AudioState state; ///< state of audio
    smtk::Event::AudioState prevState; ///< state of audio in previous cycle
    smFloat prevVolume; ///< state of audio volume in previous cycle

    smFloat volume; ///< volume (max volume is 1.0)

public:
    smBool continuousPlaying; ///< true if audio is to be played continuously

    /// \brief constructor initialize various states
    smAudio(const smString& fileName,
            const smString& p_referenceName,
            smErrorLog *p_log = NULL)
    {
        device = AudioDevicePtr(OpenDevice());
        sound = OutputStreamPtr(OpenSound(device, fileName.c_str(), false));
        volume = 1.0f; //max volume
        prevVolume = 1.0f;
        log = p_log;
        continuousPlaying = true;
        referenceName = p_referenceName;
        prevState = state = smtk::Event::AudioState::Stop;
        this->eventHandler->registerEvent(smtk::Event::EventType::Audio,shared_from_this());
    }

    /// \brief destructor
    ~smAudio()
    {
        ///clean up process
        sound = 0;
        device = 0;
    }

    /// \brief play the audio
    smAudioReturnType play()
    {

        if (sound == NULL)
        {
            if (log != NULL)
            {
                log->addError("Error in playing sound.Sound object is NULL");
            }

            return SIMMEDTK_AUDIO_SOUNDERROR;
        }

        switch (state)
        {
            case smtk::Event::AudioState::Play:
            {
                if (sound->isPlaying())
                {
                    if (state ==  smtk::Event::AudioState::Play || state ==  smtk::Event::AudioState::PlayContinuous)
                    {
                        return SIMMEDTK_AUDIO_PLAYING;
                    }

                }
                else
                {
                    sound->setVolume(volume);
                    sound->play();
                    return SIMMEDTK_AUDIO_PLAYING;
                }
            }

            case smtk::Event::AudioState::Stop:
            {
                if (prevState != smtk::Event::AudioState::Stop)
                {
                    sound->stop();
                }
                return SIMMEDTK_AUDIO_STOPPED;
            }

            case smtk::Event::AudioState::PlayContinuous:
            {
                sound->setRepeat(continuousPlaying);
                return SIMMEDTK_AUDIO_PLAYING;
            }
        }

        return SIMMEDTK_AUDIO_SOUNDNOOP;
    }

    /// \brief stop the audio
    void stop()
    {
        if (sound != NULL)
        {
            if (sound->isPlaying())
            {
                sound->stop();
            }
        }
    }

    /// \brief set the state of audio and continue playing
    void setState(smtk::Event::AudioState p_state)
    {
        state = p_state;
        play();
        prevState = state;
    }

    /// \brief set volume of audio
    void setVolume(smFloat p_volume)
    {
        if (p_volume > 0.0 && p_volume < 1.0)
        {
            volume = p_volume;
        }
    }

    /// \brief gather input events and change states and volume if needed
    void handleEvent(std::shared_ptr<smtk::Event::smEvent> event) override
    {
        if(!this->isListening())
        {
            return;
        }

        auto audioEvent = std::static_pointer_cast<smtk::Event::smAudioEvent>(event);

        if (audioEvent != nullptr)
        {
            if (audioEvent->getSound() == referenceName)
            {
                setVolume(audioEvent->getVolume());
                setState(audioEvent->getState());
            }
        }
    }

};

#endif
