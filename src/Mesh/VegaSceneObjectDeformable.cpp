#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "VegaSceneObjectDeformable.h"

smVegaSceneObjectDeformable::smVegaSceneObjectDeformable(char * filenameOBJ):
    smVegaSceneObjectWithRestPosition(filenameOBJ)
{
}

smVegaSceneObjectDeformable::~smVegaSceneObjectDeformable()
{
}

void smVegaSceneObjectDeformable::ResetDeformationToRest()
{
  for(int i = 0; i < n; i++)
      mesh->setPosition(i, Vec3d(restPosition[3 * i + 0], restPosition[3 * i + 1], restPosition[3 * i + 2]));
}

void smVegaSceneObjectDeformable::AddVertexDeformations(double * u)
{
  for(int i = 0; i < n; i++)
  {
    mesh->setPosition(i, mesh->getPosition(i) + Vec3d(u[3 * i + 0], u[3 * i + 1], u[3 * i + 2]));
  }
}

void smVegaSceneObjectDeformable::SetVertexDeformations(double * u)
{
  for(int i = 0; i < n; i++)
  {
    mesh->setPosition(i, Vec3d(restPosition[3 * i + 0] + u[3 * i + 0], restPosition[3 * i + 1] + u[3 * i + 1], restPosition[3 * i + 2] + u[3 * i + 2]));
  }
}

void smVegaSceneObjectDeformable::SetVertexDeformations(float * u)
{
  // set the deformations
  for(int i = 0; i < n; i++)
  {
      mesh->setPosition(i, mesh->getPosition(i) + Vec3d(restPosition[3 * i + 0] + u[3 * i + 0], restPosition[3 * i + 1] + u[3 * i + 1], restPosition[3 * i + 2] + u[3 * i + 2]));
  }
}
