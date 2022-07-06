/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

//#include "imstkCylinder.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkMeshIO.h"
//#include "imstkVTKMeshIO.h"
//#include "imstkGeometryUtilities.h"

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

        imstkNew<CollidingObject> obj("Floor");
        obj->setVisualGeometry(geometry);
        obj->getVisualModel(0)->getRenderMaterial()->setColor(Color::DarkGray);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(40.0);
        geometry->setPosition(0.0, -6.0, 0.0);
        geometry->setNormal(Vec3d(0.0, 1.0, 1.0));

        imstkNew<CollidingObject> obj("Back Plane");
        obj->setVisualGeometry(geometry);
        obj->getVisualModel(0)->getRenderMaterial()->setColor(Color::LightGray);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }
    {
        imstkNew<Sphere> geometry;
        geometry->setRadius(2.0);
        geometry->setPosition(0.0, -6.0, 0.0);

        imstkNew<CollidingObject> obj("Sphere on Floor");
        obj->setVisualGeometry(geometry);
        obj->getVisualModel(0)->getRenderMaterial()->setColor(Color::Red);
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

        imstkNew<CollidingObject> obj("Floor");
        obj->setVisualGeometry(geometry);
        obj->getVisualModel(0)->getRenderMaterial()->setColor(Color::DarkGray);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(40.0);
        geometry->setPosition(0.0, -6.0, 0.0);
        geometry->setNormal(Vec3d(0.0, 1.0, 1.0));

        imstkNew<CollidingObject> obj("Back Plane");
        obj->setVisualGeometry(geometry);
        obj->getVisualModel(0)->getRenderMaterial()->setColor(Color::LightGray);
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

    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(0.0, -6.0, 0.0);
        geometry->setNormal(Vec3d(0.0, 1.0, 0.0));

        imstkNew<CollidingObject> obj("Floor");
        obj->setVisualGeometry(geometry);
        obj->getVisualModel(0)->getRenderMaterial()->setColor(Color(0.2, 0.2, 0.2, 1.0));
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(0.0, 0.0, -7.0);
        geometry->setNormal(Vec3d(0.0, 0.0, 1.0));

        imstkNew<CollidingObject> obj("Back Wall");
        obj->setVisualGeometry(geometry);
        obj->getVisualModel(0)->getRenderMaterial()->setColor(Color::LightGray);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(0.0, 0.0, 7.0);
        geometry->setNormal(Vec3d(0.0, 0.0, -1.0));

        imstkNew<CollidingObject> obj("Front Wall");
        obj->setVisualGeometry(geometry);
        obj->getVisualModel(0)->getRenderMaterial()->setColor(Color::LightGray);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(7.0, 0.0, 0.0);
        geometry->setNormal(Vec3d(-1.0, 0.0, 0.0));

        imstkNew<CollidingObject> obj("Left Wall");
        obj->setVisualGeometry(geometry);
        obj->getVisualModel(0)->getRenderMaterial()->setColor(Color::LightGray);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }
    {
        imstkNew<Plane> geometry;
        geometry->setWidth(14.0);
        geometry->setPosition(-7.0, 0.0, 0.0);
        geometry->setNormal(Vec3d(1.0, 0.0, 0.0));

        imstkNew<CollidingObject> obj("Right Wall");
        obj->setVisualGeometry(geometry);
        obj->getVisualModel(0)->getRenderMaterial()->setColor(Color::LightGray);
        obj->setCollidingGeometry(geometry);
        solids.push_back(obj);
    }

    return solids;
}

///
/// \brief Generate cylinder
///
std::vector<std::shared_ptr<CollidingObject>> generateSolidsScene4(const std::shared_ptr<Scene>& scene)
{
  std::vector<std::shared_ptr<CollidingObject>> solids;

  {
    auto surfaceObject = std::make_shared<SceneObject>("SurfaceObj");
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

    auto obj = std::make_shared<CollidingObject>("cylinder");
    //obj->addVisualModel(visualModel);
    obj->setCollidingGeometry(surfMesh);
    scene->addSceneObject(obj);
    solids.push_back(obj);
  }
  return solids;
}

std::vector<std::shared_ptr<CollidingObject>> generateSolids(const std::shared_ptr<Scene>& scene)
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
