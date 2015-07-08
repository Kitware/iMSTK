#include "smCore/smRenderDelegate.h"
#include "smCore/smFactory.h"
#include "smCollision/smMeshCollisionModel.h"
#include "smCollision/smSurfaceTree.h"

class smSurfaceTreeRenderDelegate : public smRenderDelegate
{
public:
  virtual void draw() const override;

  template<typename T>
  bool drawTree() const;
};

/// \brief Draw the surface tree.
void smSurfaceTreeRenderDelegate::draw() const
{
  if (this->drawTree<smSurfaceTree<smMeshCollisionModel::AABBNodeType>>())
    return;
  if (this->drawTree<smSurfaceTree<smOctreeCell>>())
    return;
}

template<typename T>
bool smSurfaceTreeRenderDelegate::drawTree() const
{
  T* geom = this->getSourceGeometryAs<T>();
  if (!geom)
    return false;

  smVec3d center;
  double length;
  glColor3fv(smColor::colorGreen.toGLColor());

  glEnable(GL_LIGHTING);
  glPushAttrib(GL_LIGHTING_BIT);
  glColor3fv(smColor::colorGreen.toGLColor());
  glColor3fv(smColor::colorBlue.toGLColor());

  glPushMatrix();
  glColor3fv(smColor::colorPink.toGLColor());

  geom->getRoot()->draw();

  glPopMatrix();


  glPopAttrib();
  glEnable(GL_LIGHTING);
  return true;
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_surface_tree_render_delegate)
    SIMMEDTK_REGISTER_CLASS(smRenderDelegate,smRenderDelegate,smSurfaceTreeRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
