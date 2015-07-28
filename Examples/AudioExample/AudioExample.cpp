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

#include "AudioExample.h"
#include "Core/SDK.h"


AudioKeyboardController::AudioKeyboardController()
    : loopSound{false},
      soundVolume{100}
{
}

AudioKeyboardController::AudioKeyboardController(std::shared_ptr<Audio> a)
    : loopSound{false},
      soundVolume{100},
      sound(a)
{
}

void AudioKeyboardController::setSound(std::shared_ptr<Audio> a)
{
    sound = a;
}

void AudioKeyboardController::handleEvent(std::shared_ptr<core::Event> event)
{
    assert(sound);

    auto keyboardEvent = std::static_pointer_cast<event::KeyboardEvent>(event);

    if(keyboardEvent->getPressed())
    {
        switch(keyboardEvent->getKeyPressed())
        {
        case event::Key::S:
            sound->play();
            break;
        case event::Key::P:
            sound->pause();
            break;
        case event::Key::H:
            sound->stop();
            break;
        case event::Key::L:
            loopSound = !loopSound;
            sound->setLoop(loopSound);
            break;
        case event::Key::I:
            if (1.0 > soundVolume)
            {
                soundVolume += 0.1;
                sound->setVolume(soundVolume);
            }
            break;
        case event::Key::D:
            if (0.0 <= soundVolume)
            {
                soundVolume -= 0.1;
                sound->setVolume(soundVolume);
            }
            break;
        default:
            break;
        }
    }
}

void runAudioExample()
{
    std::shared_ptr<SDK> sdk;
    std::shared_ptr<OpenGLViewer> viewer;
    std::shared_ptr<Audio> sound;
    std::shared_ptr<AudioKeyboardController> audioCtl;

    //Create an instance of the SimMedTK framework/SDK
    sdk = SDK::getInstance();

    //Create a viewer to see the scene through
    viewer = std::make_shared<OpenGLViewer>();
    sdk->addViewer(viewer);

    //Create the audio controller
    audioCtl = std::make_shared<AudioKeyboardController>();

    sound = std::make_shared<Audio>();
    sound->open("Sounds/ping.ogg", "PingSound");

    audioCtl->setSound(sound);

    //Setup the window title in the window manager
    viewer->setWindowTitle("SimMedTK Audio Example");

    //Set some viewer properties
    viewer->setScreenResolution(800, 640);

    //Uncomment the following line for fullscreen
    //viewer->viewerRenderDetail |= SIMMEDTK_VIEWERRENDER_FULLSCREEN;

    //Link up the event system between this the audio controller and the viewer
    viewer->attachEvent(core::EventType::Keyboard, audioCtl);

    sdk->run();

    return;
}
