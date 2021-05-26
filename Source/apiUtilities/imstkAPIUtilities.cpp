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
#include "imstkGraph.h"
#include "imstkLinearProjectionConstraint.h"
#include "imstkLogger.h"
#include "imstkVecDataArray.h"

// Objects
#include "imstkCollidingObject.h"
#include "imstkFEMDeformableBodyModel.h"
#include "imstkScene.h"
#include "imstkSceneManager.h"

// Geometry
#include "imstkCapsule.h"
#include "imstkOrientedBox.h"
#include "imstkImageData.h"
#include "imstkMeshIO.h"
#include "imstkPlane.h"
#include "imstkSphere.h"
#include "imstkSurfaceMesh.h"
#include "imstkTetrahedralMesh.h"

namespace imstk
{
namespace apiutils
{
std::unordered_map<std::string, std::function<std::shared_ptr<Geometry>()>> geomMakeFunctions =
{
    { "Sphere", []() { return std::make_shared<Sphere>(); } },
    { "Plane", []() { return std::make_shared<Sphere>(); } },
    { "OrientedBox", []() { return std::make_shared<OrientedBox>(); } },
    { "Capsule", []() { return std::make_shared<Sphere>(); } },
    { "Cylinder", []() { return std::make_shared<Sphere>(); } }
};

///
/// \brief Create a analytical visual scene object that and add it to the scene
///
std::shared_ptr<SceneObject>
createVisualAnalyticalSceneObject(std::string            type,
                                  std::shared_ptr<Scene> scene,
                                  const std::string&     objName,
                                  const Vec3d            scale /* = 1.*/,
                                  const Vec3d            t /*= Vec3d(0., 0., 0.)*/)
{
    CHECK(scene != nullptr) << "Error: Scene object is not valid!";
    CHECK(!objName.empty()) << "Error: Name is empty!";

    if (geomMakeFunctions.count(type) == 0)
    {
        LOG(WARNING) << "Scene object geometry type " << type << " is not analytical!";
        return nullptr;
    }
    std::shared_ptr<Geometry> geom = geomMakeFunctions[type]();

    geom->scale(scale, Geometry::TransformType::ApplyToData);
    geom->translate(t, Geometry::TransformType::ApplyToData);

    auto sceneObj = std::make_shared<SceneObject>(objName);
    sceneObj->setVisualGeometry(geom);
    scene->addSceneObject(sceneObj);

    return sceneObj;
}

///
/// \brief Create a analytical colliding scene object that and add it to the scene
///
std::shared_ptr<CollidingObject>
createCollidingAnalyticalSceneObject(std::string            type,
                                     std::shared_ptr<Scene> scene,
                                     const std::string&     objName,
                                     const Vec3d            scale /*= 1.*/,
                                     const Vec3d            t /*= Vec3d(0., 0., 0.)*/)
{
    CHECK(scene != nullptr) << "Scene is not valid!";
    CHECK(!objName.empty()) << "Name is empty!";

    if (geomMakeFunctions.count(type) == 0)
    {
        LOG(WARNING) << "Scene object geometry type " << type << " is not analytical!";
        return nullptr;
    }
    std::shared_ptr<Geometry> geom = geomMakeFunctions[type]();

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
    CHECK(scene != nullptr) << "Error: Scene object supplied is not valid!";
    CHECK(!fileName.empty()) << "Error: Name is empty!";

    auto mesh = MeshIO::read(fileName);
    auto SurfaceMesh = std::dynamic_pointer_cast<imstk::SurfaceMesh>(mesh);

    // Create object and add to scene
    auto meshSceneObject = std::make_shared<SceneObject>("meshObject");
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
    CHECK(dynaModel != nullptr) << "Dynamic model object supplied is not valid!";

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

std::shared_ptr<Graph>
getMeshGraph(std::shared_ptr<PointSet> m)
{
    LOG(WARNING) << "Note: The graph of a point set has no edges";

    return std::make_shared<Graph>(m->getNumVertices());
}

std::shared_ptr<Graph>
getMeshGraph(std::shared_ptr<SurfaceMesh> m)
{
    auto gMesh = std::make_shared<Graph>(m->getNumVertices());
    for (auto tri : *m->getTriangleIndices())
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
    for (auto tet : *m->getTetrahedraIndices())
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
