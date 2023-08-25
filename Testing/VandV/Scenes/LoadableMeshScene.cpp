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
