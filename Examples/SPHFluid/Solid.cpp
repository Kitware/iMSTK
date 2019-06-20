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

#include "imstkSimulationManager.h"
#include "imstkPlane.h"
#include "imstkSphere.h"

using namespace imstk;

///
/// \brief Generate two planes and a solid sphere
///
std::vector<std::shared_ptr<CollidingObject>> generateSolidsScene1(const std::shared_ptr<Scene>& scene)
{
    std::vector<std::shared_ptr<CollidingObject>> solids;

    {
        auto geometry = std::make_shared<Plane>();
        geometry->setWidth(40);
        geometry->setPosition(0, -6, 0);
        geometry->setNormal(Vec3d(0, 1, -0.5));

        auto visualModel = std::make_shared<VisualModel>(geometry);
        auto material        = std::make_shared<RenderMaterial>();
        material->setColor(Color::DarkGray);
        visualModel->setRenderMaterial(material);

        auto obj = std::make_shared<CollidingObject>("Floor");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        scene->addSceneObject(obj);
        solids.push_back(obj);
    }
    {
        auto geometry = std::make_shared<Plane>();
        geometry->setWidth(40);
        geometry->setPosition(0, -6, 0);
        geometry->setNormal(Vec3d(0, 1, 1));

        auto visualModel = std::make_shared<VisualModel>(geometry);
        auto material = std::make_shared<RenderMaterial>();
        material->setColor(Color::LightGray);
        visualModel->setRenderMaterial(material);

        auto obj = std::make_shared<CollidingObject>("Back Plane");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        scene->addSceneObject(obj);
        solids.push_back(obj);
    }
    {
        auto geometry = std::make_shared<Sphere>();
        geometry->setRadius(2);
        geometry->setPosition(0, -6, 0);

        auto visualModel = std::make_shared<VisualModel>(geometry);
        auto material    = std::make_shared<RenderMaterial>();
        material->setColor(Color::Red);
        visualModel->setRenderMaterial(material);

        auto obj = std::make_shared<CollidingObject>("Sphere on Floor");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        scene->addSceneObject(obj);
        solids.push_back(obj);
    }

    return solids;
}

///
/// \brief Generate two planes
///
std::vector<std::shared_ptr<CollidingObject>> generateSolidsScene2(const std::shared_ptr<Scene>& scene)
{
    std::vector<std::shared_ptr<CollidingObject>> solids;

    {
        auto geometry = std::make_shared<Plane>();
        geometry->setWidth(40);
        geometry->setPosition(0, -6, 0);
        geometry->setNormal(Vec3d(0, 1, -0.5));

        auto visualModel = std::make_shared<VisualModel>(geometry);
        auto material        = std::make_shared<RenderMaterial>();
        material->setColor(Color::DarkGray);
        visualModel->setRenderMaterial(material);

        auto obj = std::make_shared<CollidingObject>("Floor");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        scene->addSceneObject(obj);
        solids.push_back(obj);
    }
    {
        auto geometry = std::make_shared<Plane>();
        geometry->setWidth(40);
        geometry->setPosition(0, -6, 0);
        geometry->setNormal(Vec3d(0, 1, 1));

        auto visualModel = std::make_shared<VisualModel>(geometry);
        auto material        = std::make_shared<RenderMaterial>();
        material->setColor(Color::LightGray);
        visualModel->setRenderMaterial(material);

        auto obj = std::make_shared<CollidingObject>("Back Plane");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        scene->addSceneObject(obj);
        solids.push_back(obj);
    }

    return solids;
}

///
/// \brief Generate an open box by 5 planes: 1 floor and 4 walls
///
std::vector<std::shared_ptr<CollidingObject>> generateSolidsScene3(const std::shared_ptr<Scene>& scene)
{
    std::vector<std::shared_ptr<CollidingObject>> solids;

    {
        auto geometry = std::make_shared<Plane>();
        geometry->setWidth(14);
        geometry->setPosition(0, -6, 0);
        geometry->setNormal(Vec3d(0, 1, 0));

        auto visualModel = std::make_shared<VisualModel>(geometry);
        auto material = std::make_shared<RenderMaterial>();
        material->setColor(Color(0.2, 0.2, 0.2, 1.0));
        visualModel->setRenderMaterial(material);

        auto obj = std::make_shared<CollidingObject>("Floor");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        scene->addSceneObject(obj);
        solids.push_back(obj);
    }
    {
        auto geometry = std::make_shared<Plane>();
        geometry->setWidth(14);
        geometry->setPosition(0, 0, -7);
        geometry->setNormal(Vec3d(0, 0, 1));

        auto visualModel = std::make_shared<VisualModel>(geometry);
        auto material = std::make_shared<RenderMaterial>();
        material->setColor(Color::LightGray);
        visualModel->setRenderMaterial(material);

        auto obj = std::make_shared<CollidingObject>("Back Wall");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        scene->addSceneObject(obj);
        solids.push_back(obj);
    }
    {
        auto geometry = std::make_shared<Plane>();
        geometry->setWidth(14);
        geometry->setPosition(0, 0, 7);
        geometry->setNormal(Vec3d(0, 0, -1));

        auto visualModel = std::make_shared<VisualModel>(geometry);
        auto material = std::make_shared<RenderMaterial>();
        material->setColor(Color::LightGray);
        visualModel->setRenderMaterial(material);

        auto obj = std::make_shared<CollidingObject>("Front Wall");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        scene->addSceneObject(obj);
        solids.push_back(obj);
    }
    {
        auto geometry = std::make_shared<Plane>();
        geometry->setWidth(14);
        geometry->setPosition(7, 0, 0);
        geometry->setNormal(Vec3d(-1, 0, 0));

        auto visualModel = std::make_shared<VisualModel>(geometry);
        auto material = std::make_shared<RenderMaterial>();
        material->setColor(Color::LightGray);
        visualModel->setRenderMaterial(material);

        auto obj = std::make_shared<CollidingObject>("Left Wall");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        scene->addSceneObject(obj);
        solids.push_back(obj);
    }
    {
        auto geometry = std::make_shared<Plane>();
        geometry->setWidth(14);
        geometry->setPosition(-7, 0, 0);
        geometry->setNormal(Vec3d(1, 0, 0));

        auto visualModel = std::make_shared<VisualModel>(geometry);
        auto material = std::make_shared<RenderMaterial>();
        material->setColor(Color::LightGray);
        visualModel->setRenderMaterial(material);

        auto obj = std::make_shared<CollidingObject>("Right Wall");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        scene->addSceneObject(obj);
        solids.push_back(obj);
    }

    return solids;
}

std::vector<std::shared_ptr<CollidingObject>> generateSolids(const std::shared_ptr<Scene>& scene, int sceneIdx)
{
    switch (sceneIdx)
    {
    case 1:
        return generateSolidsScene1(scene);
    case 2:
        return generateSolidsScene2(scene);
    case 3:
        return generateSolidsScene3(scene);
    default:
        LOG(FATAL) << "Invalid scene index";
        return {}; // To avoid warning
    }
}
