/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include "imstkVisualTestingUtils.h"

namespace imstk
{
class DirectionalLight;
class Geometry;
class RenderMaterial;
class SceneObject;
class VisualModel;
} // namespace imstk

using namespace imstk;

class RenderTest : public VisualTest
{
public:
    void runAllMaterials();

protected:
    void createScene();

    void updateMaterial();

    void applyColor();

    // Render Contents
    std::shared_ptr<DirectionalLight> light;
    std::shared_ptr<Geometry>       geom;
    std::shared_ptr<RenderMaterial> renderMaterial;
    std::shared_ptr<VisualModel>    visualModel;
    std::shared_ptr<SceneObject>    sceneObj;

    int displayMode;
    int color;
    int shadingModel;
    int blendMode;
    std::string dm, c, sm, bm;
};