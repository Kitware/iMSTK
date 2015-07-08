#include "smCore/smGeometry.h"
#include "smCore/smRenderDelegate.h"
#include "smCore/smFactory.h"

class smPlaneRenderDelegate : public smRenderDelegate
{
public:
  virtual void draw() const override;
};

void smPlaneRenderDelegate::draw() const
{
  smGLRenderer::draw(*this->getSourceGeometryAs<smPlane>());
//         if (this->movedOrRotated)
//         {
//             updateDrawPoints();
//         }
//         glEnable(GL_LIGHTING);
//
//         glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, smColor::colorGray.toGLColor());
//         glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, smColor::colorWhite.toGLColor());
//         glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, smColor::colorGray.toGLColor());
//
//         glEnable(GL_BLEND);
//         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//         glPushMatrix();
//             glBegin(GL_QUADS);
//                 glVertex3f(this->drawPoints[0][0], this->drawPoints[0][1], this->drawPoints[0][2]);
//                 glVertex3f(this->drawPoints[1][0], this->drawPoints[1][1], this->drawPoints[1][2]);
//                 glVertex3f(this->drawPoints[2][0], this->drawPoints[2][1], this->drawPoints[2][2]);
//                 glVertex3f(this->drawPoints[3][0], this->drawPoints[3][1], this->drawPoints[3][2]);
//             glEnd();
//         glPopMatrix();
//
//         glDisable(GL_BLEND);
//
//         glEnable(GL_LIGHTING);

}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_plane_render_delegate)
    SIMMEDTK_REGISTER_CLASS(smRenderDelegate,smRenderDelegate,smPlaneRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
