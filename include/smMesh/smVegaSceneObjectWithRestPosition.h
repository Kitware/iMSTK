#ifndef _SIMMEDTK_VEGASCENEOBJECTWITHRESTPOSITION_H_
#define _SIMMEDTK_VEGASCENEOBJECTWITHRESTPOSITION_H_

#include "smMesh/smVegaSceneObject.h"

class smVegaSceneObjectWithRestPosition: public smVegaSceneObject
{
public:
  smVegaSceneObjectWithRestPosition(char * filename);
  virtual ~smVegaSceneObjectWithRestPosition();

  void GetVertexRestPositions(float * buffer);
  void GetVertexRestPositions(double * buffer);
  std::shared_ptr<double> GetVertexRestPositions() { return restPosition; }

  virtual void TransformRigidly(double * centerOfMass, double * R);

protected:
    std::shared_ptr<double> restPosition;
};

#endif

