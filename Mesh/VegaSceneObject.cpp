
#include "Mesh/VegaSceneObject.h"
#include "objMeshEncode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

VegaSceneObject::VegaSceneObject(char * filename):
  mesh(nullptr)
{
  int verbose = 0;
  if (filename && filename[0])
    {
    mesh = std::make_shared<ObjMesh>(filename, verbose);

    int encStart = strlen(filename) - 4;
    if ((encStart > 0) && (strcmp(&filename[encStart], ".enc") == 0))
      {
      // must decode
      printf("Decoding mesh.\n");
      objMeshDecode(mesh.get());
      printf("Decoded mesh.\n");
      }

    BuildFaceNormals();

    n = mesh->getNumVertices();
    }
  else
    {
    n = 0;
    }
}

VegaSceneObject::~VegaSceneObject()
{
}

// assumes pre-existing face normals
// second parameter is treshold angle for hard edges
void VegaSceneObject::BuildVertexNormals(double thresholdAngle)
{
  //do stuff with structure
  mesh->buildVertexNormals(thresholdAngle);
}

void VegaSceneObject::BuildFaceNormals()
{
  mesh->buildFaceNormals();
}

void VegaSceneObject::BuildNormals(double thresholdAngle)
{
  BuildFaceNormals();
  BuildVertexNormals(thresholdAngle);
}

void VegaSceneObject::SetNormalsToFaceNormals()
{
  mesh->setNormalsToFaceNormals();
}

void VegaSceneObject::BuildNormalsFancy(double thresholdAngle)
{
  BuildFaceNormals();
  mesh->buildVertexNormalsFancy(thresholdAngle);
}

int VegaSceneObject::GetClosestVertex(Vec3d & queryPos, double * distance, double * /*auxVertexBuffer*/)
{
  return mesh->getClosestVertex(queryPos, distance);
}

void VegaSceneObject::BuildNeighboringStructure()
{
  mesh->buildVertexFaceNeighbors();
}

void VegaSceneObject::ComputeMeshGeometricParameters(Vec3d * centroid, double * radius)
{
  mesh->getMeshGeometricParameters(centroid, radius);
}

void VegaSceneObject::ComputeMeshRadius(Vec3d & centroid, double * radius)
{
  mesh->getMeshRadius(centroid, radius);
}

void VegaSceneObject::ExportMeshGeometry(int * numVertices, double ** vertices, int * numTriangles, int ** triangles)
{
  mesh->exportGeometry(numVertices, vertices, numTriangles, triangles, nullptr, nullptr);
}

void VegaSceneObject::TransformRigidly(double * centerOfMass, double * R)
{
  Vec3d cv(centerOfMass);
  Mat3d Rv(R);
  mesh->transformRigidly(cv, Rv);
}

