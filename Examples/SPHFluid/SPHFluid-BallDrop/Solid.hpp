/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkCollider.h"
#include "imstkEntity.h"
#include "imstkNew.h"
#include "imstkScene.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkMeshIO.h"
#include "imstkVisualModel.h"

#include "imstkPlane.h"
#include "imstkRenderMaterial.h"
#include "imstkSphere.h"

using namespace imstk;

std::shared_ptr<Entity> makeObject(const std::string& name, std::shared_ptr<Geometry> geometry, const imstk::Color& color)
{
    imstkNew<SceneObject> obj(name);
    auto visualModel = obj->addComponent<VisualModel>();
    visualModel->setGeometry(geometry);
    visualModel->getRenderMaterial()->setColor(color);
    auto collider = obj->addComponent<Collider>();
    collider->setGeometry(geometry);
    return obj;
}

///
/// \brief Generate two planes and a solid sphere
///
std::vector<std::shared_ptr<Entity>> generateSolidsScene1()
{
    std::vector<std::shared_ptr<Entity>> solids;

    {
        imstkNew<Plane> geometry;
        geometry->setWidth(40.0);
        geometry->setPosition(0.0, -6.0, 0.0);
        geometry->setNormal(Vec3d(0.0, 1.0, -0.5));

        solids.push_back(makeObject("Floor", geometry, Color::DarkGray));
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(40.0);
        geometry->setPosition(0.0, -6.0, 0.0);
        geometry->setNormal(Vec3d(0.0, 1.0, 1.0));

        solids.push_back(makeObject("Back Plane", geometry, Color::LightGray));
    }
    {
        imstkNew<Sphere> geometry;
        geometry->setRadius(2.0);
        geometry->setPosition(0.0, -6.0, 0.0);

        solids.push_back(makeObject("Sphere on Floor", geometry, Color::Red));
    }

    return solids;
}

///
/// \brief Generate two planes
///
std::vector<std::shared_ptr<Entity>> generateSolidsScene2()
{
    std::vector<std::shared_ptr<Entity>> solids;

    {
        imstkNew<Plane> geometry;
        geometry->setWidth(40.0);
        geometry->setPosition(0.0, -6.0, 0.0);
        geometry->setNormal(Vec3d(0.0, 1.0, -0.5));

        solids.push_back(makeObject("Floor", geometry, Color::DarkGray));
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(40.0);
        geometry->setPosition(0.0, -6.0, 0.0);
        geometry->setNormal(Vec3d(0.0, 1.0, 1.0));

        solids.push_back(makeObject("Back Plane", geometry, Color::LightGray));
    }

    return solids;
}

///
/// \brief Generate an open box by 5 planes: 1 floor and 4 walls
///
std::vector<std::shared_ptr<Entity>> generateSolidsScene3()
{
    std::vector<std::shared_ptr<Entity>> solids;

    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(0.0, -6.0, 0.0);
        geometry->setNormal(Vec3d(0.0, 1.0, 0.0));

        solids.push_back(makeObject("Floor", geometry, Color(0.2, 0.2, 0.2, 1.0)));
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(0.0, 0.0, -7.0);
        geometry->setNormal(Vec3d(0.0, 0.0, 1.0));

        solids.push_back(makeObject("Back Wall", geometry, Color::LightGray));
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(0.0, 0.0, 7.0);
        geometry->setNormal(Vec3d(0.0, 0.0, -1.0));

        solids.push_back(makeObject("Front Wall", geometry, Color::LightGray));
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(7.0, 0.0, 0.0);
        geometry->setNormal(Vec3d(-1.0, 0.0, 0.0));

        solids.push_back(makeObject("Left Wall", geometry, Color::LightGray));
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(-7.0, 0.0, 0.0);
        geometry->setNormal(Vec3d(1.0, 0.0, 0.0));

        solids.push_back(makeObject("Right Wall", geometry, Color::LightGray));
    }

    return solids;
}

///
/// \brief Generate cylinder
///
std::vector<std::shared_ptr<Entity>> generateSolidsScene4(const std::shared_ptr<Scene>& scene)
{
  std::vector<std::shared_ptr<Entity>> solids;

  {
    auto surfaceObject = std::make_shared<Entity>("SurfaceObj");
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder.stl"));

    //auto visualModel = std::make_shared<VisualModel>(surfMesh);
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    material->setColor(Color(Color::Red, 0.1));
    auto surfMeshModel = surfaceObject->addComponent<VisualModel>();
    surfMeshModel->setGeometry(surfMesh);
    surfMeshModel->setRenderMaterial(material);

    // add the scene object to the scene
    scene->addSceneObject(surfaceObject);

    //const size_t nx = 200 / 2, ny = 100 / 2, nz = 100 / 2;
    //auto tetMesh = GeometryUtils::createTetrahedralMeshCover(surfMesh, nx, ny, nz);


    // add scene object for surface object
    //auto volObject = std::make_shared<VisualObject>("VolObj");
    //volObject->addVisualModel(visualModel);
    //volObject->setVisualGeometry(tetMesh);
    //volObject->addVisualModel(visualModel);
    
    // add the scene object to the scene
    //scene->addSceneObject(volObject);
    //solids.push_back(volObject);

    auto obj = std::make_shared<SceneObject>("cylinder");
    //obj->addVisualModel(visualModel);
    obj->addComponent<Collider>()->setGeometry(surfMesh);
    scene->addSceneObject(obj);
    solids.push_back(obj);
  }
  return solids;
}

std::vector<std::shared_ptr<Entity>> generateSolids(const std::shared_ptr<Scene>& scene)
{
    switch (SCENE_ID)
    {
    case 1:
        return generateSolidsScene1();
    case 2:
        return generateSolidsScene2();
    case 3:
        return generateSolidsScene3();
    case 4:
      return generateSolidsScene4(scene);
    default:
        LOG(FATAL) << "Invalid scene index";
        return {}; // To avoid warning
    }
}
