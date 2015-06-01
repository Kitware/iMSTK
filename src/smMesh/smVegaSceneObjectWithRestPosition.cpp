#include <string.h>
#include <stdlib.h>
#include "smMesh/smVegaSceneObjectWithRestPosition.h"

smVegaSceneObjectWithRestPosition::smVegaSceneObjectWithRestPosition(char * filename): smVegaSceneObject(filename)
{
  restPosition = std::make_shared<double>(new double[3 * n]);
  for(int i = 0; i < n; i++)
  {
    Vec3d pos = mesh->getPosition(i);
    (restPosition.get())[3 * i + 0] = pos[0];
    (restPosition.get())[3 * i + 1] = pos[1];
    (restPosition.get())[3 * i + 2] = pos[2];
  }
}

smVegaSceneObjectWithRestPosition::~smVegaSceneObjectWithRestPosition()
{
}

void smVegaSceneObjectWithRestPosition::GetVertexRestPositions(double * buffer)
{
  for (int i = 0; i<3 * n; i++)
      buffer[i] = (restPosition.get()[i]);
}

void smVegaSceneObjectWithRestPosition::GetVertexRestPositions(float * buffer)
{
  for(int i=0; i<3*n; i++)
    buffer[i] = (float) (restPosition.get()[i]);
}

void smVegaSceneObjectWithRestPosition::TransformRigidly(double * centerOfMass, double * R)
{
  smVegaSceneObject::TransformRigidly(centerOfMass, R);

  for(int i=0; i<n; i++)
  {
    double * target = &(restPosition.get()[3*i]);
    double temp[3]; // centerOfMass + R * restPosition
    temp[0] = R[0] * target[0] + R[1] * target[1] + R[2] * target[2];
    temp[1] = R[3] * target[0] + R[4] * target[1] + R[5] * target[2];
    temp[2] = R[6] * target[0] + R[7] * target[1] + R[8] * target[2];
    target[0] = centerOfMass[0] + temp[0];
    target[1] = centerOfMass[1] + temp[1];
    target[2] = centerOfMass[2] + temp[2];
  }
}

