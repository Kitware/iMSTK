/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

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
        const vtksys::Status fileStatus = vtksys::SystemTools::RemoveFile(screenshotFileName);
        EXPECT_TRUE(fileStatus.IsSuccess()) << "Failed to remove existing screenshot file before screenshotTest";
        if (!fileStatus.IsSuccess())
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