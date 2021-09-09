#include "gtest/gtest.h"

#include "imstkCollisionData.h"

using namespace imstk;

namespace {
    CellVertexElement cv{ {{1,2,3},{2,3,4},{3,4,5}}, 10 };
    CellIndexElement ci{ {7,8,9,10}, 11, IMSTK_VERTEX };
    PointDirectionElement pd{ {2,3,4}, {4,5,6}, 12.0 };
    PointIndexDirectionElement pi{ 13, {3,4,5}, 2.0 };
}

TEST(imstkCollisionElementTest, Constructor)
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

TEST(imstkCollisionElementTest, CopyConstructor)
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

TEST(imstkCollisionElementTest, DataAssignment)
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

TEST(imstkCollisionElementTest, Assignment)
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