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

#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkSphere.h"

using namespace imstk;

///
/// \brief Generate two planes and a solid sphere
///
std::vector<std::shared_ptr<CollidingObject>> generateSolidsScene1()
{
    std::vector<std::shared_ptr<CollidingObject>> solids;

    {
        imstkNew<Plane> geometry;
        geometry->setWidth(40.0);
        geometry->setPosition(0.0, -6.0, 0.0);
        geometry->setNormal(Vec3d(0.0, 1.0, -0.5));

        imstkNew<VisualModel> visualModel(geometry.get());
        imstkNew<RenderMaterial> material;
        material->setColor(Color::DarkGray);
        visualModel->setRenderMaterial(material);

        imstkNew<CollidingObject> obj("Floor");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(40.0);
        geometry->setPosition(0.0, -6.0, 0.0);
        geometry->setNormal(Vec3d(0.0, 1.0, 1.0));

        imstkNew<VisualModel> visualModel(geometry.get());
        imstkNew<RenderMaterial> material;
        material->setColor(Color::LightGray);
        visualModel->setRenderMaterial(material);

        imstkNew<CollidingObject> obj("Back Plane");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }
    {
        imstkNew<Sphere> geometry;
        geometry->setRadius(2.0);
        geometry->setPosition(0.0, -6.0, 0.0);

        imstkNew<VisualModel> visualModel(geometry.get());
        imstkNew<RenderMaterial> material;
        material->setColor(Color::Red);
        visualModel->setRenderMaterial(material);

        imstkNew<CollidingObject> obj("Sphere on Floor");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }

    return solids;
}

///
/// \brief Generate two planes
///
std::vector<std::shared_ptr<CollidingObject>> generateSolidsScene2()
{
    std::vector<std::shared_ptr<CollidingObject>> solids;

    {
        imstkNew<Plane> geometry;
        geometry->setWidth(40.0);
        geometry->setPosition(0.0, -6.0, 0.0);
        geometry->setNormal(Vec3d(0.0, 1.0, -0.5));

        imstkNew<VisualModel> visualModel(geometry.get());
        imstkNew<RenderMaterial> material;
        material->setColor(Color::DarkGray);
        visualModel->setRenderMaterial(material);

        imstkNew<CollidingObject> obj("Floor");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(40.0);
        geometry->setPosition(0.0, -6.0, 0.0);
        geometry->setNormal(Vec3d(0.0, 1.0, 1.0));

        imstkNew<VisualModel> visualModel(geometry.get());
        imstkNew<RenderMaterial> material;
        material->setColor(Color::LightGray);
        visualModel->setRenderMaterial(material);

        imstkNew<CollidingObject> obj("Back Plane");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }

    return solids;
}

///
/// \brief Generate an open box by 5 planes: 1 floor and 4 walls
///
std::vector<std::shared_ptr<CollidingObject>> generateSolidsScene3()
{
    std::vector<std::shared_ptr<CollidingObject>> solids;

    imstkNew<RenderMaterial> lightGrayMat;
    lightGrayMat->setColor(Color::LightGray);

    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(0.0, -6.0, 0.0);
        geometry->setNormal(Vec3d(0.0, 1.0, 0.0));

        imstkNew<VisualModel> visualModel(geometry.get());
        imstkNew<RenderMaterial> material;
        material->setColor(Color(0.2, 0.2, 0.2, 1.0));
        visualModel->setRenderMaterial(material);

        imstkNew<CollidingObject> obj("Floor");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(0.0, 0.0, -7.0);
        geometry->setNormal(Vec3d(0.0, 0.0, 1.0));

        imstkNew<VisualModel> visualModel(geometry.get());
        visualModel->setRenderMaterial(lightGrayMat);

        imstkNew<CollidingObject> obj("Back Wall");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(0.0, 0.0, 7.0);
        geometry->setNormal(Vec3d(0.0, 0.0, -1.0));

        imstkNew<VisualModel> visualModel(geometry.get());
        visualModel->setRenderMaterial(lightGrayMat);

        imstkNew<CollidingObject> obj("Front Wall");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(7.0, 0.0, 0.0);
        geometry->setNormal(Vec3d(-1.0, 0.0, 0.0));

        imstkNew<VisualModel> visualModel(geometry.get());
        visualModel->setRenderMaterial(lightGrayMat);

        imstkNew<CollidingObject> obj("Left Wall");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(-7.0, 0.0, 0.0);
        geometry->setNormal(Vec3d(1.0, 0.0, 0.0));

        imstkNew<VisualModel> visualModel(geometry.get());
        visualModel->setRenderMaterial(lightGrayMat);

        imstkNew<CollidingObject> obj("Right Wall");
        obj->addVisualModel(visualModel);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }

    return solids;
}

std::vector<std::shared_ptr<CollidingObject>> generateSolids()
{
    switch (SCENE_ID)
    {
    case 1:
        return generateSolidsScene1();
    case 2:
        return generateSolidsScene2();
    case 3:
        return generateSolidsScene3();
    default:
        LOG(FATAL) << "Invalid scene index";
        return {}; // To avoid warning
    }
}
