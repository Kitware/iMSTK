#include "smCore/smGeometry.h"
#include "smCore/smRenderDelegate.h"
#include "smCore/smFactory.h"

class smAABBRenderDelegate : public smRenderDelegate
{
public:
  virtual void draw() const override;
};

void smAABBRenderDelegate::draw() const
{
  smAABB* geom = this->getSourceGeometryAs<smAABB>();
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

SIMMEDTK_REGISTER_CLASS(smRenderDelegate,smRenderDelegate,smAABBRenderDelegate,2000);
