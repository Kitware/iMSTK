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
#include "imstkLevelSetDeformableObject.h"
#include "imstkLevelSetModel.h"
#include "imstkImageData.h"
#include "imstkRigidObject2.h"
#include "imstkRbdConstraint.h"

namespace imstk
{
namespace expiremental
{
static double gaussianKernel[3][3][3] =
{
    {
        { 1.0, 2.0, 1.0 },
        { 2.0, 4.0, 2.0 },
        { 1.0, 2.0, 1.0 }
    },
    {
        { 2.0, 4.0, 2.0 },
        { 4.0, 8.0, 4.0 },
        { 2.0, 4.0, 2.0 }
    },
    {
        { 1.0, 2.0, 1.0 },
        { 2.0, 4.0, 2.0 },
        { 1.0, 2.0, 1.0 }
    }
};

LevelSetCH::LevelSetCH(const Side&                               side,
                       const std::shared_ptr<CollisionData>      colData,
                       std::shared_ptr<LevelSetDeformableObject> lvlSetObj,
                       std::shared_ptr<RigidObject2>             rigidObj) :
    CollisionHandling(Type::RBD, side, colData),
    m_lvlSetObj(lvlSetObj),
    m_rigidObj(rigidObj)
{
}

void
LevelSetCH::processCollisionData()
{
    std::shared_ptr<LevelSetModel> lvlSetModel = m_lvlSetObj->getLevelSetModel();
    std::shared_ptr<ImageData>     grid = std::dynamic_pointer_cast<ImageData>(lvlSetModel->getModelGeometry());

    //const Vec3i& dim = grid->getDimensions();
    const Vec3d& invSpacing = grid->getInvSpacing();
    const Vec3d& origin     = grid->getOrigin();

    //if (m_useProportionalForce)
    //{
    //    // Apply impulses at points of contacts
    //    PositionDirectionCollisionData& pdColData = m_colData->PDColData;
    //    for (int i = 0; i < pdColData.getSize(); i++)
    //    {
    //        const Vec3d& pos = pdColData[i].posB;
    //        const Vec3d& normal = pdColData[i].dirAtoB;
    //        const Vec3i  coord = (pos - origin).cwiseProduct(invSpacing).cast<int>();

    //        const double fN = normal.dot(m_rigidObj->getRigidBody()->getForce());
    //        const double S = m_velocityScaling;

    //        for (int z = 0; z < 3; z++)
    //        {
    //            for (int y = 0; y < 3; y++)
    //            {
    //                for (int x = 0; x < 3; x++)
    //                {
    //                    const Vec3i fCoord = coord + Vec3i(x - 1, y - 1, z - 1);
    //                    /*float S = 0.05f *
    //                            glm::max(
    //                                    glm::dot(-glm::normalize(computeGrad(fCoords[0], fCoords[1], imgPtr, dim, spacing)),
    //                                            glm::vec2(rigidObj->getForce())), 0.0f);*/
    //                    lvlSetModel->addImpulse(fCoord, S * gaussianKernel[x][y][z]);
    //                }
    //            }
    //        }
    //    }
    //}
    //else
    {
        // Apply impulses at points of contacts
        PositionDirectionCollisionData& pdColData = m_colData->PDColData;
        for (int i = 0; i < pdColData.getSize(); i++)
        {
            const Vec3d& pos = pdColData[i].posB;
            //const Vec3d& normal = pdColData[i].dirAtoB;
            const Vec3i  coord = (pos - origin).cwiseProduct(invSpacing).cast<int>();
            const double S     = m_velocityScaling;

            for (int z = 0; z < 3; z++)
            {
                for (int y = 0; y < 3; y++)
                {
                    for (int x = 0; x < 3; x++)
                    {
                        const Vec3i fCoord = coord + Vec3i(x - 1, y - 1, z - 1);
                        lvlSetModel->addImpulse(fCoord, S * gaussianKernel[x][y][z]);
                    }
                }
            }
        }
    }
}
}
}