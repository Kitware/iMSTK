/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkScene.h"
#include "imstkSceneObject.h"
#include "imstkVTKViewer.h"
#include "imstkSphere.h"

#include <gtest/gtest.h>

using namespace imstk;

///
/// \brief Render a hundred empty frames and stop
///
TEST(VTKViewerTest, OpenCloseWindow)
{
    auto scene = std::make_shared<Scene>("TestScene");

    VTKViewer viewer;
    viewer.setActiveScene(scene);
    viewer.init();
    for (int i = 0; i < 100; i++)
    {
        viewer.update();
    }
    viewer.uninit();
}

///
/// \brief Test addition of scene objects to the viewer
///
TEST(VTKViewerTest, AddSceneObject)
{
    auto scene = std::make_shared<Scene>("TestScene");

    auto obj    = std::make_shared<SceneObject>();
    auto sphere = std::make_shared<Sphere>();
    obj->setVisualGeometry(sphere);
    scene->addSceneObject(obj);

    VTKViewer viewer;
    viewer.setActiveScene(scene);
    viewer.init();
    for (int i = 0; i < 1000; i++)
    {
        viewer.update();
    }
    viewer.uninit();
}

///
/// \brief Test runtime addition of scene objects to the viewer
///
TEST(VTKViewerTest, RuntimeAddSceneObject)
{
    auto scene = std::make_shared<Scene>("TestScene");

    VTKViewer viewer;
    viewer.setActiveScene(scene);
    viewer.init();
    for (int i = 0; i < 1000; i++)
    {
        if (i == 500)
        {
            auto obj1    = std::make_shared<SceneObject>();
            auto sphere1 = std::make_shared<Sphere>();
            obj1->setVisualGeometry(sphere1);
            scene->addSceneObject(obj1);

            auto obj2    = std::make_shared<SceneObject>();
            auto sphere2 = std::make_shared<Sphere>();
            obj2->setVisualGeometry(sphere2);
            scene->addSceneObject(obj2);
        }
        viewer.update();
    }
    viewer.uninit();
}

///
/// \brief Test runtime removal of multiple scene objects with the viewer
///
TEST(VTKViewerTest, RuntimeRemoveSceneObjects)
{
    auto scene = std::make_shared<Scene>("TestScene");

    auto obj1    = std::make_shared<SceneObject>("obj1");
    auto sphere1 = std::make_shared<Sphere>();
    obj1->setVisualGeometry(sphere1);
    scene->addSceneObject(obj1);

    auto obj2    = std::make_shared<SceneObject>("obj2");
    auto sphere2 = std::make_shared<Sphere>();
    obj2->setVisualGeometry(sphere1);
    scene->addSceneObject(obj2);

    VTKViewer viewer;
    viewer.setActiveScene(scene);
    viewer.init();
    for (int i = 0; i < 1000; i++)
    {
        if (i == 500)
        {
            scene->removeSceneObject(obj1);
            scene->removeSceneObject(obj2);
        }
        viewer.update();
    }
    viewer.uninit();
}