#include "Core/Geometry.h"
#include "Core/RenderDelegate.h"
#include "Core/Factory.h"
#include "Mesh/Mesh.h"

class MeshRenderDelegate : public RenderDelegate
{
public:
  virtual void draw() const override;
  virtual bool isTargetTextured() const override;
};

void MeshRenderDelegate::draw() const
{
  auto geom = this->getSourceGeometryAs<Mesh>();
  if (!geom)
    return;

  auto mesh = std::dynamic_pointer_cast<Mesh>(geom->shared_from_this());
  GLRenderer::drawSurfaceMeshTriangles(mesh, geom->getRenderDetail());

  if (geom->getRenderDetail()->renderType & SIMMEDTK_RENDER_NORMALS)
    {
    GLRenderer::drawNormals(mesh,
      geom->getRenderDetail()->normalColor,
      geom->getRenderDetail()->normalLength);
    }
}

bool MeshRenderDelegate::isTargetTextured() const
{
  auto geom = this->getSourceGeometryAs<Mesh>();
  if (!geom)
    return false;

  return geom->isMeshTextured();
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_mesh_render_delegate)
    SIMMEDTK_REGISTER_CLASS(RenderDelegate,RenderDelegate,MeshRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
