/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.*/

#include "LoadableMeshScene.h"

#include "imstkPlane.h"
#include "imstkDynamicObject.h"
#include "imstkGeometryUtilities.h"
#include "imstkNew.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdObjectCollision.h"
#include "imstkMeshIO.h"
#include "imstkObjectControllerGhost.h"
#include "imstkTetrahedralMesh.h"

#include "imstkCamera.h"
#include "imstkRenderMaterial.h"
#include "imstkVisualModel.h"

namespace imstk
{
std::shared_ptr<SurfaceMesh>
createCollidingSurfaceMesh()
{
    imstkNew<VecDataArray<double, 3>> verticesPtr;
    VecDataArray<double, 3>&          vertices = *verticesPtr.get();
    int                               nSides   = 5;
    double                            width    = 40.0;
    double                            height   = 40.0;
    int                               nRows    = 2;
    int                               nCols    = 2;
    vertices.resize(nRows * nCols * nSides);
    const double dy = width / static_cast<double>(nCols - 1);
    const double dx = height / static_cast<double>(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double y = static_cast<double>(dy * j);
            const double x = static_cast<double>(dx * i);
            vertices[i * nCols + j] = Vec3d(x - 20, -10.0, y - 20);
        }
    }

    // c. Add connectivity data
    std::shared_ptr<VecDataArray<int, 3>> trianglesPtr = std::make_shared<VecDataArray<int, 3>>();
    VecDataArray<int, 3>&                 triangles    = *trianglesPtr;
    for (int i = 0; i < nRows - 1; ++i)
    {
        for (int j = 0; j < nCols - 1; j++)
        {
            triangles.push_back(Vec3i(i * nCols + j, i * nCols + j + 1, (i + 1) * nCols + j));
            triangles.push_back(Vec3i((i + 1) * nCols + j + 1, (i + 1) * nCols + j, i * nCols + j + 1));
        }
    }

    int nPointPerSide = nRows * nCols;
    //sidewalls 1 and 2 of box
    width  = 10.0;
    height = 40.0;
    nRows  = 2;
    nCols  = 2;
    const double dz  = width / static_cast<double>(nCols - 1);
    const double dx1 = height / static_cast<double>(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double z = static_cast<double>(dz * j);
            const double x = static_cast<double>(dx1 * i);
            vertices[(nPointPerSide) + i * nCols + j]     = Vec3d(x - 20, z - 10.0, 20);
            vertices[(nPointPerSide * 2) + i * nCols + j] = Vec3d(x - 20, z - 10.0, -20);
        }
    }

    // c. Add connectivity data
    for (int i = 0; i < nRows - 1; ++i)
    {
        for (int j = 0; j < nCols - 1; j++)
        {
            triangles.push_back(Vec3i((nPointPerSide) + i * nCols + j, (nPointPerSide) + i * nCols + j + 1, (nPointPerSide) + (i + 1) * nCols + j));
            triangles.push_back(Vec3i((nPointPerSide) + (i + 1) * nCols + j + 1, (nPointPerSide) + (i + 1) * nCols + j, (nPointPerSide) + i * nCols + j + 1));

            triangles.push_back(Vec3i((nPointPerSide * 2) + i * nCols + j + 1, (nPointPerSide * 2) + i * nCols + j, (nPointPerSide * 2) + (i + 1) * nCols + j));
            triangles.push_back(Vec3i((nPointPerSide * 2) + (i + 1) * nCols + j, (nPointPerSide * 2) + (i + 1) * nCols + j + 1, (nPointPerSide * 2) + i * nCols + j + 1));
        }
    }

    //sidewalls 3 and 4 of box
    width  = 10.0;
    height = 40.0;
    nRows  = 2;
    nCols  = 2;
    const double dz1 = width / static_cast<double>(nCols - 1);
    const double dy1 = height / static_cast<double>(nRows - 1);
    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; j++)
        {
            const double z = static_cast<double>(dz1 * j);
            const double y = static_cast<double>(dy1 * i);
            vertices[(nPointPerSide * 3) + i * nCols + j] = Vec3d(20, z - 10.0, y - 20);
            vertices[(nPointPerSide * 4) + i * nCols + j] = Vec3d(-20, z - 10.0, y - 20);
        }
    }

    // c. Add connectivity data
    for (int i = 0; i < nRows - 1; ++i)
    {
        for (int j = 0; j < nCols - 1; j++)
        {
            triangles.push_back(Vec3i((nPointPerSide * 3) + i * nCols + j + 1, (nPointPerSide * 3) + i * nCols + j, (nPointPerSide * 3) + (i + 1) * nCols + j));
            triangles.push_back(Vec3i((nPointPerSide * 3) + (i + 1) * nCols + j, (nPointPerSide * 3) + (i + 1) * nCols + j + 1, (nPointPerSide * 3) + i * nCols + j + 1));

            triangles.push_back(Vec3i((nPointPerSide * 4) + i * nCols + j, (nPointPerSide * 4) + i * nCols + j + 1, (nPointPerSide * 4) + (i + 1) * nCols + j));
            triangles.push_back(Vec3i((nPointPerSide * 4) + (i + 1) * nCols + j + 1, (nPointPerSide * 4) + (i + 1) * nCols + j, (nPointPerSide * 4) + i * nCols + j + 1));
        }
    }

    imstkNew<SurfaceMesh> floorMeshColliding;
    floorMeshColliding->initialize(verticesPtr, trianglesPtr);
    return floorMeshColliding;
}

static std::shared_ptr<PbdObject>
createPlaneObject(std::shared_ptr<PbdModel> model)
{
    auto planeGeometry = std::make_shared<Plane>();

    auto planeObj = std::make_shared<PbdObject>("Plane");

    // Create the object
    planeObj->setVisualGeometry(planeGeometry);
    planeObj->setPhysicsGeometry(planeGeometry);
    planeObj->setDynamicalModel(model);
    planeObj->setCollidingGeometry(planeGeometry);
    planeObj->getPbdBody()->setRigid(
      Vec3d(0.0, 0.0, 0),
      1.0,
      Quatd::Identity(),
      Mat3d::Identity() * 1000000);

    planeObj->getVisualModel(0)->getRenderMaterial()->setOpacity(0.5);

    return planeObj;
}

static std::shared_ptr<PbdObject>
loadMeshObject(
    const std::string&                      name,
    std::shared_ptr<PbdModel>               model,
    const LoadableMeshScene::Configuration& cfg)
{
    auto object = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    auto        tetMesh = MeshIO::read<TetrahedralMesh>(cfg.filePath);
    const Vec3d center  = tetMesh->getCenter();

    tetMesh->translate(-center + Vec3d(0, 1, 0), Geometry::TransformType::ApplyToData);
    tetMesh->scale(2.0, Geometry::TransformType::ApplyToData);

    auto surfMesh = tetMesh->extractSurfaceMesh();

    // Setup the material
    auto material = std::make_shared<RenderMaterial>();
    material->setDisplayMode(RenderMaterial::DisplayMode::WireframeSurface);
    material->setBackFaceCulling(false);
    material->setOpacity(0.5);

    // Add a visual model to render the tet mesh
    auto visualModel = std::make_shared<VisualModel>();
    visualModel->setGeometry(tetMesh);
    visualModel->setRenderMaterial(material);

    model->getConfig()->m_gravity = Vec3d(0.0, -9.8, 0.0);

    object->addVisualModel(visualModel);
    object->setPhysicsGeometry(tetMesh);
    object->setDynamicalModel(model);
    object->setCollidingGeometry(tetMesh);

    // Gallblader is about 60g
    object->getPbdBody()->uniformMassValue = cfg.mass / tetMesh->getNumVertices();

    if (cfg.constraintTypes.empty())
    {
        model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 500.0,
            object->getPbdBody()->bodyHandle);
        model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Volume, 500.0,
            object->getPbdBody()->bodyHandle);
    }
    else
    {
        for (int i = 0; i < cfg.constraintTypes.size(); i++)
        {
            model->getConfig()->enableConstraint(cfg.constraintTypes[i], cfg.constraintValues[i], object->getPbdBody()->bodyHandle);
        }
    }

    return object;
}

LoadableMeshScene::LoadableMeshScene()
{
    LoadableMeshScene::Configuration cfg;
    setConfiguration(cfg);
}

LoadableMeshScene::LoadableMeshScene(const LoadableMeshScene::Configuration& cfg)
{
    setConfiguration(cfg);
}

void
LoadableMeshScene::setConfiguration(const Configuration& cfg)
{
    ProgrammableScene::setConfiguration(cfg);
    m_config = cfg;
}

bool
LoadableMeshScene::setupScene(double sampleTime)
{
    auto loadedObject = loadMeshObject("LoadedObject", m_pbdModel, m_config);
    m_scene->addSceneObject(loadedObject);

    auto floor = createPlaneObject(m_pbdModel);
    m_scene->addSceneObject(floor);

    // Collisions
    auto collision = std::make_shared<PbdObjectCollision>(floor, loadedObject);
    collision->setRigidBodyCompliance(0.0001); // Helps with smoothness
    m_scene->addInteraction(collision);

    auto deviceClient = std::make_shared<ProgrammableClient>();
    deviceClient->setDeltaTime(getTimeStep());

    deviceClient->addWaitCommand(0, 20);
    std::vector<int> toHold;
    toHold.push_back(0);
    deviceClient->addHoldCommand(floor, 0, 20, toHold);

    m_clients.push_back(deviceClient);

    m_duration = deviceClient->getTotalDuration();

    return ProgrammableScene::setupScene(sampleTime);
}

bool
LoadableMeshScene::setupView()
{
    m_scene->getActiveCamera()->setPosition(0.0, 1.0, 2.0);
    m_scene->getActiveCamera()->setFocalPoint(0.0, 0.0, 0.0);
    m_scene->getActiveCamera()->setViewUp(Vec3d(0.0, 1.0, 0.0));

    return true;
}
} // namespace imstk
