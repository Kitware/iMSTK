/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkScene.h"
#include "imstkVTKViewer.h"

#include <vtkRenderWindow.h>
#include <gtest/gtest.h>

using namespace imstk;

///
/// \brief Render a hundred empty frames and stop
///
TEST(VTKViewerTest, OpenCloseWindow)
{
        auto scene = std::make_shared<Scene>("Empty Scene");

        VTKViewer viewer;
        viewer.setActiveScene(scene);
        viewer.init();
        for (int i = 0; i < 100; i++)
    {
        viewer.update();
        }
        viewer.uninit();
}