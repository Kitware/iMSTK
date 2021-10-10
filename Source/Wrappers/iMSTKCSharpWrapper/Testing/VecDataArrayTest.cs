using System;
using imstk;
using NUnit.Framework;


namespace imstkCSUnitTest
{
[TestFixture]
public class VecDataArrayTest
{
    [Test]
    public void TestConstructor()
    {
        // default constructors
        {
            VecDataArray3d u = new VecDataArray3d();
            Assert.AreEqual(0, u.size());
            Assert.AreEqual(3, u.getNumberOfComponents());
            Assert.GreaterOrEqual(u.getCapacity(), 3*0);
            Assert.AreEqual(Utils.IMSTK_DOUBLE, u.getScalarType());
        }

        {
            VecDataArray3d u = new VecDataArray3d(4);
            Assert.AreEqual(4, u.size());
            Assert.AreEqual(3, u.getNumberOfComponents());
            Assert.GreaterOrEqual(u.getCapacity(), 3*4);
        }

    }

    [Test]
    public void TestResize()
    {
        Assert.Multiple(() =>
        {
            VecDataArray3d u = new VecDataArray3d(4);
            u.resize(5);
            Assert.AreEqual(5, u.size());
            Assert.AreEqual(3, u.getNumberOfComponents());
            Assert.GreaterOrEqual(u.getCapacity(), 3*u.size());
        
        });
    }

    [Test]
    public void TestFill()
    {
        Assert.Multiple(() =>
        {
            VecDataArray3d u = new VecDataArray3d(2);
            Vec3d val = new Vec3d(0.0, 1.0, 2.0);
            u.fill(val);
            Assert.AreEqual(u[0][0], 0.0, tol);
            Assert.AreEqual(u[0][1], 1.0, tol);
            Assert.AreEqual(u[0][2], 2.0, tol);

            Assert.AreEqual(u[1][0], 0.0, tol);
            Assert.AreEqual(u[1][1], 1.0, tol);
            Assert.AreEqual(u[1][2], 2.0, tol);
        });
    }

    [Test]
    public void TestEraseAndPushBack()
    {
        Assert.Multiple(() =>
        {
            VecDataArray3d u = new VecDataArray3d(4);
            u[0] = new Vec3d(0.0, 1.0, 2.0);
            u[1] = new Vec3d(3.0, 4.0, 5.0);
            u[2] = new Vec3d(6.0, 7.0, 8.0);
            u[3] = new Vec3d(9.0, 10.0, 11.0);

            u.erase(1);
            Assert.AreEqual(3, u.size());
            Assert.GreaterOrEqual(u.getCapacity(), u.size()*3);

            Assert.AreEqual(0.0, u[0][0], tol);
            Assert.AreEqual(1.0, u[0][1], tol);
            Assert.AreEqual(2.0, u[0][2], tol);
            Assert.AreEqual(6.0, u[1][0], tol);
            Assert.AreEqual(7.0, u[1][1], tol);
            Assert.AreEqual(8.0, u[1][2], tol);
            Assert.AreEqual(9.0, u[2][0], tol);
            Assert.AreEqual(10.0, u[2][1], tol);
            Assert.AreEqual(11.0, u[2][2], tol);

            u.push_back(new Vec3d(12.0, 13.0, 14.0));
            Assert.AreEqual(4, u.size());
            Assert.GreaterOrEqual(u.getCapacity(), u.size()*3);
            Assert.AreEqual(0.0, u[0][0], tol);
            Assert.AreEqual(1.0, u[0][1], tol);
            Assert.AreEqual(2.0, u[0][2], tol);

            Assert.AreEqual(6.0, u[1][0], tol);
            Assert.AreEqual(7.0, u[1][1], tol);
            Assert.AreEqual(8.0, u[1][2], tol);

            Assert.AreEqual(9.0, u[2][0], tol);
            Assert.AreEqual(10.0, u[2][1], tol);
            Assert.AreEqual(11.0, u[2][2], tol);

            Assert.AreEqual(12.0, u[3][0], tol);
            Assert.AreEqual(13.0, u[3][1], tol);
            Assert.AreEqual(14.0, u[3][2], tol);
        });
    }

    [Test]
    public void TestSetValuesAndGetValues()
    {
        Assert.Multiple(() =>
        {
            const int sz = 99;
            const int numData = (int)sz * 3;
            const double eps = 0.01;
            double[] values = new double[numData];
            for (uint i=0; i<values.Length; ++i)
            {
                values[i] = (double)i + eps;
            }

            VecDataArray3d u = new VecDataArray3d(sz);
            u.setValues(values);

            for (uint i=0; i<sz; ++i)
            {
                Assert.AreEqual(u[i][0], i*3+0 + eps, tol);
                Assert.AreEqual(u[i][1], i*3+1 + eps, tol);
                Assert.AreEqual(u[i][2], i*3+2 + eps, tol);
            }

            double[] toValues = new double[numData];

            u.getValues(toValues);

            for (int i=0; i<numData; ++i)
            {
                Assert.AreEqual(toValues[i], values[i], tol);
            }
        });
    }

    private static double tol = 1e-14;
}
}
