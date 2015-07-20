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

#ifndef RENDER_CUBE_H
#define RENDER_CUBE_H

#include "Core/SDK.h"
#include "Devices/Audio.h"

class AudioKeyboardController : public CoreClass
{
public:
    /// \brief Default constructor
    ///
    AudioKeyboardController();

    /// \brief Default constructor
    ///
    /// \param a Pointer to sound to be controlled
    AudioKeyboardController(std::shared_ptr<smAudio> a);

    /// \brief Event handling function from CoreClass
    ///
    /// \param event Event to handle from the main event system
    void handleEvent(std::shared_ptr<mstk::Event::Event> event) override;

    /// \brief Set the sound to be controlled
    ///
    /// \param a Pointer to sound to be controlled
    void setSound(std::shared_ptr<smAudio> a);

private:
    std::shared_ptr<smAudio> sound; ///< Pointer to sound being controlled
    bool loopSound; ///< Flag to loop the sound or not
    float soundVolume;
};

void runAudioExample();

#endif
