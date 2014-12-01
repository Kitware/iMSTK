/*****************************************************
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
	\brief	    This header includes the rendering parameters. For each module seperate rendering
*****************************************************
*/

#ifndef SMCONFIGRENDERING_H 
#define SMCONFIGRENDERING_H 

#include "smCore/smConfig.h"

#ifdef SOFMIS_RENDERER_OPENGL
	#include <GL/glew.h>
	#include <GL/glut.h>
#endif

#include <QString>
#include <QVector>

#include "math.h"
#include "smUtilities/smVec3.h"

class smShader;

///this shows the Vertex Buffer Object Size
///It should be bigger than the total data of vertices and indices of the scene objects. 
///the value is in bytes
#define SOFMIS_VBOBUFFER_DATASIZE	500000
#define SOFMIS_VBOBUFFER_INDEXSIZE	100000
#define SOFMIS_VIEWER_MAXLIGHTS	32

#define SOFMIS_MAX_TEXTURENBR 36
#define SOFMIS_MAX_VBOBUFFERS 10

///The configuration for VBO
enum smVBOType{
	SOFMIS_VBO_STATIC,
	SOFMIS_VBO_NOINDICESCHANGE,
	SOFMIS_VBO_DYNAMIC,
};

///Vertex Buffer Objects Return Types
enum smVBOResult{
	SOFMIS_VBO_OK,
	SOFMIS_VBO_NODATAMEMORY,
	SOFMIS_VBO_NOINDEXMEMORY,
	SOFMIS_VBO_INVALIDOPERATION,
	SOFMIS_VBO_BUFFERPOINTERERROR
};

struct smColor{
	union{
		smFloat rgba[4];
		struct{
			smFloat r;
			smFloat g;
			smFloat b;
			smFloat a;
		};
	};
	smColor(){
		rgba[0]=0.8f;
		rgba[1]=0.8f;
		rgba[2]=0.8f;
		rgba[3]=1.0f;
	}
	smColor(smFloat r,smFloat g,smFloat b,smFloat a=1.0){
		rgba[0]=r;
		rgba[1]=g;
		rgba[2]=b;
		rgba[3]=a;
	}

	///Dark ratio. the valu is between 0 and 1.0
	void darken(smFloat p_darkFactor){

		rgba[0]=(rgba[1]-rgba[1]*(p_darkFactor));
		rgba[1]=(rgba[2]-rgba[2]*(p_darkFactor));
		rgba[2]=(rgba[3]-rgba[3]*(p_darkFactor));
		rgba[0]=(rgba[0]<0?0:rgba[0]);
		rgba[1]=(rgba[1]<0?0:rgba[1]);
		rgba[2]=(rgba[2]<0?0:rgba[2]);
	}

	void lighten(smFloat p_darkFactor){

		rgba[0]=rgba[1]+rgba[1]*(p_darkFactor);
		rgba[1]=rgba[2]+rgba[2]*(p_darkFactor);
		rgba[2]=rgba[3]+rgba[3]*(p_darkFactor);

		rgba[0]=(rgba[0]>1.0?1.0:rgba[0]);
		rgba[1]=(rgba[1]<1.0?1.0:rgba[1]);
		rgba[2]=(rgba[2]<1.0?1.0:rgba[2]);
	}


	smFloat operator()(smInt p_i){
		if(p_i<0||p_i>3)
			return -1;
		return rgba[p_i];
	}

	smColor &operator=(smColor &p_color){
		rgba[0]=p_color.rgba[0];      
		rgba[1]=p_color.rgba[1];      
		rgba[2]=p_color.rgba[2];      
		rgba[3]=p_color.rgba[3];      
		return *this;
	}

	smGLFloat* toGLColor(){
		return (smGLFloat*)rgba;
	}

	inline void setValue(smFloat p_red,smFloat p_green,smFloat p_blue,smFloat p_alpha){
		rgba[0]=p_red;
		rgba[1]=p_green;
		rgba[2]=p_blue;
		rgba[3]=p_alpha;
	}

	static smColor colorWhite;
	static smColor colorBlue;
	static smColor colorGreen;
	static smColor colorRed;
	static smColor colorGray;
	static smColor colorPink;
	static smColor colorYellow;
};


///renderType. this shows how the render will be done
#define    SOFMIS_RENDER_TEXTURE			(1<<1)
#define    SOFMIS_RENDER_MATERIALCOLOR		(1<<2)
#define    SOFMIS_RENDER_SHADOWS			(1<<3)
#define    SOFMIS_RENDER_COLORMAP			(1<<4)
#define    SOFMIS_RENDER_WIREFRAME			(1<<5)
#define    SOFMIS_RENDER_VERTICES			(1<<6)
#define	   SOFMIS_RENDER_HIGHLIGHTVERTICES	(1<<7)
#define    SOFMIS_RENDER_TRANSPARENT		(1<<8)
#define    SOFMIS_RENDER_LOCALAXIS			(1<<9)
#define    SOFMIS_RENDER_HIGHLIGHT			(1<<10)
#define    SOFMIS_RENDER_TETRAS				(1<<11)
#define    SOFMIS_RENDER_SURFACE			(1<<12)
#define    SOFMIS_RENDER_CUSTOMRENDERONLY	(1<<13)
#define    SOFMIS_RENDER_SHADERTEXTURE		(1<<14)
#define    SOFMIS_RENDER_FACES				(1<<15)
///Renders Vertex Buffer Objects
#define	   SOFMIS_RENDER_VBO				(1<<16)
#define    SOFMIS_RENDER_NORMALS			(1<<17)
#define    SOFMIS_RENDER_VAO				(1<<18)
#define    SOFMIS_RENDER_NONE				(1<<31)

///type definitions for variable viewerRenderDetail in smViewer 
#define    SOFMIS_VIEWERRENDER_GLOBALAXIS						(1<<1)
#define    SOFMIS_VIEWERRENDER_TEXT								(1<<2)
#define    SOFMIS_VIEWERRENDER_FADEBACKGROUND					(1<<3)
#define    SOFMIS_VIEWERRENDER_FADEBACKGROUNDIMAGE				(1<<4)
#define    SOFMIS_VIEWERRENDER_GROUND							(1<<5)
#define    SOFMIS_VIEWERRENDER_VBO_ENABLED						(1<<6)
#define    SOFMIS_VIEWERRENDER_WIREFRAMEALL						(1<<7)
#define    SOFMIS_VIEWERRENDER_TRANSPARENCY						(1<<8)
#define    SOFMIS_VIEWERRENDER_FULLSCREEN						(1<<9)
#define    SOFMIS_VIEWERRENDER_RESTORELASTCAMSETTINGS			(1<<10)
#define    SOFMIS_VIEWERRENDER_SOFTSHADOWS						(1<<11)
#define    SOFMIS_VIEWERRENDER_DISABLE							(1<<12)
#define    SOFMIS_VIEWERRENDER_DYNAMICREFLECTION				(1<<13)





struct smViewerDetail{
	smUInt environment;
	smColor backGroundColor;
};

struct smUnifiedID;


///smRenderDetail has rendering options and features.
///It shows how the mesh should be rendered
struct smRenderDetail{
public:
	smRenderDetail(){/*:shaders(10)*/
		renderType=SOFMIS_RENDER_MATERIALCOLOR|SOFMIS_RENDER_FACES;
		highLightColor.rgba[0]=1.0f;
		highLightColor.rgba[1]=0.0f;
		highLightColor.rgba[2]=0.0f;
		pointSize=1;
		lineSize=1;
		shadowColor.rgba[0]=0.0f;
		shadowColor.rgba[1]=0.0f;
		shadowColor.rgba[2]=0.0f;
		shadowColor.rgba[3]=0.5f;
		colorDiffuse=smColor::colorWhite;
		colorAmbient=smColor::colorWhite;
		colorSpecular=smColor::colorWhite;
		normalColor=smColor::colorGreen;
		wireFrameColor=smColor::colorBlue;
		shininess=50.0;
		debugDraw=false;
		castShadow=true;
		canGetShadow=true;
	}

	smUInt renderType;
	smColor colorDiffuse;
	smColor colorAmbient;
	smColor colorSpecular;
	smColor highLightColor;
	smColor shadowColor;
	smBool castShadow;
	smBool canGetShadow;
	smColor normalColor;
	smColor wireFrameColor;
	smFloat pointSize;
	smFloat lineSize;
	smFloat shininess;
	smBool debugDraw;

	void addShader(smUnifiedID p_shaderID);
	void addVAO(smUnifiedID p_shaderID);
	vector <smUnifiedID >shaders;
	vector<smChar>shaderEnable;

	vector <smUnifiedID>VAOs;
	vector<smChar>VAOEnable;
};

#endif
