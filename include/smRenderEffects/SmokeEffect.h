#ifndef SMOKEEFFECT1_H
#define SMOKEEFFECT1_H

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glut.h>	// Header File For The GLu32 Library
#include <assert.h>
#include "audiere.h"
#include <QThreadPool>
#include <cv.h>
#include "highgui.h"
#include "smRendering/smViewer.h"

#define SMOKE_TEXTURES 2
#define SMOKE_MAXTEXTURES 5
#define SPARK_TEXTURES 2

using namespace audiere;

enum SMOKESTATE{
	SMOKE_NOTSTARTARTED,
	SMOKE_START,
	SMOKE_CONTINUES,
	SMOKE_END,
	SMOKE_FADE
};

class SmokeAudio{
private:
	AudioDevicePtr device;
	OutputStreamPtr sound;

public:
	SmokeAudio(char *fileName ){
		device=AudioDevicePtr(OpenDevice());
		sound=OutputStreamPtr(OpenSound(device,fileName, false));
	}

	void play(){
		if(sound->isPlaying())
			return;
		if(sound!=NULL){
			sound->setRepeat(true);

			sound->play();
			sound->setVolume(1.0f);
		}
	}

	void stop(){
		if(sound!=NULL){
			if(sound->isPlaying())
				sound->stop();
		}
	}
};

class FrameGrabber;
class smSpark;

class SmokeVideo{
	// User Defined Variables
	float angle;												// Used For Rotation
	int next;												// Used For Animation
	int effect;												// Current Effect
	bool sp;													// Space Bar Pressed?
	bool env;											// Environment Mapping (Default On)
	bool ep;													// 'E' Pressed?
	bool bg;											// Background (Default On)
	bool bp;													// 'B' Pressed?
	int smokeType[SMOKE_MAXTEXTURES];	 //0 zero is environment which is default,smoke 1 is source	smoke
	int width;
	int height;
	volatile uchar *pdata2[SMOKE_TEXTURES];	
	long lastframe[SMOKE_TEXTURES];									// Last Frame Of The Stream
	unsigned char* data[SMOKE_TEXTURES];
	FrameGrabber *frameGrapper;
	float quadSourceSmokeScale;
	float quadEnvironmentSmokeScale;

public:
	void Init();
	void Init2();
	~SmokeVideo();
	SmokeVideo();
	void Deinitialize (void);
	bool Initialize ();
	void  initFrameGraper();
	void setSmokeTypes();

	void drawSmokeVideo(bool grapFrame, bool render, float fadeEffect, float toolTipX,
                         float toolTipY, float depth, float, float, float,smDrawParam p_params);
	smSpark *spark;

private:
	void flipIt(void* buffer);
	void OpenAVI2(char* szFile, int textureNumber);
	void GrabAVIFrame(int textureNumber);
	void CloseAVI(void);

	friend class FrameGrabber;
};

///it is shader change to smShader later on/
class smSpark{

public:
	smBool isSparkEnabled;

	smSpark(){
		isSparkEnabled=false;
	}

	vector<smSurfaceMesh *>enabledMeshes;
	void createSparkShader();
	void drawSpark(float p_toolX,float p_toolY,float p_toolZ,smDrawParam p_params);

	void addSparkMesh(smSurfaceMesh *p_meshes){
		enabledMeshes.push_back(p_meshes);
	}

	smBool isToolCloseToTissue(smVec3f p_tool);
};

#endif