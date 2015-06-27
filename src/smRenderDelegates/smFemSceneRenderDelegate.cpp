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

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_fem_scene_render_delegate)
    SIMMEDTK_REGISTER_CLASS(smRenderDelegate,smRenderDelegate,smFemSceneRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
