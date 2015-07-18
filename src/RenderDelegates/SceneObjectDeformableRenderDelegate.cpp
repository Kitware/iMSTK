#include "Core/RenderDelegate.h"
#include "Core/Factory.h"
#include "Simulators/SceneObjectDeformable.h"

/// \brief  Displays the fem object with primary or secondary mesh, fixed vertices,
///  vertices interacted with, ground plane etc.
class smSceneObjectDeformableRenderDelegate : public smRenderDelegate
{
public:
  virtual void draw() const override;
};

void smSceneObjectDeformableRenderDelegate::draw() const
{
  auto geom = this->getSourceGeometryAs<smSceneObjectDeformable>();
  if (!geom)
    return;

  if (geom->renderSecondaryMesh && !!geom->getSecondarySurfaceMesh())
    {
    geom->getSecondarySurfaceMesh()->draw();
    }
  else
    {
    geom->getPrimarySurfaceMesh()->draw();
    }
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_scene_object_deformable_render_delegate)
    SIMMEDTK_REGISTER_CLASS(smRenderDelegate,smRenderDelegate,smSceneObjectDeformableRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
