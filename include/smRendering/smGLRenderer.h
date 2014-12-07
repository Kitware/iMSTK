/*
****************************************************
SOFMIS LICENSE

****************************************************

\author:    <http:\\acor.rpi.edu>
SOFMIS TEAM IN ALPHABATIC ORDER
Anderson Maciel, Ph.D.
Ganesh Sankaranarayanan, Ph.D.
Sreekanth A Venkata
Suvranu De, Ph.D.
Tansel Halic
Zhonghua Lu

\author:    Module by Tansel Halic


\version    1.0
\date       04/2009
\bug	    None yet
\brief	    This Module has the common opengl Rendering functions . 

*****************************************************
*/

#ifndef SMGLRENDERER_H
#define SMGLRENDERER_H

#include <GL/glew.h>
#include <GL/glut.h>
#include <QGLViewer/qglviewer.h>

#include "smCore/smConfig.h"
#include "smUtilities/smVec3.h"
#include "smCore/smCoreClass.h"

class smMesh;
class smAABB;
class smLineMesh;
class smViewer;
class smPlane;

struct smRenderDetail;
/// \brief gl rendering utilities
class smGLRenderer:public smCoreClass{
public:
	/// \brief constructor
	smGLRenderer();
	/// \brief draws surface mesh with given rendertdail and draw paramters
	static void drawSurfaceMeshTriangles(smMesh *p_surfaceMesh,smRenderDetail *renderDetail, smDrawParam p_drawParam);
	/// \brief  draw normals
	static void drawNormals(smMesh *p_mesh,smColor p_color=smColor::colorBlue);
	/// \brief  draw line mesh
	static void drawLineMesh(smLineMesh *p_lineMesh,smRenderDetail *renderDetail);
	/// \brief draw coordinate system
	static void drawCoordSystem(smViewer *viewer,QString p_name,smVec3<smFloat> p_pos,smVec3<smFloat> dirX,smVec3<smFloat> dirY,smVec3<smFloat> dirZ);
	/// \brief begin rendering triangle
	static void beginTriangles();
	/// \brief draw triangle
	static void drawTriangle(smVec3<smFloat> &p_1,smVec3<smFloat> &p_2,smVec3<smFloat> &p_3);
	/// \brief end triangle rendering
	static void endTriangles();
	/// \brief draw AABB
	static void draw( smAABB &aabb,smColor p_color=smColor(1.0,1.0,1.0));
	/// \brief  draw plane
	static void draw(smPlane &p_plane,smFloat p_scale=10.0, smColor p_color=smColor::colorYellow);
	/// \brief switch to default rendering
	static void enableDefaultGLRendering();
};

#endif
