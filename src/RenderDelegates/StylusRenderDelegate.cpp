#include "Core/Geometry.h"
#include "Core/RenderDelegate.h"
#include "Core/Factory.h"
#include "Rendering/GLUtils.h"
#include "Simulators/StylusObject.h"

class smStylusRenderDelegate : public smRenderDelegate
{
public:
  virtual void initDraw() const override;
  virtual void draw() const override;
};

void smStylusRenderDelegate::initDraw() const
{
  smStylusRigidSceneObject* geom = this->getSourceGeometryAs<smStylusRigidSceneObject>();
  smString errorText;
  tree<smMeshContainer*>::pre_order_iterator iter = geom->meshes.begin();
  smGLInt newList = glGenLists(geom->meshes.size());
  smGLUtils::queryGLError(errorText);

  smInt listCounter = 0;

  while (iter != geom->meshes.end())
    {
    glNewList(newList + listCounter, GL_COMPILE);
    iter.node->data->mesh->draw();
    glEndList();
    iter.node->data->mesh->renderingID = (newList + listCounter);
    listCounter++;
    iter++;
    }
}

void smStylusRenderDelegate::draw() const
{
  smStylusRigidSceneObject* geom = this->getSourceGeometryAs<smStylusRigidSceneObject>();
  smMatrix44d viewMatrix;

#pragma unroll

  for (smInt i = 0; i < 2; i++)
    {
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    tree<smMeshContainer*>::pre_order_iterator iter = geom->meshes.begin();
    glPushMatrix();

    if (i == 2 && geom->enableDeviceManipulatedTool)
      {
      viewMatrix = iter.node->data->currentDeviceMatrix;
      }
    else
      {
      viewMatrix = iter.node->data->currentViewerMatrix;
      }

    glMultMatrixd(viewMatrix.data());
    glCallList(iter.node->data->mesh->renderingID);
    glPopMatrix();
    iter++;

    while (iter != geom->meshes.end())
      {
      glPushMatrix();

      if (i == 2 && geom->enableDeviceManipulatedTool)
        {
        viewMatrix = iter.node->data->currentDeviceMatrix;
        }
      else
        {
        viewMatrix = iter.node->data->currentViewerMatrix;
        }

      glMultMatrixd(viewMatrix.data());
      glCallList(iter.node->data->mesh->renderingID);
      glPopMatrix();
      iter++;
      }

    glPopMatrix();
    }
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_stylus_render_delegate)
    SIMMEDTK_REGISTER_CLASS(smRenderDelegate,smRenderDelegate,smStylusRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
