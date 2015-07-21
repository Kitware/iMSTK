#include "Core/Geometry.h"
#include "Core/RenderDelegate.h"
#include "Simulators/PBDSceneObject.h"

class PBDSurfaceRenderDelegate : public RenderDelegate
{
public:
  virtual void draw() const override;
};

void PBDSurfaceRenderDelegate::draw() const
{
  SurfaceMesh* mesh = this->getSourceGeometryAs<PBDSurfaceSceneObject>()->mesh;
  mesh->draw();
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_pbd_surface_render_delegate)
    SIMMEDTK_REGISTER_CLASS(RenderDelegate,RenderDelegate,PBDSurfaceRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
