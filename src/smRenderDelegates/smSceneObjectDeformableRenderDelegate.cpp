#include "smCore/smRenderDelegate.h"
#include "smCore/smFactory.h"
#include "smSimulators/smSceneObjectDeformable.h"

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

SIMMEDTK_REGISTER_CLASS(smRenderDelegate,smRenderDelegate,smSceneObjectDeformableRenderDelegate,2000);
