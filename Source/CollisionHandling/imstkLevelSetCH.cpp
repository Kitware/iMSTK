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

#include "imstkLevelSetCH.h"
#include "imstkCollisionData.h"
#include "imstkImageData.h"
#include "imstkLevelSetDeformableObject.h"
#include "imstkLevelSetModel.h"
#include "imstkRbdConstraint.h"
#include "imstkRigidObject2.h"

namespace imstk
{
namespace expiremental
{
LevelSetCH::LevelSetCH(const Side&                               side,
                       const std::shared_ptr<CollisionData>      colData,
                       std::shared_ptr<LevelSetDeformableObject> lvlSetObj,
                       std::shared_ptr<RigidObject2>             rigidObj) :
    CollisionHandling(Type::LevelSet, side, colData),
    m_lvlSetObj(lvlSetObj),
    m_rigidObj(rigidObj)
{
    setKernel(m_kernelSize, m_kernelSigma);
    maskAllPoints();
}

LevelSetCH::~LevelSetCH()
{
    if (m_kernelWeights != nullptr)
    {
        delete[] m_kernelWeights;
    }
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
LevelSetCH::processCollisionData()
{
    std::shared_ptr<LevelSetModel> lvlSetModel = m_lvlSetObj->getLevelSetModel();
    std::shared_ptr<ImageData>     grid = std::dynamic_pointer_cast<ImageData>(lvlSetModel->getModelGeometry());

    if (grid == nullptr)
    {
        LOG(FATAL) << "LevelSetCH::processCollisionData: level set model geometry is not ImageData";
        return;
    }

    //const Vec3i& dim = grid->getDimensions();
    const Vec3d& invSpacing = grid->getInvSpacing();
    const Vec3d& origin     = grid->getOrigin();

    if (m_useProportionalForce)
    {
        // Apply impulses at points of contacts
        PositionDirectionCollisionData& pdColData = m_colData->PDColData;
        for (int i = 0; i < pdColData.getSize(); i++)
        {
            // If the point is in the mask, let it apply impulses
            if (m_ptIdMask.count(pdColData[i].nodeIdx) != 0)
            {
                const Vec3d& pos    = pdColData[i].posB;
                const Vec3d& normal = pdColData[i].dirAtoB;
                const Vec3i  coord  = (pos - origin).cwiseProduct(invSpacing).cast<int>();

                // Scale the applied impulse by the normal force
                const double fN = normal.normalized().dot(m_rigidObj->getRigidBody()->getForce()) / m_rigidObj->getRigidBody()->getForce().norm();
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
        PositionDirectionCollisionData& pdColData = m_colData->PDColData;
        for (int i = 0; i < pdColData.getSize(); i++)
        {
            // If the point is in the mask, let it apply impulses
            if (m_ptIdMask.count(pdColData[i].nodeIdx) != 0)
            {
                const Vec3d& pos = pdColData[i].posB;
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
    std::shared_ptr<PointSet> pointSet = std::dynamic_pointer_cast<PointSet>(m_rigidObj->getCollidingGeometry());
    for (int i = 0; i < static_cast<int>(pointSet->getNumVertices()); i++)
    {
        m_ptIdMask.insert(i);
    }
}
}
}