#ifndef _SIMMEDTK_VEGASCENEOBJECTWITHRESTPOSITION_H_
#define _SIMMEDTK_VEGASCENEOBJECTWITHRESTPOSITION_H_

#include "VegaSceneObject.h"

class VegaSceneObjectWithRestPosition: public VegaSceneObject
{
public:
  VegaSceneObjectWithRestPosition(char * filename);
  virtual ~VegaSceneObjectWithRestPosition();

  virtual void TransformRigidly(double * centerOfMass, double * R);

protected:
    std::vector<double> restPosition;
};

#endif

