#include "smCore/smRenderDelegate.h"
#include "smCore/smFactory.h"
#include "smCore/smVector.h"
#include "smMesh/smPhysXVolumeMesh.h"

class smPhysXVolumeMeshRenderDelegate : public smRenderDelegate
{
public:
  virtual void draw() const override;
};

void smPhysXVolumeMeshRenderDelegate::draw() const
{
  smPhysXVolumeMesh* geom = this->getSourceGeometryAs<smPhysXVolumeMesh>();
  if (!geom)
    return;

  if (geom->renderSurface)
    {
    smGLRenderer::drawSurfaceMeshTriangles(geom->surfaceMesh, geom->getRenderDetail());
    }

  if (geom->renderTetras)
    {
    glBegin(GL_TRIANGLES);

    for (smInt i = 0; i < geom->nbrTetra; i++)
      {
      if (geom->drawTet[i])
        {
        smVec3d p0(geom->nodes[geom->tetra[i].vert[0]]);
        smVec3d p1(geom->nodes[geom->tetra[i].vert[1]]);
        smVec3d p2(geom->nodes[geom->tetra[i].vert[2]]);
        smVec3d p3(geom->nodes[geom->tetra[i].vert[3]]);

        smVec3d normal, center, v[4];
        float scale = 0.9;

        center = p0;
        v[0] = p0;
        center += p1;
        v[1] = p1;
        center += p2;
        v[2] = p2;
        center += p3;
        v[3] = p3;
        center = center * 0.25f;
        v[0] = center + (v[0] - center) * scale;
        v[1] = center + (v[1] - center) * scale;
        v[2] = center + (v[2] - center) * scale;
        v[3] = center + (v[3] - center) * scale;
        smInt sides[4][3] = {{2, 1, 0}, {0, 1, 3}, {1, 2, 3}, {2, 0, 3}};

        for (smInt side = 0; side < 4; side ++)
          {
          smVec3d &v0 = v[sides[side][0]];
          smVec3d &v1 = v[sides[side][1]];
          smVec3d &v2 = v[sides[side][2]];
          smVec3d a = v1 - v0;
          smVec3d b = v2 - v0;
          normal = a.cross(b);
          normal.normalize();
          glNormal3dv(normal.data());
          glVertex3dv(v0.data());
          glVertex3dv(v1.data());
          glVertex3dv(v2.data());
          }
        }
      }

    glEnd();
    }
}

SIMMEDTK_REGISTER_CLASS(smRenderDelegate,smRenderDelegate,smPhysXVolumeMeshRenderDelegate,2000);
