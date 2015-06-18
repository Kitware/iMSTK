#include "smSimulators/smFemSceneObject.h"
#include "smCore/smRenderDelegate.h"
#include "smCore/smFactory.h"
#include "smMesh/smVolumeMesh.h"

class smFemSceneRenderDelegate : public smRenderDelegate
{
public:
  virtual void draw() const override;
};

void smFemSceneRenderDelegate::draw() const
{
  smVolumeMesh* v_mesh = this->getSourceGeometryAs<smFemSceneObject>()->v_mesh;
  smRenderDelegate::Ptr delegate = v_mesh->getRenderDelegate();
  if (delegate)
    delegate->draw();
}

SIMMEDTK_REGISTER_CLASS(smRenderDelegate,smRenderDelegate,smFemSceneRenderDelegate,2000);
