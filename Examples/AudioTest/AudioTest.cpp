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
#include "imstkTimer.h"
#include "imstkSimulationManager.h"

// Objects
#include "imstkForceModelConfig.h"
#include "imstkFEMDeformableBodyModel.h"
#include "imstkVirtualCouplingPBDObject.h"
#include "imstkDynamicObject.h"
#include "imstkDeformableObject.h"
#include "imstkPbdObject.h"
#include "imstkSceneObject.h"
#include "imstkLight.h"
#include "imstkCamera.h"
#include "imstkRigidObject.h"


#include "imstkGraph.h"

// Time Integrators
#include "imstkBackwardEuler.h"

// Solvers
#include "imstkNonlinearSystem.h"
#include "imstkNewtonSolver.h"
#include "imstkConjugateGradient.h"
#include "imstkPbdSolver.h"
#include "imstkGaussSeidel.h"
#include "imstkJacobi.h"
#include "imstkSOR.h"

// Geometry
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCube.h"
#include "imstkCylinder.h"
#include "imstkTetrahedralMesh.h"
#include "imstkHexahedralMesh.h"
#include "imstkSurfaceMesh.h"
#include "imstkMeshIO.h"
#include "imstkLineMesh.h"
#include "imstkDecalPool.h"

// Maps
#include "imstkTetraTriangleMap.h"
#include "imstkIsometricMap.h"
#include "imstkOneToOneMap.h"

// Devices and controllers
#include "imstkHDAPIDeviceClient.h"
#include "imstkHDAPIDeviceServer.h"
#include "imstkVRPNDeviceClient.h"
#include "imstkVRPNDeviceServer.h"
#include "imstkCameraController.h"
#include "imstkSceneObjectController.h"
#include "imstkLaparoscopicToolController.h"

// Collisions
#include "imstkInteractionPair.h"
#include "imstkPointSetToPlaneCD.h"
#include "imstkPointSetToSphereCD.h"
#include "imstkVirtualCouplingCH.h"
#include "imstkPointSetToSpherePickingCD.h"
#include "imstkPickingCH.h"
#include "imstkBoneDrillingCH.h"

// logger
#include "g3log/g3log.hpp"
#include "imstkLogger.h"

// imstk utilities
#include "imstkPlotterUtils.h"
#include "imstkAPIUtilities.h"

// testVTKTexture
#include <vtkOBJReader.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <string>
#include <vtkJPEGReader.h>

#ifdef iMSTK_AUDIO_ENABLED

// Audio
#include <SFML/Audio.hpp>

#endif

using namespace imstk;

// Example modified from SFML/Examples
void testSound(const std::string& filename)
{
#ifdef iMSTK_AUDIO_ENABLED
    // Load a sound buffer from a .wav file
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(filename))
    {
        LOG(WARNING) << "testSound: Could not open the input sound file: " << filename;
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
        sf::Listener::setPosition(listnerPos.x(), listnerPos.y(), listnerPos.z());

        // Display the playing position
        std::cout << "\rPlaying... " << sound.getPlayingOffset().asSeconds() << " sec        ";
        std::cout << std::flush;
    }
    std::cout << "\n" << std::endl;
#else
    LOG(INFO) << "testSound: Audio is supported only on windows!";
#endif
}

// Example modified from SFML/Examples
void playMusic(const std::string& filename)
{
#ifdef iMSTK_AUDIO_ENABLED
    // Load an ogg music file
    sf::Music music;
    if (!music.openFromFile(filename))
    {
        LOG(WARNING) << "playMusic: Could not open the input music file: " << filename;
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

void main()
{
    std::cout << "--Testing audio--\n\n";

    // Test a sound
    testSound(iMSTK_DATA_ROOT "/sound/canary.wav");

    // Test music from an .ogg file
    playMusic(iMSTK_DATA_ROOT "/sound/orchestral.ogg");
}