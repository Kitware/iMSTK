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

#include "Core/Config.h"
#include "Core/CoreClass.h"
#include "Core/ErrorLog.h"
#include "Core/SDK.h"
#include "Event/AudioEvent.h"

/// \brief class to enable audio rendering
class smAudio: public smCoreClass
{
private:
    sf::Sound sound; ///< SFML Sound object, controls the sound data
    sf::SoundBuffer soundBuffer; ///< SFML Sound buffer, contains the sound data

    std::shared_ptr<smErrorLog> log; ///< log for errors
    std::string referenceName; ///< A human readable string to refer to the object
    mstk::Event::AudioState state; ///< state of audio
    mstk::Event::AudioState prevState; ///< state of audio in previous cycle
    float prevVolume; ///< state of audio volume in previous cycle
    float volume; ///< volume (max volume is 1.0)
    bool loop; ///< play the song in a loop

public:
    smAudio() :
        referenceName(""),
        state{mstk::Event::AudioState::Unknown },
        prevState{mstk::Event::AudioState::Unknown },
        volume{1.0},
        prevVolume{1.0},
        loop{false}
    {
    }

    /// \brief constructor initialize various states
    smAudio(const std::string& fileName,
            const std::string& p_referenceName,
            smErrorLog *p_log = nullptr,
            bool p_loop = false)
        : referenceName(p_referenceName),
          loop(p_loop)
    {
        if (0 != open(fileName, p_referenceName))
        {
            assert(false);
        }

        prevState = state = mstk::Event::AudioState::Stop;
        volume = prevVolume = 1.0f;
    }

    /// \brief destructor
    ~smAudio()
    {
    }

    int open(const std::string& fileName,
               const std::string& p_referenceName)
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
    void setState(mstk::Event::AudioState p_state)
    {
        assert("" != referenceName);

        switch (state)
        {
        case mstk::Event::AudioState::Play:
            this->play();
            break;
        case mstk::Event::AudioState::Stop:
            this->stop();
            break;
        default:
            return;
        }
        state = p_state;
        prevState = state;
    }

    /// \brief set volume of audio
    void setVolume(float p_volume)
    {
        if (p_volume > 0.0 && p_volume < 1.0)
        {
            volume = p_volume;
            sound.setVolume(volume*100);//SFML takes a range 0-100
        }
    }

    void setLoop(bool p_loop)
    {
        sound.setLoop(p_loop);
        this->loop = p_loop;
    }
};

#endif
