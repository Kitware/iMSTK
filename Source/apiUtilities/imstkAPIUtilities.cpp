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
#include "imstkAPIUtilities.h"

// Objects
#include "imstkFEMDeformableBodyModel.h"
#include "imstkSceneObject.h"
#include "imstkSceneManager.h"
#include "imstkCollidingObject.h"
#include "imstkScene.h"

#include "imstkColor.h"

// Solvers
#include "imstkNonLinearSystem.h"

// Geometry
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkCapsule.h"
#include "imstkCube.h"
#include "imstkPointSet.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"
#include "imstkMeshIO.h"
#include "imstkImageData.h"

#include "imstkGraph.h"
#include "imstkCollisionGraph.h"
#include "imstkCamera.h"
#include "imstkLinearProjectionConstraint.h"

// logger
#include "imstkLogger.h"
#include "imstkTimer.h"

namespace imstk
{
namespace apiutils
{
///
/// \brief Create a analytical visual scene object that and add it to the scene
///
std::shared_ptr<VisualObject>
createVisualAnalyticalSceneObject(Geometry::Type         type,
                                  std::shared_ptr<Scene> scene,
                                  const std::string&     objName,
                                  const double           scale /* = 1.*/,
                                  const Vec3d            t /*= Vec3d(0., 0., 0.)*/)
{
    CHECK(scene != nullptr) << "createVisualAnalyticalSceneObject: Scene is not valid!";
    CHECK(!objName.empty()) << "createVisualAnalyticalSceneObject: Name is empty!";

    std::shared_ptr<Geometry> geom;
    switch (type)
    {
    case Geometry::Type::Sphere:
        geom = std::make_shared<Sphere>();
        break;

    case Geometry::Type::Plane:
        geom = std::make_shared<Plane>();
        break;

    case Geometry::Type::Cube:
        geom = std::make_shared<Cube>();
        break;

    case Geometry::Type::Capsule:
        geom = std::make_shared<Capsule>();
        break;

    case Geometry::Type::ImageData:
        geom = std::make_shared<ImageData>();
        break;

    default:
        LOG(WARNING) << "createVisualAnalyticalSceneObject: Scene object geometry type is not analytical!";
        return nullptr;
    }

    geom->scale(scale, Geometry::TransformType::ApplyToData);
    geom->translate(t, Geometry::TransformType::ApplyToData);

    auto sceneObj = std::make_shared<VisualObject>(objName);
    sceneObj->setVisualGeometry(geom);
    scene->addSceneObject(sceneObj);

    return sceneObj;
}

///
/// \brief Create a analytical colliding scene object that and add it to the scene
///
std::shared_ptr<CollidingObject>
createCollidingAnalyticalSceneObject(Geometry::Type         type,
                                     std::shared_ptr<Scene> scene,
                                     const std::string&     objName,
                                     const double           scale /*= 1.*/,
                                     const Vec3d            t /*= Vec3d(0., 0., 0.)*/)
{
    CHECK(scene != nullptr) << "createCollidingSphereSceneObject: Scene is not valid!";
    CHECK(!objName.empty()) << "createCollidingAnalyticalSceneObject: Name is empty!";

    std::shared_ptr<Geometry> geom;
    switch (type)
    {
    case Geometry::Type::Sphere:
        geom = std::make_shared<Sphere>();
        break;

    case Geometry::Type::Plane:
        geom = std::make_shared<Plane>();
        break;

    case Geometry::Type::Cube:
        geom = std::make_shared<Cube>();
        break;

    default:
        LOG(WARNING) << "createCollidingAnalyticalSceneObject: Scene object geometry type is not analytical!";
        return nullptr;
    }

    geom->scale(scale, Geometry::TransformType::ApplyToData);
    geom->translate(t, Geometry::TransformType::ApplyToData);

    auto sceneObj = std::make_shared<CollidingObject>(objName);
    sceneObj->setVisualGeometry(geom);
    sceneObj->setCollidingGeometry(geom);
    scene->addSceneObject(sceneObj);

    return sceneObj;
}

///
/// \brief Read a mesh, create a visual scene object and add to the scene
///
std::shared_ptr<SceneObject>
createAndAddVisualSceneObject(std::shared_ptr<Scene> scene,
                              const std::string&     fileName,
                              const std::string&     objectName)
{
    CHECK(scene != nullptr) << "createAndAddVisualSceneObject: Scene is not valid!";
    CHECK(!fileName.empty()) << "createAndAddVisualSceneObject: Name is empty!";

    auto mesh = MeshIO::read(fileName);
    auto SurfaceMesh = std::dynamic_pointer_cast<imstk::SurfaceMesh>(mesh);

    // Create object and add to scene
    auto meshSceneObject = std::make_shared<VisualObject>("meshObject");
    meshSceneObject->setVisualGeometry(SurfaceMesh);
    meshSceneObject->setName(objectName);
    scene->addSceneObject(meshSceneObject);

    return meshSceneObject;
}

///
/// \brief Create a non-linear system using FEM dynamic model
///
std::shared_ptr<NonLinearSystem<SparseMatrixd>>
createNonLinearSystem(std::shared_ptr<FEMDeformableBodyModel> dynaModel)
{
    CHECK(dynaModel != nullptr) << "createNonLinearSystem: Dynamic model is not valid!";

    auto nlSystem = std::make_shared<NonLinearSystem<SparseMatrixd>>(
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
printUPS(std::shared_ptr<SceneManager> sceneManager)
{
    if (!sceneManager)
    {
        LOG(WARNING) << "APIUtilities::printUPS - scene manager is not valid! Unable to print UPS";
        return;
    }

    sceneManager->setPostUpdateCallback([&sceneManager](Module* module)
            {
                std::cout << "\r" << module->getName() << " running at "
                          << sceneManager->getUPS() << " ups   " << std::flush;
                });
}

std::shared_ptr<Graph>
getMeshGraph(std::shared_ptr<PointSet> m)
{
    LOG(WARNING) << "The graph of a point set has no edges";

    return std::make_shared<Graph>(m->getNumVertices());
}

std::shared_ptr<Graph>
getMeshGraph(std::shared_ptr<SurfaceMesh> m)
{
    auto gMesh = std::make_shared<Graph>(m->getNumVertices());
    for (auto tri : m->getTrianglesVertices())
    {
        gMesh->addEdge(tri[0], tri[1]);
        gMesh->addEdge(tri[0], tri[2]);
        gMesh->addEdge(tri[1], tri[2]);
    }

    return gMesh;
}

std::shared_ptr<Graph>
getMeshGraph(std::shared_ptr<TetrahedralMesh> m)
{
    auto gMesh = std::make_shared<Graph>(m->getNumVertices());
    for (auto tet : m->getTetrahedraVertices())
    {
        gMesh->addEdge(tet[0], tet[1]);
        gMesh->addEdge(tet[0], tet[2]);
        gMesh->addEdge(tet[0], tet[3]);
        gMesh->addEdge(tet[1], tet[2]);
        gMesh->addEdge(tet[1], tet[3]);
        gMesh->addEdge(tet[2], tet[3]);
    }
    return gMesh;
}
}     //apiutils
} // imstk
