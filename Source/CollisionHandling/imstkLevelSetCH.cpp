/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkLevelSetCH.h"
#include "imstkCollisionData.h"
#include "imstkImageData.h"
#include "imstkLevelSetDeformableObject.h"
#include "imstkLevelSetModel.h"
#include "imstkRbdConstraint.h"
#include "imstkRigidObject2.h"

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
LevelSetCH::setInputLvlSetObj(std::shared_ptr<LevelSetDeformableObject> lvlSetObj)
{
    setInputObjectA(lvlSetObj);
}

void
LevelSetCH::setInputRigidObj(std::shared_ptr<RigidObject2> rbdObj)
{
    setInputObjectB(rbdObj);
    maskAllPoints();
}

std::shared_ptr<LevelSetDeformableObject>
LevelSetCH::getLvlSetObj()
{
    return std::dynamic_pointer_cast<LevelSetDeformableObject>(getInputObjectA());
}

std::shared_ptr<RigidObject2>
LevelSetCH::getRigidObj()
{
    return std::dynamic_pointer_cast<RigidObject2>(getInputObjectB());
}

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
    std::shared_ptr<LevelSetDeformableObject> lvlSetObj = getLvlSetObj();
    std::shared_ptr<RigidObject2>             rbdObj    = getRigidObj();

    if (lvlSetObj == nullptr || rbdObj == nullptr)
    {
        return;
    }

    std::shared_ptr<LevelSetModel> lvlSetModel = lvlSetObj->getLevelSetModel();
    std::shared_ptr<ImageData>     grid = std::dynamic_pointer_cast<SignedDistanceField>(lvlSetModel->getModelGeometry())->getImage();

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
            const CollisionElement& rbdContactElement = elementsB[i];

            if (lsmContactElement.m_type != CollisionElementType::PointDirection
                || rbdContactElement.m_type != CollisionElementType::PointIndexDirection)
            {
                continue;
            }

            // If the point is in the mask, let it apply impulses
            if (m_ptIdMask.count(rbdContactElement.m_element.m_PointIndexDirectionElement.ptIndex) != 0)
            {
                const Vec3d& pos    = lsmContactElement.m_element.m_PointDirectionElement.pt;
                const Vec3d& normal = lsmContactElement.m_element.m_PointDirectionElement.dir;
                const Vec3i  coord  = (pos - origin).cwiseProduct(invSpacing).cast<int>();

                // Scale the applied impulse by the normal force
                const double fN = normal.normalized().dot(rbdObj->getRigidBody()->getForce()) / rbdObj->getRigidBody()->getForce().norm();
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
                            lvlSetModel->addImpulse(fCoord, S * m_kernelWeights[j++]);
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
            const CollisionElement& rbdContactElement = elementsB[i];

            if (lsmContactElement.m_type != CollisionElementType::PointDirection
                || rbdContactElement.m_type != CollisionElementType::PointIndexDirection)
            {
                continue;
            }

            // If the point is in the mask, let it apply impulses
            if (m_ptIdMask.count(rbdContactElement.m_element.m_PointIndexDirectionElement.ptIndex) != 0)
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
                            lvlSetModel->addImpulse(fCoord, S * m_kernelWeights[j++]);
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
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(getRigidObj()->getCollidingGeometry());
    for (int i = 0; i < pointSet->getNumVertices(); i++)
    {
        m_ptIdMask.insert(i);
    }
}
} // namespace imstk