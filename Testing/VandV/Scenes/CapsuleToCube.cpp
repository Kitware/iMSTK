/* This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.*/

#include "CapsuleToCube.h"

#include "imstkCapsule.h"
#include "imstkGeometryUtilities.h"
#include "imstkPbdModel.h"
#include "imstkPbdObject.h"
#include "imstkPbdObjectController.h"
#include "imstkPbdObjectCollision.h"
#include "imstkObjectControllerGhost.h"
#include "imstkTetrahedralMesh.h"

#include "imstkCamera.h"
#include "imstkRenderMaterial.h"
#include "imstkVisualModel.h"

namespace imstk
{
void
CapsuleToCubeScene::Configuration::toString(std::ostream& str) const
{
    ProgrammableScene::Configuration::toString(str);
    switch (type)
    {
    case CapsuleToCubeScene::Type::DeformableCube:
        str << "\ttype: DeformableCube\n";
        break;
    case CapsuleToCubeScene::Type::GraspDeformableCube:
        str << "\ttype: GraspDeformableCube\n";
        break;
    case CapsuleToCubeScene::Type::RigidCube:
        str << "\ttype: RigidCube\n";
    }
}

void
CapsuleToCubeScene::Configuration::addConstraint(PbdModelConfig::ConstraintGenType& constraintType, double& constraintValue)
{
    constraintTypes.push_back(constraintType);
    constraintValues.push_back(constraintValue);
}

static std::shared_ptr<CollidingObject>
makeCube(
    const std::string&                       name,
    const CapsuleToCubeScene::Configuration& cfg)
{
    auto cube = std::make_shared<CollidingObject>(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tetMesh  = GeometryUtils::toTetGrid(Vec3d::Zero(), cfg.size, cfg.divisions);
    std::shared_ptr<SurfaceMesh>     surfMesh = tetMesh->extractSurfaceMesh();

    // Setup the Object
    cube->setCollidingGeometry(surfMesh);
    cube->setVisualGeometry(surfMesh);
    cube->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);

    return cube;
}

static std::shared_ptr<PbdObject>
makePbdCube(
    const std::string&                       name,
    std::shared_ptr<PbdModel>                model,
    const CapsuleToCubeScene::Configuration& cfg)
{
    auto cube = std::make_shared<PbdObject>(name);

    // Setup the Geometry
    std::shared_ptr<TetrahedralMesh> tetMesh  = GeometryUtils::toTetGrid(Vec3d::Zero(), cfg.size, cfg.divisions);
    std::shared_ptr<SurfaceMesh>     surfMesh = tetMesh->extractSurfaceMesh();

    // Setup the Object
    cube->setPhysicsGeometry(surfMesh);
    cube->setCollidingGeometry(surfMesh);
    cube->setVisualGeometry(surfMesh);
    cube->getVisualModel(0)->getRenderMaterial()->setDisplayMode(RenderMaterial::DisplayMode::Wireframe);
    cube->setDynamicalModel(model);
    cube->getPbdBody()->uniformMassValue = cfg.uniformMassValue;
    if (cfg.constraintTypes.empty())
    {
        // Use dihedral+distance constraints, worse results. More performant (can use larger mesh)
        model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Dihedral, 1000.0,
        cube->getPbdBody()->bodyHandle);
        model->getConfig()->enableConstraint(PbdModelConfig::ConstraintGenType::Distance, 500.0,
        cube->getPbdBody()->bodyHandle);
    }
    else
    {
        for (int i = 0; i < cfg.constraintTypes.size(); i++)
        {
            model->getConfig()->enableConstraint(cfg.constraintTypes[i], cfg.constraintValues[i], cube->getPbdBody()->bodyHandle);
        }
    }
    // Set bottom verts to be static
    std::shared_ptr<VecDataArray<double, 3>> vertices = surfMesh->getVertexPositions();
    for (int i = 0; i < surfMesh->getNumVertices(); i++)
    {
        const Vec3d& pos = (*vertices)[i];
        if (pos[1] <= 0 - cfg.size[1] * 0.5)
        {
            cube->getPbdBody()->fixedNodeIds.push_back(i);
        }
    }

    return cube;
}

static std::shared_ptr<PbdObject>
makeCapsuleToolObj(std::shared_ptr<PbdModel> model)
{
    auto toolGeometry = std::make_shared<Capsule>();
    toolGeometry->setRadius(0.1);
    toolGeometry->setLength(1);
    toolGeometry->setPosition(Vec3d(0.0, 0.0, 0.0));
    toolGeometry->setRotation(Vec3d(0.0, 0.0, 1.0), 0.785);

    auto toolObj = std::make_shared<PbdObject>("Capsule");

    // Create the object
    toolObj->setVisualGeometry(toolGeometry);
    toolObj->setPhysicsGeometry(toolGeometry);
    toolObj->setCollidingGeometry(toolGeometry);
    toolObj->setDynamicalModel(model);
    toolObj->getPbdBody()->setRigid(
      Vec3d(5.0, 5.0, 0),
      1.0,
      Quatd::Identity(),
      Mat3d::Identity() * 1000000);

    toolObj->getVisualModel(0)->getRenderMaterial()->setOpacity(0.5);

    // Add a component for controlling via another device
    auto controller = toolObj->addComponent<PbdObjectController>();
    controller->setControlledObject(toolObj);
    // controller->setTranslationScaling(50.0);
    controller->setLinearKs(5000.0);
    controller->setAngularKs(1000.0);
    controller->setUseCritDamping(true);
    controller->setForceScaling(0.001);
    controller->setSmoothingKernelSize(15);
    controller->setUseForceSmoothening(true);

    // Add extra component to tool for the ghost
    auto controllerGhost = toolObj->addComponent<ObjectControllerGhost>();
    controllerGhost->setController(controller);

    return toolObj;
}

CapsuleToCubeScene::CapsuleToCubeScene()
{
    CapsuleToCubeScene::Configuration cfg;
    cfg.type = CapsuleToCubeScene::Type::DeformableCube;
    setConfiguration(cfg);
}

CapsuleToCubeScene::CapsuleToCubeScene(const CapsuleToCubeScene::Configuration& cfg)
{
    setConfiguration(cfg);
}

void
CapsuleToCubeScene::setConfiguration(const Configuration& cfg)
{
    ProgrammableScene::setConfiguration(cfg);
    m_config = cfg;
}

bool
CapsuleToCubeScene::setupScene(double sampleTime)
{
    std::shared_ptr<PbdObject> capsule = makeCapsuleToolObj(m_pbdModel);
    m_scene->addSceneObject(capsule);

    if (m_config.type == Type::RigidCube)
    {
        auto cube = makeCube("Cube", m_config);
        m_scene->addSceneObject(cube);
        auto pbdToolCollision = std::make_shared<PbdObjectCollision>(capsule, cube);
        pbdToolCollision->setRigidBodyCompliance(0.0001); // Helps with smoothness
        m_scene->addInteraction(pbdToolCollision);
    }
    else
    {
        auto cube = makePbdCube("Cube", m_pbdModel, m_config);
        m_scene->addSceneObject(cube);
        auto pbdToolCollision = std::make_shared<PbdObjectCollision>(capsule, cube);
        pbdToolCollision->setRigidBodyCompliance(0.0001); // Helps with smoothness
        m_scene->addInteraction(pbdToolCollision);
        if (m_config.type == Type::GraspDeformableCube)
        {
            auto grasping = std::make_shared<PbdObjectGrasping>(cube, capsule);
            grasping->setStiffness(0.3);
            grasping->setName("PbdObjectGrasping");
            m_scene->addInteraction(grasping);
        }
    }

    // Setup progammed movement
    auto controller   = capsule->getComponent<SceneObjectController>();
    auto deviceClient = std::make_shared<ProgrammableClient>();
    deviceClient->setDeltaTime(getTimeStep());

    double topOfCube       = (m_config.size[1] * .5) + .5; // Bottom of capsule adjacnet to cube
    double rightSideOfCube = (m_config.size[1] * .5);      // Side of capsule adjacnet to cube
    switch (m_config.type)
    {
    case Type::RigidCube:
    {
        double x = -0.08;
        double z = -0.20;
        deviceClient->addLinearMovement(Vec3d(x, 2.50, z),
        Vec3d(x, 0.90, z),
        0, 5);
        deviceClient->addLinearMovement(Vec3d(x, 0.90, z),
        Vec3d(x, 1.20, z),
        7, 2);
        deviceClient->addLinearMovement(Vec3d(x, 1.20, z),
        Vec3d(x, 0.90, z),
        10, 2);
        deviceClient->addLinearMovement(Vec3d(x, 0.90, z),
        Vec3d(x, 1.20, z),
        13, 2);
        deviceClient->addLinearMovement(Vec3d(x, 1.20, z),
        Vec3d(x, 0.90, z),
        16, 2);
        deviceClient->addLinearMovement(Vec3d(x, 0.90, z),
        Vec3d(x, 2.50, z),
        19, 5);
        break;
    }
    case Type::DeformableCube:
    {
        deviceClient->addLinearMovement(Vec3d(0.25, topOfCube + 1.5, 0),
        Vec3d(0.25, topOfCube - 0.01, 0),
        0, 5);
        deviceClient->addCircularMovement(Vec3d(0.25, topOfCube - 0.05, 0.0),
        Vec3d(0.0, topOfCube - 0.01, 0.0),
        6, 5);
        deviceClient->addLinearMovement(Vec3d(0.25, topOfCube - 0.01, 0.0),
        Vec3d(0.25, topOfCube + 1.5, 0),
        12, 5);
        break;
    }
    case Type::GraspDeformableCube:
    {
        deviceClient->addLinearMovement(Vec3d(rightSideOfCube, topOfCube + 1, 0),
        Vec3d(rightSideOfCube, topOfCube - 0.01, 0),
        0, 3);
        deviceClient->addLinearMovement(Vec3d(rightSideOfCube, topOfCube - 0.01, 0),
        Vec3d(rightSideOfCube, topOfCube + 1, 0),
        7, 5);
        deviceClient->addGrasping(capsule,
        std::dynamic_pointer_cast<PbdObjectGrasping>(m_scene->getSceneObject("PbdObjectGrasping")),
        6, 5);
        break;
    }
    }
    // deviceClient->setDistanceToTraverse(Vec3d(-4, -4, 0));
    controller->setDevice(deviceClient);
    m_scene->addControl(controller);

    m_clients.push_back(deviceClient);

    m_duration = deviceClient->getTotalDuration();

    return ProgrammableScene::setupScene(sampleTime);
}

bool
CapsuleToCubeScene::setupView()
{
    m_scene->getActiveCamera()->setPosition(0.0, 3.0, 5.0);
    m_scene->getActiveCamera()->setFocalPoint(0.0, 1.0, 0.0);
    m_scene->getActiveCamera()->setViewUp(Vec3d(0.0, 1.0, 0.0));

    return true;
}
} // namespace imstk
