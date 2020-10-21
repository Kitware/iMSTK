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

///
/// \brief Generate cylinder
///
std::vector<std::shared_ptr<CollidingObject>> generateSolidsScene4(const std::shared_ptr<Scene>& scene)
{
  std::vector<std::shared_ptr<CollidingObject>> solids;

  {
    auto surfaceObject = std::make_shared<VisualObject>("SurfaceObj");
    auto surfMesh = std::dynamic_pointer_cast<SurfaceMesh>(MeshIO::read(iMSTK_DATA_ROOT "/cylinder/cylinder.stl"));

    //auto visualModel = std::make_shared<VisualModel>(surfMesh);
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    material->setColor(Color(Color::Red, 0.1));
    auto color = material->getColor();
    auto surfMeshModel = std::make_shared<VisualModel>(surfMesh);
    surfMeshModel->setRenderMaterial(material);
    surfaceObject->addVisualModel(surfMeshModel);

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
