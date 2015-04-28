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
#include "smCore/smTextureManager.h"

/// \brief A simple example of how to render an object using SimMedTK
///
/// \detail This is the default constructor, however, this is where the main
/// program runs.  This program will create a cube with a texture pattern
/// numbering each side of the cube, that's all it does.
AudioExample::AudioExample()
    : loopSound{false}, soundVolume{1.0}
{
    //Create an instance of the SimMedTK framework/SDK
    simmedtkSDK = smSDK::createSDK();

    //Create a viewer to see the scene through
    simmedtkSDK->addViewer(&viewer);

    sound.open("Sounds/ping.ogg", "PingSound");

    //Setup the window title in the window manager
    viewer.setWindowTitle("SimMedTK Audio Example");

    //Add the RenderExample object we are in to the viewer from the SimMedTK SDK
    viewer.addObject(this);

    //Set some viewer properties
    viewer.setScreenResolution(800, 640);

    //Uncomment the following line for fullscreen
    //viewer.viewerRenderDetail |= SIMMEDTK_VIEWERRENDER_FULLSCREEN;

    //Link up the event system between this object and the SimMedTK SDK
    simmedtkSDK->getEventDispatcher()->registerEventHandler(this, SIMMEDTK_EVENTTYPE_KEYBOARD);
}

AudioExample::~AudioExample()
{
}

void AudioExample::handleEvent(smEvent *p_event)
{
    switch (p_event->eventType.eventTypeCode)
    {
    case SIMMEDTK_EVENTTYPE_KEYBOARD:
    {
        smKeyboardEventData* kbData =
            (smKeyboardEventData*)p_event->data;
        smKey key = kbData->keyBoardKey;
        if (key == smKey::Escape && kbData->pressed)
        {
            //Tell the framework to shutdown
            simmedtkSDK->shutDown();
        }
        else if (key == smKey::S && kbData->pressed)
        {
            sound.play();
        }
        else if (key == smKey::P && kbData->pressed)
        {
            sound.pause();
        }
        else if (key == smKey::H && kbData->pressed)
        {
            sound.stop();
        }
        else if (key == smKey::L && kbData->pressed)
        {
            loopSound = !loopSound;
            sound.setLoop(loopSound);
        }
        else if (key == smKey::I && kbData->pressed)
        {
            if (1.0 > soundVolume)
            {
                soundVolume += 0.1;
                sound.setVolume(soundVolume);
            }
        }
        else if (key == smKey::D && kbData->pressed)
        {
            if (0.0 <= soundVolume)
            {
                soundVolume -= 0.1;
                sound.setVolume(soundVolume);
            }
        }
        break;
    }
    default:
        break;
    }
}

void AudioExample::simulateMain(smSimulationMainParam p_param)
{
    std::cout << "Usage:\n"
              << "Start sound: \'s\' key\n"
              << "Pause sound: \'p\' key\n"
              << "Halt sound: \'h\' key\n"
              << "Toggle Loop sound: \'l\' key\n"
              << "Increase volume: \'i\' key\n"
              << "Decrease volume: \'d\' key\n";
    //Run the simulator framework
    simmedtkSDK->run();
}

void runAudioExample()
{
    smSimulationMainParam simulationParams;
    AudioExample ae;

    ae.simulateMain(simulationParams);

    return;
}
