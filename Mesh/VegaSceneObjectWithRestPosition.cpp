
#include "Mesh/VegaSceneObjectWithRestPosition.h"

#include <string.h>
#include <stdlib.h>

VegaSceneObjectWithRestPosition::VegaSceneObjectWithRestPosition(char * filename): VegaSceneObject(filename)
{
  restPosition.resize(3 * n);
  for(int i = 0; i < n; i++)
  {
    Vec3d pos = mesh->getPosition(i);
    restPosition[3 * i + 0] = pos[0];
    restPosition[3 * i + 1] = pos[1];
    restPosition[3 * i + 2] = pos[2];
  }
}

VegaSceneObjectWithRestPosition::~VegaSceneObjectWithRestPosition()
{
}

void VegaSceneObjectWithRestPosition::TransformRigidly(double * centerOfMass, double * R)
{
  VegaSceneObject::TransformRigidly(centerOfMass, R);

  for(int i=0; i<n; i++)
  {
    double * target = &(restPosition[3*i]);
    double temp[3]; // centerOfMass + R * restPosition
    temp[0] = R[0] * target[0] + R[1] * target[1] + R[2] * target[2];
    temp[1] = R[3] * target[0] + R[4] * target[1] + R[5] * target[2];
    temp[2] = R[6] * target[0] + R[7] * target[1] + R[8] * target[2];
    target[0] = centerOfMass[0] + temp[0];
    target[1] = centerOfMass[1] + temp[1];
    target[2] = centerOfMass[2] + temp[2];
  }
}

