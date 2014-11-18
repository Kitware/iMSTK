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
\date       05/2009
\bug	    None yet
\brief	    This Module is for Viewer. This class is the Rendering module of the SDK.
\brief      Module is in test now

*****************************************************
*/
#ifndef SMVIEWER_H 
#define SMVIEWER_H 
#include <QGLViewer/qglviewer.h>
#include <GL/glut.h>
#include <QDrag>
#include <QUrl>
#include <QDialog>

#include "smCore/smConfig.h"
#include "smShader/smShader.h"
#include "smCore/smScene.h"
#include "smCore/smDispatcher.h"
#include "smCore/smModule.h"
#include "smCore/smStaticSceneObject.h"
#include "smUtilities/smGLUtils.h"
#include "smUtilities/smMatrix44.h"
#include "smRendering/smVBO.h"
#include "smCore/smEventData.h"
#include "smCore/smEventHandler.h"
#include "smSimulators/smPBDSceneObject.h"
#include "smSimulators/smFemSceneObject.h"
#include "smUtilities/smDataStructs.h"
#include "smSimulators/smStylusObject.h"
#include "smRendering/smLight.h"
#include "smCore/smDoubleBuffer.h"
#include "smRendering/smFrameBuffer.h"
#include "smUtilities/smVec3.h"
#include "smShader/SceneTextureShader.h"

//includes are commented for successful compilation on my side..Tansel.9/11/2013
//#include "smATC3DGInterface.h" // Trakstar

//forward declaration
class smSDK;
class smOpenGLWindowStream;
class MetalShader;
class smCameraCollisionInterface{
public:
	virtual bool checkCameraCollision()=0;
}; 


enum smRenderingStageType{
	SMRENDERSTAGE_SHADOWPASS,
	SMRENDERSTAGE_DPMAPPASS,
	SMRENDERSTAGE_CUSTOMPASS,
	SMRENDERSTAGE_FINALPASS
};

///Viewer Class. Right now it is of type QGLViewer, which could be changed later on if needed.
class smViewer : public QGLViewer,public smModule,public smEventHandler{
protected:

	vector<smCoreClass*> objectList;
	smIndiceArray<smLight*> *lights;

	///Vertex Buffer objects
	smVBO *vboDynamicObject;
	smVBO * vboStaticObject;
	smShader shader;
	smShader shader1;
	GLuint paramColor;
	smErrorLog *log;

	//virtual void drawSurfaceMeshTriangles(smSurfaceMesh *p_surfaceMesh,smRenderDetail *renderDetail);
	virtual void drawSurfaceMeshTriangles(smMesh *p_surfaceMesh,smRenderDetail *renderDetail);

	virtual void drawSMStaticObject(smStaticSceneObject *p_smPhsyObject);
	virtual void drawSurfaceMeshTrianglesVBO(smSurfaceMesh *p_surfaceMesh,smRenderDetail *renderDetail,smInt p_objectId,smVBOType p_VBOType);
	void drawSMDeformableObject(smPBDSurfaceSceneObject *p_smPhsyObject);
	//virtual void drawPhysXObject(smPhysXObject *p_phsyXObject);
	void drawFemObject(smFemSceneObject *p_smFEM);
	void drawNormals(smMesh *p_mesh);
	friend class smSDK;
	smMatrix44<smFloat> shadowMatrix;
	smColor shadowColor;

	///Frame Buffer for Shadow rendering
	smFrameBuffer *fbo;
	smFrameBuffer *backfbo;
	smFrameBuffer *frontfbo;

	///Shadow Shader
	smShader *shadow;
	smGLInt shadowMapUniform;
	void drawSmLight(smLight *light);
	smInt unlimitedFPSVariableChanged;
	smBool unlimitedFPSEnabled;
	smInt screenResolutionWidth;
	smInt screenResolutionHeight;

public:
	smRenderingStageType renderStage; 
	smBool boostViewer;

	///if the camera motion is enabled from other external devices 
	smBool enableCameraMotion;

	//it is public for now
	smOpenGLWindowStream *windowOutput;
	///Viewer settings
	smUInt viewerRenderDetail;
	smBool consoleDisplay;
	bool isCameraCollided;
	smInt addLight(smLight *p_light);
	smBool setLight(smInt lightId,smLight *p_light);
	void refreshLights();
	smBool updateLight(smInt p_lightId,smLight *p_light);
	void setLightPos(smInt p_lightId,smLightPos p_pos);
	void setLightPos(smInt p_lightId,smLightPos p_pos,smVec3<smFloat> p_direction);
	void setUnlimitedFPS(smBool p_enableFPS);
	smViewer(smErrorLog *log);
	virtual  void init();
	void exitViewer();
	void addObject(smCoreClass *object);
	void addText(QString p_tag);
	void updateText(QString p_tag,QString p_string);
	void updateText(smInt p_handle,QString p_string);
	void setScreenResolution(smInt p_width,smInt p_height);
	void setSceneAsTextureShader(SceneTextureShader *p_shader);
	smColor defaultDiffuseColor;
	smColor defaultAmbientColor;
	smColor defaultSpecularColor;
	///if you want to put offset in camera movement
	smDouble offsetAngle_Direction;
	smDouble offsetAngle_UpDirection;
	smDouble offsetAngle_rightDirection;
	smVec3d finalDeviceCameraDir;
	smVec3d finalDeviceUpCameraDir;
	smVec3d finalDeviceRightCameraDir;

	// TrakStar ----------------------------------------------------------------------------------------------//
	///Pipe to receive the trakstar information
#ifdef smATC3DGInterface__
	smPipeRegisteration trakstarpipeReg;  //Trakstar
	smFloat trakStar_caliRotMat[9];
	smFloat trakStar_curRotMat[9];
	smFloat trakStar_mvRotMat[9];
	smVec3f trakStar_caliPos;
	smVec3f trakStar_offPos;
	smVec3f trakStar_curPos;
	smBool trakStar_enable;

	void processTrakstarData();
#endif
	// ------------------------------------------------------------------------------------------------------------//

protected :
	void initDepthBuffer();
	///Set the color and other viewer defaults
	void setToDefaults();
	///Texture for shadow mapping
	smGLUInt shadowMapTexture;
	virtual void beginFrame();
	virtual void endFrame();
	virtual void draw();
	void adjustFPS();
	virtual void renderScene(smDrawParam p_param);
	void drawWithShadows(smDrawParam &p_param);
	void renderTextureOnView();
	void drawConsole();
	void keyPressEvent(QKeyEvent *e);
	void handleEvent(smEvent *p_event);
	void enableLights();
	smFloat lightDrawScale;
	//delete this..this is for demo..
	smVec3<smDouble> hapticPosition;
	smVec3<smDouble>  hapticForce;
	void dropEvent(QDropEvent *event);
	void dragEnterEvent(QDragEnterEvent *event);
	virtual void exec();

public:
	smVec3<smDouble> deviceCameraPos;
	smVec3<smDouble> deviceCameraDir;
	smVec3<smDouble> deviceCameraUpDir;
	qglviewer::Camera prevCamera;
	smBool  checkCameraCollisionWithScene();
	void addCollisionCheckMeshes(smMesh *mesh);
	vector<smMesh*> collisionMeshes;
	smFloat cameraRadius;
	smBool prevState_collided;
	smVec3f lastCamPos;
	smBool checkCameraCollision;
	smCameraCollisionInterface *notes_cameraCollision;
	MetalShader *renderandreflection;
	vector<QDialog *>forms;
	void addForm(QDialog *p_form);
};

#endif