#include "smCore/smStaticSceneObject.h"
#include "smCore/smRenderDelegate.h"
#include "smCore/smFactory.h"

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

SIMMEDTK_REGISTER_CLASS(smRenderDelegate,smRenderDelegate,smStaticSceneObjectRenderDelegate,2000);
