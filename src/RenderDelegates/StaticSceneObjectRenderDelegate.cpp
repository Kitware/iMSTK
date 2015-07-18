#include "Core/StaticSceneObject.h"
#include "Core/RenderDelegate.h"
#include "Core/Factory.h"

class smStaticSceneObjectRenderDelegate : public smRenderDelegate
{
public:
  virtual void draw() const override;
};

void smStaticSceneObjectRenderDelegate::draw() const
{
  smStaticSceneObject* geom = this->getSourceGeometryAs<smStaticSceneObject>();
  if (!geom)
    return;

  geom->getModel()->draw();
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_static_scene_object_render_delegate)
    SIMMEDTK_REGISTER_CLASS(smRenderDelegate,smRenderDelegate,smStaticSceneObjectRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
