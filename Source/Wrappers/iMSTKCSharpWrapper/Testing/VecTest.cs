
using System;
using imstk;
using NUnit.Framework;
using System.Runtime.InteropServices;

namespace imstkCSUnitTest
{

[TestFixture]
public class Vec3dTest
{
    [Test]
    public void TestConstructorAndIndexing()
    {
        Assert.Multiple(() =>
        {
            Vec3d u = new Vec3d(0.1, 0.2);
            Assert.AreEqual(0.1, u[0], tol);
            Assert.AreEqual(0.2, u[1], tol);

            u[2] = 100.0;
            Assert.AreEqual(100.0, u[2], tol);

            Vec3d v = new Vec3d(0.1, 0.2, 0.3);
            Assert.AreEqual(0.1, v[0], tol);
            Assert.AreEqual(0.2, v[1], tol);
            Assert.AreEqual(0.3, v[2], tol);
            v[2] = 100.0 + 1e-12;
            Assert.AreEqual(100.0+1e-12, v[2], tol);
        });
    }

    [Test]
    public void TestOperators()
    {
        Assert.Multiple(() =>
        {
            Vec3d v = new Vec3d(0.1, 0.2, 0.3);
            // *
            const double factor = 2.0 + 1e-6;
            v = v*factor;
            Assert.AreEqual(0.1*factor, v[0], tol);
            Assert.AreEqual(0.2*factor, v[1], tol);
            Assert.AreEqual(0.3*factor, v[2], tol);

            v = factor*v;
            Assert.AreEqual(0.1*factor*factor, v[0], tol);
            Assert.AreEqual(0.2*factor*factor, v[1], tol);
            Assert.AreEqual(0.3*factor*factor, v[2], tol);

            // /
            v = v / factor;
            Assert.AreEqual(0.1*factor, v[0], tol);
            Assert.AreEqual(0.2*factor, v[1], tol);
            Assert.AreEqual(0.3*factor, v[2], tol);

            // +
            Vec3d u = new Vec3d(0.1, 0.2, 0.3);
            v = u + new Vec3d(0.9, 0.8, 0.7);
            Assert.AreEqual(1.0, v[0], tol);
            Assert.AreEqual(1.0, v[1], tol);
            Assert.AreEqual(1.0, v[2], tol);

            // -
            v = u - new Vec3d(0.1-1e-6, 0.2-1e-7, 0.3-1e-8);
            Assert.AreEqual(1e-6, v[0], tol);
            Assert.AreEqual(1e-7, v[1], tol);
            Assert.AreEqual(1e-8, v[2], tol);

            // normalized
            u = new Vec3d(3.0, 4.0, 0.0);
            Vec3d w = u.normalized();
            Assert.AreEqual(0.6, w[0], tol);
            Assert.AreEqual(0.8, w[1], tol);
            Assert.AreEqual(0.0, w[2], tol);

        });
    }

    private static double tol = 1e-14;
}

[TestFixture]
public class Vec2fTest
{
    [Test]
    public void TestConstructorAndIndexing()
    {
        Assert.Multiple(() =>
        {
            Vec2f u = new Vec2f(0.1f, 0.2f);
            Assert.AreEqual(0.1f, u[0], tol);
            Assert.AreEqual(0.2f, u[1], tol);

            u[1] = 100.0f + 1e-4f;
            Assert.AreEqual(100.0f+1e-4f, u[1], tol);
        });
    }

    [Test]
    public void TestOperators()
    {
        Assert.Multiple(() =>
        {
            Vec2f v = new Vec2f(0.1f, 0.2f);
            // *
            const float factor = 2.0f + 1e-4f;
            v = factor*v;
            Assert.AreEqual(0.1f*factor, v[0], tol);
            Assert.AreEqual(0.2f*factor, v[1], tol);

            // /
            v = v / factor;
            Assert.AreEqual(0.1f, v[0], tol);
            Assert.AreEqual(0.2f, v[1], tol);
        });
    }

    private static double tol = 1e-6;
}

[TestFixture]
public class Vec3iTest
{
    [Test]
    public void TestConstructorAndIndexing()
    {
        Assert.Multiple(() =>
        {
            Vec3i u = new Vec3i(1, 2);
            Assert.AreEqual(1, u[0]);
            Assert.AreEqual(2, u[1]);

            u[2] = 100;
            Assert.AreEqual(100, u[2]);
        });
    }
}

[TestFixture]
public class Vec4iTest
{
    [Test]
    public void TestConstructorAndIndexing()
    {
        Assert.Multiple(() =>
        {
            Vec4i u = new Vec4i(1, 2, 3, 4);
            Assert.AreEqual(1, u[0]);
            Assert.AreEqual(2, u[1]);
            Assert.AreEqual(3, u[2]);
            Assert.AreEqual(4, u[3]);

            u[1] = 100;
            Assert.AreEqual(100, u[1]);
        });
    }
}


[TestFixture]
public class Vec6dTest
{
    [Test]
    public void TestConstructorAndIndexing()
    {
        Assert.Multiple(() =>
        {
            Vec6d u = new Vec6d(0.1, 0.2, 0.3, 0.4, 0.5, 0.6);
            Assert.AreEqual(0.1, u[0], tol);
            Assert.AreEqual(0.2, u[1], tol);
            Assert.AreEqual(0.3, u[2], tol);
            Assert.AreEqual(0.4, u[3], tol);
            Assert.AreEqual(0.5, u[4], tol);
            Assert.AreEqual(0.6, u[5], tol);

            u[2] = 100.0;
            Assert.AreEqual(100.0, u[2], tol);

            Vec6d v = new Vec6d(0.1, 0.2, 0.3);
            Assert.AreEqual(0.1, v[0], tol);
            Assert.AreEqual(0.2, v[1], tol);
            Assert.AreEqual(0.3, v[2], tol);
            v[5] = 100.0 + 1e-12;
            Assert.AreEqual(100.0+1e-12, v[5], tol);
        });
    }

    [Test]
    public void TestOperators()
    {
        Assert.Multiple(() =>
        {
            Vec6d v = new Vec6d(0.1, 0.2, 0.3, 0.4, 0.5, 0.6);
            // *
            const double factor = 2.0 + 1e-6;
            v = v*factor;
            Assert.AreEqual(0.1*factor, v[0], tol);
            Assert.AreEqual(0.2*factor, v[1], tol);
            Assert.AreEqual(0.3*factor, v[2], tol);
            Assert.AreEqual(0.4*factor, v[3], tol);
            Assert.AreEqual(0.5*factor, v[4], tol);
            Assert.AreEqual(0.6*factor, v[5], tol);

            v = factor*v;
            Assert.AreEqual(0.1*factor*factor, v[0], tol);
            Assert.AreEqual(0.2*factor*factor, v[1], tol);
            Assert.AreEqual(0.3*factor*factor, v[2], tol);
            Assert.AreEqual(0.4*factor*factor, v[3], tol);
            Assert.AreEqual(0.5*factor*factor, v[4], tol);
            Assert.AreEqual(0.6*factor*factor, v[5], tol);

            // /
            v = v / factor;
            Assert.AreEqual(0.1*factor, v[0], tol);
            Assert.AreEqual(0.2*factor, v[1], tol);
            Assert.AreEqual(0.3*factor, v[2], tol);
            Assert.AreEqual(0.4*factor, v[3], tol);
            Assert.AreEqual(0.5*factor, v[4], tol);
            Assert.AreEqual(0.6*factor, v[5], tol);
        });
    }

    private static double tol = 1e-14;
}
}

