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

SIMMEDTK_REGISTER_CLASS(smRenderDelegate,smRenderDelegate,smPBDSurfaceRenderDelegate,2000);
