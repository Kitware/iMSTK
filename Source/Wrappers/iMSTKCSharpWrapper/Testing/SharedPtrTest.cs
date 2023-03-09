using System;
using imstk;
using NUnit.Framework;
using System.Runtime.InteropServices;

namespace imstkCSUnitTest
{

[TestFixture]
public class SharedPtrTest
{
    // [Test]
    // public void TestActor()
    // {
    //     Assert.Multiple(() =>
    //     {
    //         HandleRef surfMeshHandleRef;
    //         {
    //             SurfaceMesh surfMesh = new SurfaceMesh();
    //             surfMeshHandleRef = SurfaceMesh.getCPtr(surfMesh);
    //             int use_count = UtilsPINVOKE.DebugAndTestOnly_use_count(SurfaceMesh.getCPtr(surfMesh), "SurfaceMesh");
    //             Assert.GreaterOrEqual(use_count, 1);
    //         }
    //
    //         GC.Collect();
    //         GC.WaitForPendingFinalizers();
    //
    //         // {
    //         //     DebugAndTestOnly u = new DebugAndTestOnly();
    //         //     Assert.AreEqual(1, DebugAndTestOnly.getNumCreated());
    //         //     Assert.AreEqual(0, DebugAndTestOnly.getNumDestroyed());
    //         // }
    //
    //         DebugAndTestOnly debugObj = createAndDestroy<DebugAndTestOnly>();
    //         debugObj = null;
    //         GC.Collect();
    //         GC.WaitForPendingFinalizers();
    //         Assert.AreEqual(1, DebugAndTestOnly.getNumCreated());
    //         // Assert.AreEqual(1, DebugAndTestOnly.getNumDestroyed());
    //     });
    //
    // }

    [Test]
    public void TestCast()
    {
        PbdSystem dynamicalModel = createPbdSystem();
        Assert.Multiple(() =>
        {
            PointSet pointSet = Utils.castToPointSet(dynamicalModel.getModelGeometry());
            Assert.That(pointSet, Is.Not.Null);
            Assert.AreEqual(4, pointSet.getNumVertices());

            SurfaceMesh surfMesh = Utils.castToSurfaceMesh(dynamicalModel.getModelGeometry());
            Assert.That(surfMesh, Is.Not.Null);
            Assert.AreEqual(2, surfMesh.getNumTriangles());

            TetrahedralMesh tetMesh = Utils.castToTetrahedralMesh(Utils.castToPointSet(dynamicalModel.getModelGeometry()));
            Assert.That(tetMesh, Is.Null);
        });
    }

    [Test]
    public void TestVertices()
    {
        Assert.Multiple(() =>
        {
            PbdSystem dynamicalModel = createPbdSystem();
            PointSet pointSet = Utils.castToPointSet(dynamicalModel.getModelGeometry());
            Assert.AreEqual(4, pointSet.getNumVertices());

            VecDataArray3d vertices = pointSet.getVertexPositions();
            Assert.AreEqual(4, vertices.size());
            Assert.AreEqual(0.0, vertices[0][0], tol);
            Assert.AreEqual(0.0, vertices[0][1], tol);
            Assert.AreEqual(0.0, vertices[0][2], tol);

            Assert.AreEqual(1.0, vertices[1][0], tol);
            Assert.AreEqual(0.0, vertices[1][1], tol);
            Assert.AreEqual(0.0, vertices[1][2], tol);

            Assert.AreEqual(0.0, vertices[2][0], tol);
            Assert.AreEqual(1.0, vertices[2][1], tol);
            Assert.AreEqual(0.0, vertices[2][2], tol);

            Assert.AreEqual(1.0, vertices[3][0], tol);
            Assert.AreEqual(1.0, vertices[3][1], tol);
            Assert.AreEqual(0.0, vertices[3][2], tol);

            // make changes to vertices
            vertices[0] = new Vec3d(0.1, 0.2, 0.3);

            PointSet pointSet2 = Utils.castToPointSet(dynamicalModel.getModelGeometry());
            VecDataArray3d changedVertices = pointSet.getVertexPositions();
            Assert.AreEqual(0.1, changedVertices[0][0], tol);
            Assert.AreEqual(0.2, changedVertices[0][1], tol);
            Assert.AreEqual(0.3, changedVertices[0][2], tol);

            Assert.AreEqual(1.0, changedVertices[1][0], tol);
            Assert.AreEqual(0.0, changedVertices[1][1], tol);
            Assert.AreEqual(0.0, changedVertices[1][2], tol);

            Assert.AreEqual(0.0, changedVertices[2][0], tol);
            Assert.AreEqual(1.0, changedVertices[2][1], tol);
            Assert.AreEqual(0.0, changedVertices[2][2], tol);

            Assert.AreEqual(1.0, changedVertices[3][0], tol);
            Assert.AreEqual(1.0, changedVertices[3][1], tol);
            Assert.AreEqual(0.0, changedVertices[3][2], tol);
        });
    }
        
    [Test]
    public void TestIndices()
    {
        Assert.Multiple(() =>
        {
            PbdSystem dynamicalModel = createPbdSystem();
            SurfaceMesh surfMesh = Utils.castToSurfaceMesh(dynamicalModel.getModelGeometry());
            Assert.That(surfMesh, Is.Not.Null);
            Assert.AreEqual(2, surfMesh.getNumTriangles());

            VecDataArray3i indices = surfMesh.getTriangleIndices();
            Assert.AreEqual(0, indices[0][0]);
            Assert.AreEqual(1, indices[0][1]);
            Assert.AreEqual(2, indices[0][2]);

            Assert.AreEqual(1, indices[1][0]);
            Assert.AreEqual(3, indices[1][1]);
            Assert.AreEqual(2, indices[1][2]);

            // make changes to indices
            indices[0] = new Vec3i(1, 2, 0);

            SurfaceMesh surfMesh2 = Utils.castToSurfaceMesh(dynamicalModel.getModelGeometry());
            Assert.That(surfMesh2, Is.Not.Null);
            Assert.AreEqual(2, surfMesh2.getNumTriangles());

            VecDataArray3i changedIndices = surfMesh2.getTriangleIndices();
            Assert.AreEqual(2, changedIndices.size());
            Assert.AreEqual(1, changedIndices[0][0]);
            Assert.AreEqual(2, changedIndices[0][1]);
            Assert.AreEqual(0, changedIndices[0][2]);

            Assert.AreEqual(1, changedIndices[1][0]);
            Assert.AreEqual(3, changedIndices[1][1]);
            Assert.AreEqual(2, changedIndices[1][2]);
        });
    }

    public static PbdSystem createPbdSystem()
    {
        PbdSystem pbdSystem = new PbdSystem();
        SurfaceMesh surfMesh = createSurfaceMesh();
        pbdSystem.setModelGeometry(surfMesh);
        return pbdSystem;
    }

    public static SurfaceMesh createSurfaceMesh()
    {

        VecDataArray3i conn = new VecDataArray3i(2);
        conn[0] = new Vec3i(0, 1, 2);
        conn[1] = new Vec3i(1, 3, 2);

        VecDataArray3d coords = new VecDataArray3d(4);
        coords[0] = new Vec3d(0.0, 0.0, 0.0);
        coords[1] = new Vec3d(1.0, 0.0, 0.0);
        coords[2] = new Vec3d(0.0, 1.0, 0.0);
        coords[3] = new Vec3d(1.0, 1.0, 0.0);

        SurfaceMesh surfMesh = new SurfaceMesh();
        surfMesh.initialize(coords, conn);

        return surfMesh;
    }

    private static T createAndDestroy<T>() where T : new()
    {
        T u = new T();
        return u;
    }


    private static double tol = 1e-14;
}
}
