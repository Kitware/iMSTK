#include "smCore/smGeometry.h"
#include "smCore/smRenderDelegate.h"
#include "smSimulators/smPBDSceneObject.h"

class smPBDSurfaceRenderDelegate : public smRenderDelegate
{
public:
  virtual void draw() const override;
};

void smPBDSurfaceRenderDelegate::draw() const
{
  smSurfaceMesh* mesh = this->getSourceGeometryAs<smPBDSurfaceSceneObject>()->mesh;
  mesh->draw();
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_pbd_surface_render_delegate)
    SIMMEDTK_REGISTER_CLASS(smRenderDelegate,smRenderDelegate,smPBDSurfaceRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
