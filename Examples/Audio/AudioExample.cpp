/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

#include "imstkMath.h"
#include "imstkLogUtility.h"

#ifdef iMSTK_AUDIO_ENABLED

// Audio
#include <SFML/Audio.hpp>

#endif

using namespace imstk;

void
playSound(const std::string& filename)
{
#ifdef iMSTK_AUDIO_ENABLED
    // Load a sound buffer from a .wav file
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(filename))
    {
        LOG(FATAL) << "testSound: Could not open the input sound file: " << filename;
        return;
    }

    // Display sound informations
    std::cout << filename << std::endl;
    std::cout << " " << buffer.getDuration().asSeconds() << " seconds" << std::endl;
    std::cout << " " << buffer.getSampleRate() << " samples / sec" << std::endl;
    std::cout << " " << buffer.getChannelCount() << " channels" << std::endl;

    // Create a sound instance and play it
    sf::Sound sound(buffer);
    sound.setPosition(0., 0., 0.);
    sound.setMinDistance(5.);
    sound.setAttenuation(10.);

    sound.play();

    Vec3d listnerPos(-5., -5., -5.);
    // Loop while the sound is playing
    while (sound.getStatus() == sf::Sound::Playing)
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(100));

        // Move the listener away
        listnerPos += Vec3d(.2, .2, .2);
        sf::Listener::setPosition((float)listnerPos.x(), (float)listnerPos.y(), (float)listnerPos.z());

        // Display the playing position
        std::cout << "\rPlaying... " << sound.getPlayingOffset().asSeconds() << " sec        ";
        std::cout << std::flush;
    }
    std::cout << "\n" << std::endl;
#else if
    LOG(INFO) << "testSound: Audio is supported only on windows!";
#endif
}

void
playMusic(const std::string& filename)
{
#ifdef iMSTK_AUDIO_ENABLED
    // Load an ogg music file
    sf::Music music;
    if (!music.openFromFile(filename))
    {
        LOG(FATAL) << "playMusic: Could not open the input music file: " << filename;
        return;
    }

    // Display music informations
    std::cout << filename << ":" << std::endl;
    std::cout << " " << music.getDuration().asSeconds() << " seconds" << std::endl;
    std::cout << " " << music.getSampleRate() << " samples / sec" << std::endl;
    std::cout << " " << music.getChannelCount() << " channels" << std::endl;

    // Play it
    music.play();

    // Loop while the music is playing
    while (music.getStatus() == sf::Music::Playing)
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(100));

        // Display the playing position
        std::cout << "\rPlaying... " << music.getPlayingOffset().asSeconds() << " sec        ";
        std::cout << std::flush;
    }
    std::cout << "\n" << std::endl;
#else
    LOG(INFO) << "playMusic: Audio is supported only on windows!";
#endif
}

///
/// \brief This example demonstrates the audio feature in imstk.
/// NOTE: Example modified from SFML/Examples
///
int
main()
{
    // Initialize g3logger
    auto logger = std::make_shared<LogUtility>();
    logger->createLogger("audio-Example", "./");

    #ifndef iMSTK_AUDIO_ENABLED
    LOG(INFO) << "Audio not enabled at build time\n";
    return 1;
    #endif

    LOG(INFO) << "--Testing audio--\n";

    // Test a sound
    playSound(iMSTK_DATA_ROOT "/sound/canary.wav");

    // Test music from an .ogg file
    playMusic(iMSTK_DATA_ROOT "/sound/orchestral.ogg");

    return 0;
}