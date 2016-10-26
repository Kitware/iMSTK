#include "imstkPbdObject.h"

#include "imstkGeometryMap.h"

#include <g3log/g3log.hpp>

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
    m_pbdModel = std::make_shared<PositionBasedDynamicsModel>();
    auto mesh = std::static_pointer_cast<Mesh>(m_physicsGeometry);
    m_pbdModel->setModelGeometry(mesh);
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

std::shared_ptr<PositionBasedDynamicsModel>
PbdObject::getDynamicalModel() const
{
    return m_pbdModel;
}

void
PbdObject::setDynamicalModel(std::shared_ptr<PositionBasedDynamicsModel> dynaModel)
{
    m_pbdModel = dynaModel;
}

size_t
PbdObject::getNumOfDOF() const
{
    return numDOF;
}

void
PbdObject::init(int nCons, ...)
{
    auto state = m_pbdModel->getState();

    va_list args;
    va_start(args, nCons);
    for(int i = 0; i < nCons; ++i)
    {
        char* s = va_arg(args,char*);
        int len = 0;
        while (s[len] != ' ' && s[len] != '\0')
        {
            ++len;
        }

        if (strncmp("FEM",&s[0],len)==0)
        {
            int pos = len+1;
            len = 0;
            while (s[pos+len] != ' ' && s[pos+len] != '\0')
            {
                ++len;
            }

            if (strncmp("Corotation",&s[pos],len)==0)
            {
                LOG(INFO) << "Creating Corotation constraints";
                m_pbdModel->initializeFEMConstraints(FEMConstraint::MaterialType::Corotation);
            }
            else if (strncmp("NeoHookean",&s[pos],len)==0)
            {
                LOG(INFO) << "Creating Neohookean constraints";
                m_pbdModel->initializeFEMConstraints(FEMConstraint::MaterialType::NeoHookean);
            }
            else if (strncmp("Stvk",&s[pos],len)==0)
            {
                LOG(INFO) << "Creating StVenant-Kirchhoff constraints";
                m_pbdModel->initializeFEMConstraints(FEMConstraint::MaterialType::StVK);
            }
            else
            { // default
                m_pbdModel->initializeFEMConstraints(FEMConstraint::MaterialType::StVK);
            }

            float YoungModulus, PoissonRatio;
            sscanf(&s[pos+len+1], "%f %f", &YoungModulus, &PoissonRatio);
            m_pbdModel->setElasticModulus(YoungModulus, PoissonRatio);
        }
        else if (strncmp("Volume",&s[0],len)==0)
        {
            float stiffness;
            sscanf(&s[len+1], "%f", &stiffness);
            LOG(INFO) << "Creating Volume constraints " << stiffness ;
            m_pbdModel->initializeVolumeConstraints(stiffness);
        }
        else if (strncmp("Distance",&s[0],len)==0)
        {
            float stiffness;
            sscanf(&s[len+1], "%f", &stiffness);
            LOG(INFO) << "Creating Distance constraints " << stiffness;
            m_pbdModel->initializeDistanceConstraints(stiffness);
        }
        else if (strncmp("Area",&s[0],len)==0)
        {
            float stiffness;
            sscanf(&s[len+1], "%f", &stiffness);
            LOG(INFO) << "Creating Area constraints " << stiffness;
            m_pbdModel->initializeAreaConstraints(stiffness);
        }
        else if (strncmp("Dihedral",&s[0],len)==0)
        {
            float stiffness;
            sscanf(&s[len+1], "%f", &stiffness);
            LOG(INFO) << "Creating Dihedral constraints " << stiffness;
            m_pbdModel->initializeDihedralConstraints(stiffness);
        }
        else
        {
            exit(0);
        }
    }

    state->setUniformMass(va_arg(args,double));

    if (nCons > 0)
    {
        char* gstring = va_arg(args,char*);
        float x,y,z;
        sscanf(gstring,"%f %f %f", &x, &y, &z);
        Vec3d g(x,y,z);
        state->setGravity(g);
        state->setTimeStep(va_arg(args,double));
        char *s = va_arg(args,char*);

        if (strlen(s) > 0)
        {
            while (1)
            {
                int idx = atoi(s);
                state->setFixedPoint(idx-1);
                while (*s != ' ' && *s != '\0')
                {
                    ++s;
                }

                if (*s == '\0')
                {
                    break;
                }
                else
                {
                    ++s;
                }
            }
        }
        m_pbdModel->setNumberOfInterations(va_arg(args,int));
    }

    if (m_physicsToCollidingGeomMap && m_collidingGeometry)
    {
        m_pbdModel->setProximity(va_arg(args,double));
        m_pbdModel->setContactStiffness(va_arg(args,double));
    }
}

void PbdObject::integratePosition()
{
    if (m_pbdModel && m_pbdModel->hasConstraints())
    {
        m_pbdModel->getState()->integratePosition();
    }
}

void PbdObject::integrateVelocity()
{
    if (m_pbdModel && m_pbdModel->hasConstraints())
    {
        m_pbdModel->getState()->integrateVelocity();
    }
}

void PbdObject::updateGeometry()
{
    if (m_pbdModel && m_pbdModel->hasConstraints())
    {
        m_pbdModel->updatePhysicsGeometry();
    }
}

void PbdObject::constraintProjection()
{
    if (m_pbdModel && m_pbdModel->hasConstraints())
    {
        m_pbdModel->constraintProjection();
    }
}

void PbdObject::applyPhysicsToColliding()
{
    if (m_physicsToCollidingGeomMap && m_collidingGeometry)
    {
        m_physicsToCollidingGeomMap->apply();
    }
}

void PbdObject::applyPhysicsToVisual()
{
    if (m_physicsToVisualGeomMap && m_visualGeometry)
    {
        m_physicsToVisualGeomMap->apply();
    }
}

} //imstk