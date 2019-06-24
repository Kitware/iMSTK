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

#pragma once

// Objects
#include "imstkFEMDeformableBodyModel.h"
#include "imstkSceneObject.h"

// Solvers
#include "imstkNonlinearSystem.h"

// Geometry
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCapsule.h"
#include "imstkCube.h"
#include "imstkSurfaceMesh.h"
#include "imstkMeshIO.h"

// logger
#include "g3log/g3log.hpp"
#include "imstkTimer.h"

namespace imstk
{
namespace apiutils
{
///
/// \brief Create a analytical visual scene object that and add it to the scene
///
std::shared_ptr<imstk::VisualObject>
createVisualAnalyticalSceneObject(imstk::Geometry::Type type,
                                  std::shared_ptr<imstk::Scene> scene,
                                  const std::string objName,
                                  const double scale = 1.,
                                  const imstk::Vec3d t = imstk::Vec3d(0., 0., 0.))
{
    if (!scene)
    {
        LOG(WARNING) << "createVisualAnalyticalSceneObject: Scene is not valid!";
        return nullptr;
    }

    if (objName.empty())
    {
        LOG(WARNING) << "createVisualAnalyticalSceneObject: Name is empty!";
        return nullptr;
    }

    std::shared_ptr<imstk::Geometry> geom;
    switch (type)
    {
    case imstk::Geometry::Type::Sphere:
        geom = std::make_shared<imstk::Sphere>();
        break;

    case imstk::Geometry::Type::Plane:
        geom = std::make_shared<imstk::Plane>();
        break;

    case imstk::Geometry::Type::Cube:
        geom = std::make_shared<imstk::Cube>();
        break;

    case imstk::Geometry::Type::Capsule:
        geom = std::make_shared<imstk::Capsule>();
        break;

    default:
        LOG(WARNING) << "createVisualAnalyticalSceneObject: Scene object geometry type is not analytical!";
        return nullptr;
    }

    geom->scale(scale, Geometry::TransformType::ApplyToData);
    geom->translate(t, Geometry::TransformType::ApplyToData);

    auto sceneObj = std::make_shared<imstk::VisualObject>(objName);
    sceneObj->setVisualGeometry(geom);
    scene->addSceneObject(sceneObj);

    return sceneObj;
}

///
/// \brief Create a analytical colliding scene object that and add it to the scene
///
std::shared_ptr<imstk::CollidingObject>
createCollidingAnalyticalSceneObject(imstk::Geometry::Type type,
                                     std::shared_ptr<imstk::Scene> scene,
                                     const std::string objName,
                                     const double scale = 1.,
                                     const imstk::Vec3d t = imstk::Vec3d(0., 0., 0.))
{
    if (!scene)
    {
        LOG(WARNING) << "createCollidingSphereSceneObject: Scene is not valid!";
        return nullptr;
    }

    if (objName.empty())
    {
        LOG(WARNING) << "createCollidingAnalyticalSceneObject: Name is empty!";
        return nullptr;
    }

    std::shared_ptr<imstk::Geometry> geom;
    switch (type)
    {
    case imstk::Geometry::Type::Sphere:
        geom = std::make_shared<imstk::Sphere>();
        break;

    case imstk::Geometry::Type::Plane:
        geom = std::make_shared<imstk::Plane>();
        break;

    case imstk::Geometry::Type::Cube:
        geom = std::make_shared<imstk::Cube>();
        break;

    default:
        LOG(WARNING) << "createCollidingAnalyticalSceneObject: Scene object geometry type is not analytical!";
        return nullptr;
    }

    geom->scale(scale, Geometry::TransformType::ApplyToData);
    geom->translate(t, Geometry::TransformType::ApplyToData);

    auto sceneObj = std::make_shared<imstk::CollidingObject>(objName);
    sceneObj->setVisualGeometry(geom);
    sceneObj->setCollidingGeometry(geom);
    scene->addSceneObject(sceneObj);

    return sceneObj;
}

///
/// \brief Read a mesh, create a visual scene object and add to the scene
///
std::shared_ptr<imstk::SceneObject>
createAndAddVisualSceneObject(std::shared_ptr<imstk::Scene> scene,
                              const std::string fileName,
                              const std::string objectName)
{
    if (!scene)
    {
        LOG(WARNING) << "createAndAddVisualSceneObject: Scene is not valid!";
        return nullptr;
    }

    if (fileName.empty())
    {
        LOG(WARNING) << "createAndAddVisualSceneObject: File name is empty!";
        return nullptr;
    }

    auto mesh = imstk::MeshIO::read(fileName);
    auto SurfaceMesh = std::dynamic_pointer_cast<imstk::SurfaceMesh>(mesh);

    // Create object and add to scene
    auto meshSceneObject = std::make_shared<imstk::VisualObject>("meshObject");
    meshSceneObject->setVisualGeometry(SurfaceMesh);
    meshSceneObject->setName(objectName);
    scene->addSceneObject(meshSceneObject);

    return meshSceneObject;
}

///
/// \brief Create a non-linear system using FEM dynamic model
///
std::shared_ptr<imstk::NonLinearSystem>
createNonLinearSystem(std::shared_ptr<imstk::FEMDeformableBodyModel> dynaModel)
{
    if (!dynaModel)
    {
        LOG(WARNING) << "createNonLinearSystem: Dynamic model is not valid!";
        return nullptr;
    }

    auto nlSystem = std::make_shared<imstk::NonLinearSystem>(
        dynaModel->getFunction(),
        dynaModel->getFunctionGradient());

    std::vector<LinearProjectionConstraint> linProj;
    for (auto i : dynaModel->getFixNodeIds())
    {
        linProj.push_back(LinearProjectionConstraint(i, true));
    }
    nlSystem->setUnknownVector(dynaModel->getUnknownVec());
    nlSystem->setUpdateFunction(dynaModel->getUpdateFunction());
    nlSystem->setUpdatePreviousStatesFunction(dynaModel->getUpdatePrevStateFunction());

    return nlSystem;
}

///
/// \brief Print number of updates for second for a given scene
///
void
printUPS(std::shared_ptr<SceneManager> sceneManager, std::shared_ptr<UPSCounter>& ups)
{
    sceneManager->setPreInitCallback([](Module* module)
            {
                LOG(INFO) << "-- Pre initialization of " << module->getName() << " module";
    });

    sceneManager->setPreUpdateCallback([&ups](Module* module)
            {
                ups->setStartPointOfUpdate();
    });

    sceneManager->setPostUpdateCallback([&ups](Module* module)
            {
                ups->setEndPointOfUpdate();
                std::cout << "\r-- " << module->getName() << " running at "
                          << ups->getUPS() << " ups   " << std::flush;
    });

    sceneManager->setPostCleanUpCallback([](Module* module)
            {
                LOG(INFO) << "\n-- Post cleanup of " << module->getName() << " module";
    });
}
} //apiutils
} // imstk

