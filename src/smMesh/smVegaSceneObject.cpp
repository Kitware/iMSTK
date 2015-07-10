#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include "smMesh/smVegaSceneObject.h"
#include "objMeshEncode.h"

smVegaSceneObject::smVegaSceneObject(char * filename):
  mesh(NULL)
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

smVegaSceneObject::~smVegaSceneObject()
{
}

// assumes pre-existing face normals
// second parameter is treshold angle for hard edges
void smVegaSceneObject::BuildVertexNormals(double thresholdAngle)
{
  //do stuff with structure
  mesh->buildVertexNormals(thresholdAngle);
}

void smVegaSceneObject::BuildFaceNormals()
{
  mesh->buildFaceNormals();
}

void smVegaSceneObject::BuildNormals(double thresholdAngle)
{
  BuildFaceNormals();
  BuildVertexNormals(thresholdAngle);
}

void smVegaSceneObject::SetNormalsToFaceNormals()
{
  mesh->setNormalsToFaceNormals();
}

void smVegaSceneObject::BuildNormalsFancy(double thresholdAngle)
{
  BuildFaceNormals();
  mesh->buildVertexNormalsFancy(thresholdAngle);
}

int smVegaSceneObject::GetClosestVertex(Vec3d & queryPos, double * distance, double * auxVertexBuffer)
{
  return mesh->getClosestVertex(queryPos, distance);
}

void smVegaSceneObject::BuildNeighboringStructure()
{
  mesh->buildVertexFaceNeighbors();
}

void smVegaSceneObject::ComputeMeshGeometricParameters(Vec3d * centroid, double * radius)
{
  mesh->getMeshGeometricParameters(centroid, radius);
}

void smVegaSceneObject::ComputeMeshRadius(Vec3d & centroid, double * radius)
{
  mesh->getMeshRadius(centroid, radius);
}

void smVegaSceneObject::ExportMeshGeometry(int * numVertices, double ** vertices, int * numTriangles, int ** triangles)
{
  mesh->exportGeometry(numVertices, vertices, numTriangles, triangles, NULL, NULL);
}

void smVegaSceneObject::TransformRigidly(double * centerOfMass, double * R)
{
  Vec3d cv(centerOfMass);
  Mat3d Rv(R);
  mesh->transformRigidly(cv, Rv);
}

