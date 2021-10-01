using System;
using imstk;

public class PbdCloth
{
    public static void Main(string[] args)
    {
        // Write log to stdout and file
        Logger.startLogger();

        VecDataArray4i conn = new VecDataArray4i(1);
        Vec4i vec4i = new Vec4i();
        vec4i[0] = 0;
        vec4i[1] = 1;
        vec4i[2] = 2;
        vec4i[3] = 3;
        conn[0] = vec4i;

        VecDataArray3d coords = new VecDataArray3d(4);
        Vec3d xyz = new Vec3d();
        xyz[0] = 0.0;
        xyz[1] = 0.0;
        xyz[2] = 0.0;
        coords[0] = xyz;
        uint id = 0;
        Console.WriteLine("coords[{0}] = [{1}, {2}, {3}]", id, coords[id][0], coords[id][1], coords[id][2]);

        xyz[0] = 1.0;
        xyz[1] = 0.0;
        xyz[2] = 0.0;
        coords[1] = xyz;
        id = 1;
        Console.WriteLine("coords[{0}] = [{1}, {2}, {3}]", id, coords[id][0], coords[id][1], coords[id][2]);

        xyz[0] = 0.0;
        xyz[1] = 1.0;
        xyz[2] = 0.0;
        coords[2] = xyz;
        id = 2;
        id = 2;
        Console.WriteLine("coords[{0}] = [{1}, {2}, {3}]", id, coords[id][0], coords[id][1], coords[id][2]);

        xyz[0] = 0.0;
        xyz[1] = 0.0;
        xyz[2] = 1.0;
        coords[3] = xyz;
        id = 3;
        Console.WriteLine("coords[{0}] = [{1}, {2}, {3}]", id, coords[id][0], coords[id][1], coords[id][2]);

        TetrahedralMesh tetMesh = new TetrahedralMesh();
        tetMesh.initialize(coords, conn);

        Console.WriteLine("vol = {0}", tetMesh.getVolume());
        Console.WriteLine("numberOfTets = {0}", tetMesh.getNumTetrahedra());
        Vec4i conn_tet = tetMesh.getTetrahedronIndices(0);
        Console.WriteLine("conn[0] = [{0}, {1}, {2}, {3}]", conn_tet[0], conn_tet[1], conn_tet[2], conn_tet[3]);

        {
            VecDataArray4i conn2 = tetMesh.getTetrahedraIndices();
            conn2[0] = new Vec4i(1, 2, 3, 0);
            Vec4i new_conn_tet = tetMesh.getTetrahedronIndices(0);
            Console.WriteLine("conn[0] = [{0}, {1}, {2}, {3}]", new_conn_tet[0], new_conn_tet[1], new_conn_tet[2], new_conn_tet[3]);
        }

        tetMesh.print();

        PointSet asianDragonMesh = MeshIO.read("/home/jianfeng/Documents/imstk/build_csharp_shared/install/data/asianDragon/asianDragon.veg");
        // asianDragonMesh.print();
        Console.WriteLine("asianDragon.getVolume() = {0}", asianDragonMesh.getVolume());
        Console.WriteLine("asianDragon.getNumVertices() = {0}", asianDragonMesh.getNumVertices());
    }
}

