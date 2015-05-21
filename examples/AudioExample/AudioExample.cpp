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
#include "smCore/smSDK.h"


AudioKeyboardController::AudioKeyboardController()
    : loopSound{false},
      soundVolume{100}
{
}

AudioKeyboardController::AudioKeyboardController(std::shared_ptr<smAudio> a)
    : loopSound{false},
      soundVolume{100},
      sound(a)
{
}

void AudioKeyboardController::setSound(std::shared_ptr<smAudio> a)
{
    sound = a;
}

void AudioKeyboardController::handleEvent(std::shared_ptr<smtk::Event::smEvent> event)
{
    assert(sound);

    auto keyboardEvent = std::static_pointer_cast<smtk::Event::smKeyboardEvent>(event);

    if(keyboardEvent->getPressed())
    {
        switch(keyboardEvent->getKeyPressed())
        {
        case smtk::Event::smKey::S:
            sound->play();
            break;
        case smtk::Event::smKey::P:
            sound->pause();
            break;
        case smtk::Event::smKey::H:
            sound->stop();
            break;
        case smtk::Event::smKey::L:
            loopSound = !loopSound;
            sound->setLoop(loopSound);
            break;
        case smtk::Event::smKey::I:
            if (1.0 > soundVolume)
            {
                soundVolume += 0.1;
                sound->setVolume(soundVolume);
            }
            break;
        case smtk::Event::smKey::D:
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
    std::shared_ptr<smSDK> sdk;
    std::shared_ptr<smViewer> viewer;
    std::shared_ptr<smAudio> sound;
    std::shared_ptr<AudioKeyboardController> audioCtl;

    //Create an instance of the SimMedTK framework/SDK
    sdk = smSDK::getInstance();

    //Create a viewer to see the scene through
    viewer = std::make_shared<smViewer>();
    sdk->addViewer(viewer);

    //Create the audio controller
    audioCtl = std::make_shared<AudioKeyboardController>();

    sound = std::make_shared<smAudio>();
    sound->open("Sounds/ping.ogg", "PingSound");

    audioCtl->setSound(sound);

    //Setup the window title in the window manager
    viewer->setWindowTitle("SimMedTK Audio Example");

    //Set some viewer properties
    viewer->setScreenResolution(800, 640);

    //Uncomment the following line for fullscreen
    //viewer->viewerRenderDetail |= SIMMEDTK_VIEWERRENDER_FULLSCREEN;

    //Link up the event system between this the audio controller and the viewer
    viewer->attachEvent(smtk::Event::EventType::Keyboard, audioCtl);

    sdk->run();

    return;
}
