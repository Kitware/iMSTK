#include "imstkPbdObject.h"
#include "imstkGeometryMap.h"

#include <g3log/g3log.hpp>

namespace imstk
{

bool
PbdObject::initialize(int nCons, ...)
{
    std::shared_ptr<Mesh> mesh = std::dynamic_pointer_cast<Mesh>(m_physicsGeometry);
    if (mesh == nullptr)
    {
        LOG(WARNING) << "Physics geometry is not a mesh!";
        return false;
    }

    m_pbdModel = std::dynamic_pointer_cast<PbdModel>(m_dynamicalModel);
    if (m_pbdModel == nullptr)
    {
        LOG(WARNING) << "Dynamic model set is not of expected type (PbdModel)!";
        return false;
    }

    m_pbdModel->setModelGeometry(mesh);
    m_pbdModel->initialize();

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
                m_pbdModel->initializeFEMConstraints(PbdFEMConstraint::MaterialType::Corotation);
            }
            else if (strncmp("NeoHookean",&s[pos],len)==0)
            {
                LOG(INFO) << "Creating Neohookean constraints";
                m_pbdModel->initializeFEMConstraints(PbdFEMConstraint::MaterialType::NeoHookean);
            }
            else if (strncmp("Stvk",&s[pos],len)==0)
            {
                LOG(INFO) << "Creating StVenant-Kirchhoff constraints";
                m_pbdModel->initializeFEMConstraints(PbdFEMConstraint::MaterialType::StVK);
            }
            else
            { // default
                m_pbdModel->initializeFEMConstraints(PbdFEMConstraint::MaterialType::StVK);
            }

            float YoungModulus, PoissonRatio;
            sscanf(&s[pos+len+1], "%f %f", &YoungModulus, &PoissonRatio);
            m_pbdModel->computeLameConstants(YoungModulus, PoissonRatio);
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

    m_pbdModel->setUniformMass(va_arg(args, double));

    if (nCons > 0)
    {
        char* gstring = va_arg(args,char*);
        float x,y,z;
        sscanf(gstring,"%f %f %f", &x, &y, &z);
        Vec3d g(x,y,z);
        m_pbdModel->setGravity(g);
        m_pbdModel->setTimeStep(va_arg(args, double));
        char *s = va_arg(args,char*);

        if (strlen(s) > 0)
        {
            while (1)
            {
                int idx = atoi(s);
                m_pbdModel->setFixedPoint(idx - 1);
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
        m_pbdModel->setMaxNumIterations(va_arg(args,int));
    }

    if (m_physicsToCollidingGeomMap && m_collidingGeometry)
    {
        m_pbdModel->setProximity(va_arg(args,double));
        m_pbdModel->setContactStiffness(va_arg(args,double));
    }

    m_pbdModel->setNumDegreeOfFreedom(m_pbdModel->getModelGeometry()->getNumVertices() * 3);

    return true;
}

void
PbdObject::integratePosition()
{
    if (m_pbdModel && m_pbdModel->hasConstraints())
    {
        m_pbdModel->integratePosition();
    }
}

void
PbdObject::updateVelocity()
{
    if (m_pbdModel && m_pbdModel->hasConstraints())
    {
        m_pbdModel->updateVelocity();
    }
}

void
PbdObject::solveConstraints()
{
    if (m_pbdModel && m_pbdModel->hasConstraints())
    {
        m_pbdModel->projectConstraints();
    }
}

} //imstk