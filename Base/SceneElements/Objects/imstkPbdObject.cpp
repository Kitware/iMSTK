#include "imstkPbdObject.h"

namespace imstk
{

std::shared_ptr<Geometry>
PbdObject::getPhysicsGeometry() const
{
    return m_physicsGeometry;
}

void
PbdObject::setPhysicsGeometry(std::shared_ptr<Geometry> geometry)
{
    m_physicsGeometry = geometry;
}

std::shared_ptr<GeometryMap>
PbdObject::getPhysicsToCollidingMap() const
{
    return m_physicsToCollidingGeomMap;
}

void
PbdObject::setPhysicsToCollidingMap(std::shared_ptr<GeometryMap> map)
{
    m_physicsToCollidingGeomMap = map;
}

std::shared_ptr<GeometryMap>
PbdObject::getPhysicsToVisualMap() const
{
    return m_physicsToVisualGeomMap;
}

void
PbdObject::setPhysicsToVisualMap(std::shared_ptr<GeometryMap> map)
{
    m_physicsToVisualGeomMap = map;
}

std::shared_ptr<PositionBasedModel>
PbdObject::getDynamicalModel() const
{
    return m_pbdModel;
}

void
PbdObject::setDynamicalModel(std::shared_ptr<PositionBasedModel> dynaModel)
{
    m_pbdModel = dynaModel;
}

size_t
PbdObject::getNumOfDOF() const
{
    return numDOF;
}

void PbdObject::init()
{
    m_pbdModel = std::make_shared<PositionBasedModel>();
    auto mesh = std::static_pointer_cast<Mesh>(m_physicsGeometry);
    m_pbdModel->setModelGeometry(mesh.get());
    m_pbdModel->setElasticModulus(100.0, 0.3);
    Vec3d g(0.0,0.0,-9.8);
    auto state = m_pbdModel->getState();
    state->setUniformMass(1.0);
    state->setGravity(g);
    state->setTimeStep(1e-3);
    int fixedPoints[] = {51,52,103,104,155,156,207,208};
    for (int i = 0; i < sizeof(fixedPoints)/sizeof(int); ++i) {
        state->setFixedPoint(fixedPoints[i]-1);
    }
    m_pbdModel->initConstraints(PbdConstraint::Type::FEMTet);
//    m_pbdModel->initConstraints(PbdConstraint::Type::Distance);
//    m_pbdModel->initConstraints(PbdConstraint::Type::Volume);
    m_pbdModel->setNumberOfInterations(20);
}

}
