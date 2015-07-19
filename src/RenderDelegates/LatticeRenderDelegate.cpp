#include "Mesh/Lattice.h"
#include "Core/RenderDelegate.h"
#include "Core/Factory.h"

class smLatticeRenderDelegate : public smRenderDelegate
{
public:
  virtual void draw() const override;
};

void smLatticeRenderDelegate::draw() const
{
  smLattice* geom = this->getSourceGeometryAs<smLattice>();
  if (!geom)
    return;

  int index = 0;
  int index2 = 0;
  int latticeMode;
  latticeMode = SIMMEDTK_SMLATTICE_CELLPOINTSLINKS;

  if ( geom->cells == NULL || latticeMode == SIMMEDTK_SMLATTICE_NONE )
    {
    return;
    }

  glMatrixMode( GL_MODELVIEW );
  glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, reinterpret_cast<GLfloat*>(&smColor::colorYellow));

  if ( latticeMode & SIMMEDTK_SMLATTICE_SEPERATIONLINES )
    {
    for ( int j = 0; j < geom->ySeperation; j++ )
      {
      glDisable( GL_LIGHTING );
      glColor3fv(reinterpret_cast<GLfloat*>(&smColor::colorWhite));

      glBegin( GL_LINES );

      for ( int i = 0; i < geom->xSeperation; i++ )
        {
        index = i + j * geom->xSeperation * geom->zSeperation;
        index2 = index + geom->xSeperation * ( geom->zSeperation - 1 );
        glVertex3d( geom->cells[index].cellLeftCorner[0],
          geom->cells[index].cellLeftCorner[1],
          geom->cells[index].cellLeftCorner[2] - 4 * geom->zStep );
        glVertex3d( geom->cells[index2].cellLeftCorner[0],
          geom->cells[index2].cellLeftCorner[1],
          geom->cells[index2].cellLeftCorner[2] + 4 * geom->zStep );
        }

      for ( int i = 0; i < geom->zSeperation; i++ )
        {
        index = i * geom->xSeperation + j * geom->xSeperation * geom->zSeperation;
        index2 = index + ( geom->xSeperation - 1 );
        glVertex3d( geom->cells[index].cellLeftCorner[0] - 4 * geom->xStep,
          geom->cells[index].cellLeftCorner[1],
          geom->cells[index].cellLeftCorner[2] );
        glVertex3d( geom->cells[index2].cellLeftCorner[0] + 4 * geom->xStep,
          geom->cells[index2].cellLeftCorner[1],
          geom->cells[index2].cellLeftCorner[2] );
        }

      glEnd();
      }

    glEnable( GL_LIGHTING );
    glPopMatrix();
    }

  if ( latticeMode & (SIMMEDTK_SMLATTICE_CELLPOINTS | SIMMEDTK_SMLATTICE_CELLPOINTSLINKS) )
    {
    for ( int y = 0; y < geom->ySeperation; y++ )
      for ( int z = 0; z < geom->zSeperation; z++ )
        for ( int x = 0; x < geom->xSeperation; x++ )
          {

          index = x + z * geom->xSeperation + y * geom->xSeperation * geom->zSeperation;

          if ( latticeMode & SIMMEDTK_SMLATTICE_CELLPOINTSLINKS )
            {
            glDisable( GL_LIGHTING );
            glDisable( GL_TEXTURE_2D );

            glEnable( GL_COLOR_MATERIAL );

            glBegin( GL_LINE_STRIP );
            glColor3fv( reinterpret_cast<GLfloat*>(&smColor::colorWhite));
            glVertex3dv( geom->cells[index].cellLeftCorner.data() );
            glVertex3d( geom->cells[index].cellLeftCorner[0] + geom->xStep,
              geom->cells[index].cellLeftCorner[1],
              geom->cells[index].cellLeftCorner[2] );
            glVertex3d( geom->cells[index].cellLeftCorner[0] + geom->xStep,
              geom->cells[index].cellLeftCorner[1],
              geom->cells[index].cellLeftCorner[2] + geom->zStep );
            glVertex3d( geom->cells[index].cellLeftCorner[0],
              geom->cells[index].cellLeftCorner[1],
              geom->cells[index].cellLeftCorner[2] + geom->zStep );
            glVertex3dv( geom->cells[index].cellLeftCorner.data());

            glVertex3d( geom->cells[index].cellLeftCorner[0],
              geom->cells[index].cellLeftCorner[1] + geom->yStep,
              geom->cells[index].cellLeftCorner[2] );
            glVertex3d( geom->cells[index].cellLeftCorner[0] + geom->xStep,
              geom->cells[index].cellLeftCorner[1] + geom->yStep,
              geom->cells[index].cellLeftCorner[2] );
            glVertex3d( geom->cells[index].cellLeftCorner[0] + geom->xStep,
              geom->cells[index].cellLeftCorner[1] + geom->yStep,
              geom->cells[index].cellLeftCorner[2] + geom->zStep );
            glVertex3d( geom->cells[index].cellLeftCorner[0],
              geom->cells[index].cellLeftCorner[1] + geom->yStep,
              geom->cells[index].cellLeftCorner[2] + geom->zStep );
            glVertex3d( geom->cells[index].cellLeftCorner[0],
              geom->cells[index].cellLeftCorner[1] + geom->yStep,
              geom->cells[index].cellLeftCorner[2] );
            glEnd();

            glBegin( GL_LINES );
            glColor3fv( reinterpret_cast<GLfloat*>(&smColor::colorWhite));
            glVertex3dv( geom->cells[index].cellLeftCorner.data() );
            glVertex3d( geom->cells[index].cellLeftCorner[0],
              geom->cells[index].cellLeftCorner[1] + geom->yStep,
              geom->cells[index].cellLeftCorner[2] );

            glVertex3d( geom->cells[index].cellLeftCorner[0] + geom->xStep,
              geom->cells[index].cellLeftCorner[1],
              geom->cells[index].cellLeftCorner[2] );
            glVertex3d( geom->cells[index].cellLeftCorner[0] + geom->xStep,
              geom->cells[index].cellLeftCorner[1] + geom->yStep,
              geom->cells[index].cellLeftCorner[2] );

            glVertex3d( geom->cells[index].cellLeftCorner[0] + geom->xStep,
              geom->cells[index].cellLeftCorner[1],
              geom->cells[index].cellLeftCorner[2] + geom->zStep );
            glVertex3d( geom->cells[index].cellLeftCorner[0] + geom->xStep,
              geom->cells[index].cellLeftCorner[1] + geom->yStep,
              geom->cells[index].cellLeftCorner[2] + geom->zStep );

            glVertex3d( geom->cells[index].cellLeftCorner[0],
              geom->cells[index].cellLeftCorner[1],
              geom->cells[index].cellLeftCorner[2] + geom->zStep );
            glVertex3d( geom->cells[index].cellLeftCorner[0],
              geom->cells[index].cellLeftCorner[1] + geom->yStep,
              geom->cells[index].cellLeftCorner[2] + geom->zStep );
            glEnd();

            glEnable( GL_LIGHTING );
            }
          }
    }

  if ( latticeMode & SIMMEDTK_SMLATTICE_MINMAXPOINTS )
    {
    glPushMatrix();
    glPushMatrix();
    glTranslatef( geom->cells[0].cellLeftCorner[0], geom->cells[0].cellLeftCorner[1], geom->cells[0].cellLeftCorner[2] );
    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, reinterpret_cast<GLfloat*>(&smColor::colorYellow));
    //         glutSolidSphere( 2, 20, 20 );
    glPopMatrix();

    glPushMatrix();
    glTranslatef( geom->cells[geom->totalCells - 1].cellRightCorner[0],
      geom->cells[geom->totalCells - 1].cellRightCorner[1],
      geom->cells[geom->totalCells - 1].cellRightCorner[2] );
    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, reinterpret_cast<GLfloat*>(&smColor::colorRed));
    //         glutSolidSphere( 2, 20, 20 );
    glPopMatrix();
    glPopMatrix();
    }
}

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_lattice_render_delegate)
    SIMMEDTK_REGISTER_CLASS(smRenderDelegate,smRenderDelegate,smLatticeRenderDelegate,2000);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()
