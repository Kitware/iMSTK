/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkLevelSetCH.h"

#include "imstkCollider.h"
#include "imstkCollisionData.h"
#include "imstkImageData.h"
#include "imstkLevelSetModel.h"
#include "imstkPbdBody.h"
#include "imstkPbdMethod.h"

namespace imstk
{
LevelSetCH::LevelSetCH()
{
    setKernel(m_kernelSize, m_kernelSigma);
}

LevelSetCH::~LevelSetCH()
{
    if (m_kernelWeights != nullptr)
    {
        delete[] m_kernelWeights;
    }
}

void
LevelSetCH::setInputLvlSetObj(std::shared_ptr<LevelSetModel> levelSetObject, std::shared_ptr<Collider> levelSetCollider)
{
    m_levelSetObject   = levelSetObject;
    m_levelSetCollider = levelSetCollider;
}

void
LevelSetCH::setInputRigidObj(std::shared_ptr<PbdMethod> rigidPhysics, std::shared_ptr<Collider> rigidCollider)
{
    m_rigidPhysics  = rigidPhysics;
    m_rigidCollider = rigidCollider;
}

bool
LevelSetCH::initialize()
{
    CHECK(m_rigidPhysics != nullptr) << "PbdMethod for rigid body is required.";
    CHECK(m_rigidCollider != nullptr) << "Collider for rigid body is required.";
    m_rigidPbdBody = m_rigidPhysics->getPbdBody();
    maskAllPoints();
    return true;
}

std::shared_ptr<Geometry>
LevelSetCH::getCollidingGeometryA()
{
    return m_levelSetCollider->getGeometry();
}

std::shared_ptr<Geometry>
LevelSetCH::getCollidingGeometryB()
{
    return m_rigidCollider->getGeometry();
}

/*
std::shared_ptr<LevelSetModel>
LevelSetCH::getLvlSetObj()
{
    return m_levelSetObject;
}

std::shared_ptr<PbdMethod>
LevelSetCH::getRigidObj()
{
    m_rigidObject;
}
*/

void
LevelSetCH::setKernel(const int size, const double sigma)
{
    m_kernelSize  = size;
    m_kernelSigma = sigma;
    if (size % 2 == 0)
    {
        LOG(WARNING) << "LevelSetCH kernel size must be odd, increasing by 1";
        m_kernelSize++;
    }
    if (m_kernelWeights != nullptr)
    {
        delete[] m_kernelWeights;
    }
    m_kernelWeights = new double[size * size * size];

    const double invDiv   = 1.0 / (2.0 * sigma * sigma);
    const int    halfSize = static_cast<int>(size * 0.5);
    int          i = 0;
    for (int z = -halfSize; z < halfSize + 1; z++)
    {
        for (int y = -halfSize; y < halfSize + 1; y++)
        {
            for (int x = -halfSize; x < halfSize + 1; x++)
            {
                const double dist = Vec3i(x, y, z).cast<double>().norm();
                m_kernelWeights[i++] = std::exp(-dist * invDiv);
            }
        }
    }
}

void
LevelSetCH::handle(
    const std::vector<CollisionElement>& elementsA,
    const std::vector<CollisionElement>& elementsB)
{
    if (m_levelSetObject == nullptr || m_rigidPbdBody == nullptr)
    {
        return;
    }

    std::shared_ptr<ImageData> grid = std::dynamic_pointer_cast<SignedDistanceField>(m_levelSetObject->getModelGeometry())->getImage();

    if (grid == nullptr)
    {
        LOG(FATAL) << "Error: level set model geometry is not ImageData";
        return;
    }

    //const Vec3i& dim = grid->getDimensions();
    const Vec3d& invSpacing = grid->getInvSpacing();
    const Vec3d& origin     = grid->getOrigin();

    // LevelSetCH requires both sides
    if (elementsA.size() != elementsB.size())
    {
        return;
    }

    if (m_useProportionalForce)
    {
        // Apply impulses at points of contacts
        for (size_t i = 0; i < elementsA.size(); i++)
        {
            const CollisionElement& lsmContactElement = elementsA[i];
            const CollisionElement& pbdContactElement = elementsB[i];

            if (lsmContactElement.m_type != CollisionElementType::PointDirection
                || pbdContactElement.m_type != CollisionElementType::PointIndexDirection)
            {
                continue;
            }

            // If the point is in the mask, let it apply impulses
            if (m_ptIdMask.count(pbdContactElement.m_element.m_PointIndexDirectionElement.ptIndex) != 0)
            {
                const Vec3d& pos    = lsmContactElement.m_element.m_PointDirectionElement.pt;
                const Vec3d& normal = lsmContactElement.m_element.m_PointDirectionElement.dir;
                const Vec3i  coord  = (pos - origin).cwiseProduct(invSpacing).cast<int>();

                // Scale the applied impulse by the normal force
                const double fN = normal.normalized().dot(m_rigidPbdBody->externalForce) / m_rigidPbdBody->externalForce.norm();
                const double S  = std::max(fN, 0.0) * m_velocityScaling;

                const int halfSize = static_cast<int>(m_kernelSize * 0.5);
                int       j = 0;
                for (int z = -halfSize; z < halfSize + 1; z++)
                {
                    for (int y = -halfSize; y < halfSize + 1; y++)
                    {
                        for (int x = -halfSize; x < halfSize + 1; x++)
                        {
                            const Vec3i fCoord = coord + Vec3i(x, y, z);
                            m_levelSetObject->addImpulse(fCoord, S * m_kernelWeights[j++]);
                        }
                    }
                }
            }
        }
    }
    else
    {
        // Apply impulses at points of contacts
        for (size_t i = 0; i < elementsA.size(); i++)
        {
            const CollisionElement& lsmContactElement = elementsA[i];
            const CollisionElement& pbdContactElement = elementsB[i];

            if (lsmContactElement.m_type != CollisionElementType::PointDirection
                || pbdContactElement.m_type != CollisionElementType::PointIndexDirection)
            {
                continue;
            }

            // If the point is in the mask, let it apply impulses
            if (m_ptIdMask.count(pbdContactElement.m_element.m_PointIndexDirectionElement.ptIndex) != 0)
            {
                const Vec3d& pos = lsmContactElement.m_element.m_PointDirectionElement.pt;
                //const Vec3d& normal = pdColData[i].dirAtoB;
                const Vec3i  coord = (pos - origin).cwiseProduct(invSpacing).cast<int>();
                const double S     = m_velocityScaling;

                const int halfSize = static_cast<int>(m_kernelSize * 0.5);
                int       j = 0;
                for (int z = -halfSize; z < halfSize + 1; z++)
                {
                    for (int y = -halfSize; y < halfSize + 1; y++)
                    {
                        for (int x = -halfSize; x < halfSize + 1; x++)
                        {
                            const Vec3i fCoord = coord + Vec3i(x, y, z);
                            m_levelSetObject->addImpulse(fCoord, S * m_kernelWeights[j++]);
                        }
                    }
                }
            }
        }
    }
}

void
LevelSetCH::maskAllPoints()
{
    if (auto pointSet = std::dynamic_pointer_cast<PointSet>(m_rigidCollider->getGeometry()))
    {
        for (int i = 0; i < pointSet->getNumVertices(); i++)
        {
            m_ptIdMask.insert(i);
        }
    }
}
} // namespace imstk
