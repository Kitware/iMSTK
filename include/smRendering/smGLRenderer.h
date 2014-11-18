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


class smGLRenderer:public smCoreClass{

public:
	smGLRenderer();
	static void drawSurfaceMeshTriangles(smMesh *p_surfaceMesh,smRenderDetail *renderDetail, smDrawParam p_drawParam);
	static void drawNormals(smMesh *p_mesh,smColor p_color=smColor::colorBlue);
	static void drawLineMesh(smLineMesh *p_lineMesh,smRenderDetail *renderDetail);
	//static void drawCoordSystem(smVec3<smFloat> p_pos,smVec3<smFloat> xDirection,smVec3<smFloat> yDirection,smVec3<smFloat> zDirection,smFloat p_scale=1.0,smInt p_subDivisions=15, smFloat p_length=20.0);
	static void drawCoordSystem(smViewer *viewer,QString p_name,smVec3<smFloat> p_pos,smVec3<smFloat> dirX,smVec3<smFloat> dirY,smVec3<smFloat> dirZ);
	static void beginTriangles();

	static void drawTriangle(smVec3<smFloat> &p_1,smVec3<smFloat> &p_2,smVec3<smFloat> &p_3);

	static void endTriangles();
	static void draw( smAABB &aabb,smColor p_color=smColor(1.0,1.0,1.0));
	static void draw(smPlane &p_plane,smFloat p_scale=10.0, smColor p_color=smColor::colorYellow);
	static void enableDefaultGLRendering();






};




#endif