#include "Core/Geometry.h"
#include "Core/RenderDelegate.h"
#include "Core/Factory.h"
#include "Mesh/Mesh.h"

class smMeshRenderDelegate : public RenderDelegate
{
public:
  virtual void draw() const override;
  virtual bool isTargetTextured() const override;
};

void smMeshRenderDelegate::draw() const
{
  auto geom = this->getSourceGeometryAs<smMesh>();
  if (!geom)
    return;

  auto mesh = std::dynamic_pointer_cast<smMesh>(geom->shared_from_this());
  smGLRenderer::drawSurfaceMeshTriangles(mesh, geom->getRenderDetail());

  if (geom->getRenderDetail()->renderType & SIMMEDTK_RENDER_NORMALS)
    {
    smGLRenderer::drawNormals(mesh,
      geom->getRenderDetail()->normalColor,
      geom->getRenderDetail()->normalLength);
    }
}

bool smMeshRenderDelegate::isTargetTextured() const
{
  auto geom = this->getSourceGeometryAs<smMesh>();
  if (!geom)
    return false;

  return geom->isMeshTextured();
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_mesh_render_delegate)
    SIMMEDTK_REGISTER_CLASS(RenderDelegate,RenderDelegate,smMeshRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
