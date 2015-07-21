#include "Core/RenderDelegate.h"
#include "Core/Factory.h"
#include "Collision/MeshCollisionModel.h"
#include "Collision/SurfaceTree.h"

class SurfaceTreeRenderDelegate : public RenderDelegate
{
public:
  virtual void draw() const override;

  template<typename T>
  bool drawTree() const;
};

/// \brief Draw the surface tree.
void SurfaceTreeRenderDelegate::draw() const
{
  if (this->drawTree<SurfaceTree<MeshCollisionModel::AABBNodeType>>())
    return;
  if (this->drawTree<SurfaceTree<OctreeCell>>())
    return;
}

template<typename T>
bool SurfaceTreeRenderDelegate::drawTree() const
{
  T* geom = this->getSourceGeometryAs<T>();
  if (!geom)
    return false;

  core::Vec3d center;
  glColor3fv(Color::colorGreen.toGLColor());

  glEnable(GL_LIGHTING);
  glPushAttrib(GL_LIGHTING_BIT);
  glColor3fv(Color::colorGreen.toGLColor());
  glColor3fv(Color::colorBlue.toGLColor());

  glPushMatrix();
  glColor3fv(Color::colorPink.toGLColor());

  geom->getRoot()->draw();

  glPopMatrix();


  glPopAttrib();
  glEnable(GL_LIGHTING);
  return true;
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_surface_tree_render_delegate)
    SIMMEDTK_REGISTER_CLASS(RenderDelegate,RenderDelegate,SurfaceTreeRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
