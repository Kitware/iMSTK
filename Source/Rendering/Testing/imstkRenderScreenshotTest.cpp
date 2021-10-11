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

#include "imstkNew.h"
#include "imstkScene.h"
#include "imstkSceneObject.h"
#include "imstkSphere.h"
#include "imstkVTKScreenCaptureUtility.h"
#include "imstkVTKViewer.h"

#include <gtest/gtest.h>
#include <vtksys/SystemTools.hxx>

using namespace imstk;

///
/// \brief Test we are able to output a screenshot
///
TEST(imstkRenderScreenshotTest, ScreenshotTest)
{
    // If the screenshot already exists, clear it
    const std::string screenshotFileName = "Screenshot-" + std::to_string(0) + ".png";
    if (vtksys::SystemTools::FileExists(screenshotFileName))
    {
        const bool fileWasRemoved = vtksys::SystemTools::RemoveFile(screenshotFileName);
        EXPECT_TRUE(fileWasRemoved) << "Failed to remove existing screenshot file before screenshotTest";
        if (!fileWasRemoved)
        {
            return;
        }
    }

    imstkNew<Sphere> sphere(Vec3d(0.0, 0.0, 0.0), 1.0);

    imstkNew<SceneObject> sphereObj("TestObj");
    sphereObj->setVisualGeometry(sphere);

    imstkNew<Scene> scene("TestScene");
    scene->addSceneObject(sphereObj);

    imstkNew<VTKViewer> viewer;
    viewer->setActiveScene(scene);
    viewer->init();
    viewer->update();
    viewer->getScreenCaptureUtility()->saveScreenShot();
    viewer->uninit();

    EXPECT_TRUE(vtksys::SystemTools::FileExists(screenshotFileName));
}