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

#include "gtest/gtest.h"

#include "imstkCollisionData.h"

using namespace imstk;

namespace
{
} // namespace

struct imstkCollisionElementTest : public ::testing::Test
{
    imstkCollisionElementTest()
    {
        cv.pts[0] = Vec3d{ 1, 2, 3 };
        cv.pts[1] = Vec3d{ 2, 3, 4 };
        cv.pts[2] = Vec3d{ 3, 4, 5 };
        cv.pts[3] = Vec3d{ 4, 5, 6 };
        cv.size   = 10;

        ci.ids[0]   = 7;
        ci.ids[1]   = 8;
        ci.ids[2]   = 9;
        ci.ids[3]   = 10;
        ci.idCount  = 11;
        ci.cellType = IMSTK_VERTEX;

        pd.pt  = Vec3d{ 2, 3, 4 };
        pd.dir = Vec3d{ 4, 5, 6 };
        pd.penetrationDepth = 12.0;

        pi.ptIndex = 13;
        pi.dir     = Vec3d{ 3, 4, 5 };
        pi.penetrationDepth = 2.0;
    };

    CellVertexElement cv;
    CellIndexElement ci;
    PointDirectionElement pd;
    PointIndexDirectionElement pi;
};

TEST_F(imstkCollisionElementTest, Constructor)
{
    {
        CollisionElement e;
        EXPECT_EQ(CollisionElementType::Empty, e.m_type);
    }
    {
        CollisionElement e(cv);
        EXPECT_EQ(CollisionElementType::CellVertex, e.m_type);
        EXPECT_EQ(cv.size, e.m_element.m_CellVertexElement.size);
        EXPECT_EQ(cv.pts[1], e.m_element.m_CellVertexElement.pts[1]);
    }
    {
        CollisionElement e(ci);
        EXPECT_EQ(CollisionElementType::CellIndex, e.m_type);
        EXPECT_EQ(ci.idCount, e.m_element.m_CellIndexElement.idCount);
        EXPECT_EQ(ci.ids[1], e.m_element.m_CellIndexElement.ids[1]);
    }
    {
        CollisionElement e(pd);
        EXPECT_EQ(CollisionElementType::PointDirection, e.m_type);
        EXPECT_EQ(pd.dir, e.m_element.m_PointDirectionElement.dir);
        EXPECT_EQ(pd.penetrationDepth, e.m_element.m_PointDirectionElement.penetrationDepth);
    }
    {
        CollisionElement e(pi);
        EXPECT_EQ(CollisionElementType::PointIndexDirection, e.m_type);
        EXPECT_EQ(pi.dir, e.m_element.m_PointIndexDirectionElement.dir);
        EXPECT_EQ(pi.ptIndex, e.m_element.m_PointIndexDirectionElement.ptIndex);
    }
}

TEST_F(imstkCollisionElementTest, CopyConstructor)
{
    {
        CollisionElement old;
        CollisionElement e(old);
        EXPECT_EQ(CollisionElementType::Empty, e.m_type);
    }
    {
        CollisionElement old(cv);
        CollisionElement e(old);
        EXPECT_EQ(CollisionElementType::CellVertex, e.m_type);
        EXPECT_EQ(cv.size, e.m_element.m_CellVertexElement.size);
        EXPECT_EQ(cv.pts[1], e.m_element.m_CellVertexElement.pts[1]);
    }
    {
        CollisionElement old(ci);
        CollisionElement e(old);
        EXPECT_EQ(CollisionElementType::CellIndex, e.m_type);
        EXPECT_EQ(ci.idCount, e.m_element.m_CellIndexElement.idCount);
        EXPECT_EQ(ci.ids[1], e.m_element.m_CellIndexElement.ids[1]);
    }
    {
        CollisionElement old(pd);
        CollisionElement e(old);
        EXPECT_EQ(CollisionElementType::PointDirection, e.m_type);
        EXPECT_EQ(pd.dir, e.m_element.m_PointDirectionElement.dir);
        EXPECT_EQ(pd.penetrationDepth, e.m_element.m_PointDirectionElement.penetrationDepth);
    }
    {
        CollisionElement old(pi);
        CollisionElement e(old);
        EXPECT_EQ(CollisionElementType::PointIndexDirection, e.m_type);
        EXPECT_EQ(pi.dir, e.m_element.m_PointIndexDirectionElement.dir);
        EXPECT_EQ(pi.ptIndex, e.m_element.m_PointIndexDirectionElement.ptIndex);
    }
}

TEST_F(imstkCollisionElementTest, DataAssignment)
{
        CollisionElement e;
        e = cv;
        EXPECT_EQ(CollisionElementType::CellVertex, e.m_type);
        EXPECT_EQ(cv.size, e.m_element.m_CellVertexElement.size);
        EXPECT_EQ(cv.pts[1], e.m_element.m_CellVertexElement.pts[1]);

        e = ci;
        EXPECT_EQ(CollisionElementType::CellIndex, e.m_type);
        EXPECT_EQ(ci.idCount, e.m_element.m_CellIndexElement.idCount);
        EXPECT_EQ(ci.ids[1], e.m_element.m_CellIndexElement.ids[1]);

        e = pd;
        EXPECT_EQ(CollisionElementType::PointDirection, e.m_type);
        EXPECT_EQ(pd.dir, e.m_element.m_PointDirectionElement.dir);
        EXPECT_EQ(pd.penetrationDepth, e.m_element.m_PointDirectionElement.penetrationDepth);

        e = pi;
        EXPECT_EQ(CollisionElementType::PointIndexDirection, e.m_type);
        EXPECT_EQ(pi.dir, e.m_element.m_PointIndexDirectionElement.dir);
        EXPECT_EQ(pi.ptIndex, e.m_element.m_PointIndexDirectionElement.ptIndex);
}

TEST_F(imstkCollisionElementTest, Assignment)
{
    {
        CollisionElement old;
        CollisionElement e(pi);
        e = old;
        EXPECT_EQ(CollisionElementType::Empty, e.m_type);
    }
    {
        CollisionElement old(cv);
        CollisionElement e;
        e = old;
        EXPECT_EQ(CollisionElementType::CellVertex, e.m_type);
        EXPECT_EQ(cv.size, e.m_element.m_CellVertexElement.size);
        EXPECT_EQ(cv.pts[1], e.m_element.m_CellVertexElement.pts[1]);
    }
    {
        CollisionElement old(ci);
        CollisionElement e;
        e = old;
        EXPECT_EQ(CollisionElementType::CellIndex, e.m_type);
        EXPECT_EQ(ci.idCount, e.m_element.m_CellIndexElement.idCount);
        EXPECT_EQ(ci.ids[1], e.m_element.m_CellIndexElement.ids[1]);
    }
    {
        CollisionElement old(pd);
        CollisionElement e;
        e = old;
        EXPECT_EQ(CollisionElementType::PointDirection, e.m_type);
        EXPECT_EQ(pd.dir, e.m_element.m_PointDirectionElement.dir);
        EXPECT_EQ(pd.penetrationDepth, e.m_element.m_PointDirectionElement.penetrationDepth);
    }
    {
        CollisionElement old(pi);
        CollisionElement e;
        e = old;
        EXPECT_EQ(CollisionElementType::PointIndexDirection, e.m_type);
        EXPECT_EQ(pi.dir, e.m_element.m_PointIndexDirectionElement.dir);
        EXPECT_EQ(pi.ptIndex, e.m_element.m_PointIndexDirectionElement.ptIndex);
    }
}