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

std::shared_ptr<GeometryMap> PbdObject::getCollidingToPhysicsMap() const
{
    return m_collidingToPhysicsGeomMap;
}

void PbdObject::setCollidingToPhysicsMap(std::shared_ptr<GeometryMap> map)
{
    m_collidingToPhysicsGeomMap = map;
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

void PbdObject::init(int fem, ...)
{
    va_list args;
    va_start(args, fem);

    m_pbdModel = std::make_shared<PositionBasedModel>();
    auto mesh = std::static_pointer_cast<Mesh>(m_physicsGeometry);
    m_pbdModel->setModelGeometry(mesh.get());
    auto state = m_pbdModel->getState();
    char* gstring = va_arg(args,char*);
    float x,y,z;
    sscanf(gstring,"%f %f %f", &x, &y, &z);
    Vec3d g(x,y,z);
    state->setGravity(g);
    state->setUniformMass(va_arg(args,double));
    state->setTimeStep(va_arg(args,double));
    char *s = va_arg(args,char*);
    while (1)
    {
        int idx = atoi(s);
        state->setFixedPoint(idx-1);
        while (*s != ' ' && *s != '\0') ++s;
        if (*s == '\0') break; else ++s;
    }

    s = va_arg(args,char*);
    int pos = 0;
    while (1)
    {
        int len = 0;
        while (s[pos+len] != ' ' && s[pos+len] != '\0') {
            ++len;
        }

        if (strncmp("FEM",&s[pos],len)==0) {
            m_pbdModel->initConstraints(PbdConstraint::Type::FEMTet);
        }
        else if (strncmp("Volume",&s[pos],len)==0) {
            m_pbdModel->initConstraints(PbdConstraint::Type::Volume);
        }
        else if (strncmp("Distance",&s[pos],len)==0) {
            m_pbdModel->initConstraints(PbdConstraint::Type::Distance);
        }
        else if (strncmp("Area",&s[pos],len)==0) {
            m_pbdModel->initConstraints(PbdConstraint::Type::Area);
        }
        else if (strncmp("Dihedral",&s[pos],len)==0) {
            m_pbdModel->initConstraints(PbdConstraint::Type::Dihedral);
        }

        if (s[pos+len] == '\0') break; else pos += len+1;
    }

    m_pbdModel->setNumberOfInterations(va_arg(args,int));
    if (fem)
    {
        double YoungModulus = va_arg(args,double);
        double PoissonRatio = va_arg(args,double);
        m_pbdModel->setElasticModulus(YoungModulus, PoissonRatio);
    }
}

}
