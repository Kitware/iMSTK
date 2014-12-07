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

/// \brief smoke states
enum SMOKESTATE{
	SMOKE_NOTSTARTARTED,
	SMOKE_START,
	SMOKE_CONTINUES,
	SMOKE_END,
	SMOKE_FADE
};

/// \brief smoke audio play. Used for generating buzzing sound while cauterization
class SmokeAudio{
private:
	/// \brief audio device 
	AudioDevicePtr device;
	/// \brief soude stream
	OutputStreamPtr sound;

public:
	/// \brief Constructor for smoke audio. Initializes the sound stream with file name
	SmokeAudio(char *fileName ){
		device=AudioDevicePtr(OpenDevice());
		sound=OutputStreamPtr(OpenSound(device,fileName, false));
	}
	/// \brief when called plays the audio stream
	void play(){
		if(sound->isPlaying())
			return;
		if(sound!=NULL){
			sound->setRepeat(true);

			sound->play();
			sound->setVolume(1.0f);
		}
	}
	/// \brief stops playing audio stream
	void stop(){
		if(sound!=NULL){
			if(sound->isPlaying())
				sound->stop();
		}
	}
};

class FrameGrabber;
class smSpark;

/// \brief smoke video getching
class SmokeVideo{
	/// \brief User Defined Variables
	float angle;											// Used For Rotation
	int next;												// Used For Animation
	int effect;												// Current Effect
	bool sp;												// Space Bar Pressed?
	bool env;											    // Environment Mapping (Default On)
	bool ep;												// 'E' Pressed?
	bool bg;											    // Background (Default On)
	bool bp;												// 'B' Pressed?
	int smokeType[SMOKE_MAXTEXTURES];	 //0 zero is environment which is default,smoke 1 is source	smoke
	/// \brief width of video
	int width;
	/// \brief height of video
	int height;
	/// \brief  data for video frame
	volatile uchar *pdata2[SMOKE_TEXTURES];
	/// \brief lasf frame id
	long lastframe[SMOKE_TEXTURES];									// Last Frame Of The Stream
	/// \brief data for video frame
	unsigned char* data[SMOKE_TEXTURES];
	/// \brief frame grapper for the smoke video
	FrameGrabber *frameGrapper;
	/// \brief size of the smoke texture frame
	float quadSourceSmokeScale;
	/// \brief size of the screen smoke texture frame
	float quadEnvironmentSmokeScale;

public:
	/// \brief initialization
	void Init();
	/// \brief second initialization routine 
	void Init2();
	/// \brief constructor of smoke video. Deallocates the video streams
	~SmokeVideo();
	/// \brief constructor
	SmokeVideo();
	/// \brief Any User DeInitialization Goes Here
	void Deinitialize (void);

	bool Initialize ();
	/// \brief frame graper initialization 
	void  initFrameGraper();
	/// \brief set smoke types 
	void setSmokeTypes();

	/// \brief draw smoke video
	void drawSmokeVideo(bool grapFrame, bool render, float fadeEffect, float toolTipX,
                         float toolTipY, float depth, float, float, float,smDrawParam p_params);
	/// \brief spark object pointer
	smSpark *spark;

private:
	/// \brief flipping is necessary for some video types.
	void flipIt(void* buffer);
	/// \brief opens AVI format put in specified texture number
	void OpenAVI2(char* szFile, int textureNumber);
	/// \brief grab the avi from and assign it to the  texturenumber(textureNumber)
	void GrabAVIFrame(int textureNumber);
	/// \brief close the AVI
	void CloseAVI(void);

	friend class FrameGrabber;
};

/// \brief it is shader change to smShader later on
class smSpark{

public:
	/// \brief enables the spark
	smBool isSparkEnabled;
	/// \brief spark construction
	smSpark(){
		isSparkEnabled=false;
	}
	/// \brief surface mesh needed for spark generation. Spark requires mesh to compute the proxt to the haptic pos
	vector<smSurfaceMesh *>enabledMeshes;
	/// \brief loads and generates the spark shader
	void createSparkShader();
	/// \brief renders spark shader 
	void drawSpark(float p_toolX,float p_toolY,float p_toolZ,smDrawParam p_params);
	/// \brief attach mesh to the spark
	void addSparkMesh(smSurfaceMesh *p_meshes){
		enabledMeshes.push_back(p_meshes);
	}
	/// \brief finds if the tool tip is close to the surface mesh that is attached. 
	smBool isToolCloseToTissue(smVec3f p_tool);
};

#endif