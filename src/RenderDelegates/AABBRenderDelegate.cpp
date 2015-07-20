#include "Core/Geometry.h"
#include "Core/RenderDelegate.h"
#include "Core/Factory.h"

class AABBRenderDelegate : public RenderDelegate
{
public:
  virtual void draw() const override;
};

void AABBRenderDelegate::draw() const
{
  AABB* geom = this->getSourceGeometryAs<AABB>();
  if (!geom)
    return;

  glBegin(GL_LINES);
    {
    glVertex3d(geom->aabbMin[0], geom->aabbMin[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMin[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMax[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMax[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMin[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMin[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMax[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMax[1], geom->aabbMax[2]);

    glVertex3d(geom->aabbMin[0], geom->aabbMin[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMax[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMin[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMax[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMin[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMax[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMin[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMax[1], geom->aabbMax[2]);

    glVertex3d(geom->aabbMin[0], geom->aabbMin[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMin[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMax[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMax[1], geom->aabbMin[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMin[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMin[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMin[0], geom->aabbMax[1], geom->aabbMax[2]);
    glVertex3d(geom->aabbMax[0], geom->aabbMax[1], geom->aabbMax[2]);
    }
  glEnd();
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_aabb_render_delegate)
    SIMMEDTK_REGISTER_CLASS(RenderDelegate,RenderDelegate,AABBRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
