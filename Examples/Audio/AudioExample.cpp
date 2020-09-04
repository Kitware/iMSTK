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
#include "imstkLogger.h"

#ifdef iMSTK_AUDIO_ENABLED
#include <SFML/Audio.hpp>
#endif

using namespace imstk;

void
playSound(const std::string& filename)
{
#ifdef iMSTK_AUDIO_ENABLED
    // Load a sound buffer from a .wav file
    sf::SoundBuffer buffer;

    CHECK(buffer.loadFromFile(filename)) << "testSound: Could not open the input sound file: " << filename;

    // Display sound informations
    LOG(INFO) << filename;
    LOG(INFO) << " " << buffer.getDuration().asSeconds() << " seconds";
    LOG(INFO) << " " << buffer.getSampleRate() << " samples / sec";
    LOG(INFO) << " " << buffer.getChannelCount() << " channels";

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
        LOG(INFO) << "\rPlaying... " << sound.getPlayingOffset().asSeconds() << " sec        ";
        LOG(INFO) << std::flush;
    }
    LOG(INFO) << "\n";
#elif
    LOG(INFO) << "testSound: Audio is supported only on windows!";
#endif
}

void
playMusic(const std::string& filename)
{
#ifdef iMSTK_AUDIO_ENABLED
    // Load an ogg music file
    sf::Music music;

    CHECK(music.openFromFile(filename)) << "playMusic: Could not open the input music file: " << filename;

    // Display music informations
    LOG(INFO) << filename << ":";
    LOG(INFO) << " " << music.getDuration().asSeconds() << " seconds";
    LOG(INFO) << " " << music.getSampleRate() << " samples / sec";
    LOG(INFO) << " " << music.getChannelCount() << " channels";

    // Play it
    music.play();

    // Loop while the music is playing
    while (music.getStatus() == sf::Music::Playing)
    {
        // Leave some CPU time for other processes
        sf::sleep(sf::milliseconds(100));

        // Display the playing position
       LOG(INFO) << "\rPlaying... " << music.getPlayingOffset().asSeconds() << " sec        ";
       LOG(INFO) << std::flush;
    }
    LOG(INFO) << "\n";
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
    Logger::startLogger();

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