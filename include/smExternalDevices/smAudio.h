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

#include <SFML/Audio.hpp>

#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"
#include "smCore/smErrorLog.h"
#include "smCore/smSDK.h"


/// \brief contains state of the audio
enum smAudioState
{
    SIMMEDTK_AUDIOSTATE_UNKNOWN = -1,
    SIMMEDTK_AUDIOSTATE_PLAY = 0,
    SIMMEDTK_AUDIOSTATE_STOP
};

/// \brief contains data for audio rendering
struct smAudioEventData
{
    smString sound;
    smAudioState state;
    smFloat volume;

    smAudioEventData()
    {
        volume = -1;
    }
};

/// \brief class to enable audio rendering
class smAudio: public smCoreClass, public smEventHandler
{
private:
    sf::Sound sound; ///< SFML Sound object, controls the sound data
    sf::SoundBuffer soundBuffer; ///< SFML Sound buffer, contains the sound data

    smErrorLog *log; ///< log for errors rendering audio
    smString referenceName; ///< string used to identify the sound object
    smAudioState state; ///< state of audio
    smAudioState prevState; ///< state of audio in previous cycle
    smFloat prevVolume; ///< state of audio volume in previous cycle
    smFloat volume; ///< volume (max volume is 1.0)
    smBool loop; ///< play the song in a loop

public:
    smAudio() :
        log{nullptr},
        referenceName{""},
        state{SIMMEDTK_AUDIOSTATE_UNKNOWN},
        prevState{SIMMEDTK_AUDIOSTATE_UNKNOWN},
        volume{1.0},
        prevVolume{1.0},
        loop{false}
    {
    }

    /// \brief constructor initialize various states
    smAudio(const smString& fileName,
            const smString& p_referenceName,
            smErrorLog *p_log = nullptr,
            smBool p_loop = false)
    {
        log = p_log;
        if (0 != open(fileName, p_referenceName))
        {
            assert(false);
        }
        else
        {
            volume = prevVolume = 1.0f;
            prevState = state = SIMMEDTK_AUDIOSTATE_STOP;
            loop = p_loop;
            smSDK::getInstance()->getEventDispatcher()->registerEventHandler(this, SIMMEDTK_EVENTTYPE_AUDIO);
        }
    }

    /// \brief destructor
    ~smAudio()
    {
    }

    smInt open(const smString& fileName,
               const smString& p_referenceName)
    {
        if (false == soundBuffer.loadFromFile(fileName))
        {
            if (nullptr != this->log)
            {
                this->log->addError("Error loading audio file: " + fileName);
            }
            return -1;
        }
        else
        {
            sound.setBuffer(soundBuffer);
            referenceName = p_referenceName;
        }
        return 0;
    }

    /// \brief play the audio
    void play()
    {
        assert("" != referenceName);
        sound.play();
    }

    void pause()
    {
        assert("" != referenceName);
        sound.pause();
    }

    /// \brief stop the audio
    void stop()
    {
        assert("" != referenceName);
        sound.stop();
    }

    /// \brief set the state of audio and continue playing
    void setState(smAudioState p_state)
    {
        assert("" != referenceName);

        switch (state)
        {
        case SIMMEDTK_AUDIOSTATE_PLAY:
            this->play();
            break;
        case SIMMEDTK_AUDIOSTATE_STOP:
            this->stop();
            break;
        default:
            return;
        }
        state = p_state;
        prevState = state;
    }

    /// \brief set volume of audio
    void setVolume(smFloat p_volume)
    {
        if (p_volume > 0.0 && p_volume < 1.0)
        {
            volume = p_volume;
            sound.setVolume(volume*100);//SFML takes a range 0-100
        }
    }

    void setLoop(smBool p_loop)
    {
        sound.setLoop(p_loop);
        this->loop = p_loop;
    }

    /// \brief gather input events and change states and volume if needed
    void handleEvent(smEvent *p_event)
    {
        smAudioEventData *audioEvent;

        if (p_event->eventType == SIMMEDTK_EVENTTYPE_AUDIO)
        {
            audioEvent = (smAudioEventData*)p_event->data;

            if (audioEvent->sound == referenceName)
            {
                setVolume(audioEvent->volume);
                setState(audioEvent->state);
            }
        }
    }

};

#endif
