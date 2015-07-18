#ifndef _SIMMEDTK_VEGASCENEOBJECTWITHRESTPOSITION_H_
#define _SIMMEDTK_VEGASCENEOBJECTWITHRESTPOSITION_H_

#include "VegaSceneObject.h"

class smVegaSceneObjectWithRestPosition: public smVegaSceneObject
{
public:
  smVegaSceneObjectWithRestPosition(char * filename);
  virtual ~smVegaSceneObjectWithRestPosition();

  virtual void TransformRigidly(double * centerOfMass, double * R);

protected:
    std::vector<double> restPosition;
};

#endif

