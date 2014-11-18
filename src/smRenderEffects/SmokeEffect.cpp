#include <assert.h>
#include <QThread>
#include <QRunnable>
#include <QTime>
#include <boost/chrono/chrono.hpp>
#include <boost/thread/thread.hpp>
#include "smRenderEffects/SmokeEffect.h"
#include "smUtilities/smTimer.h"
#include "smShader/smShader.h"
#include "smCore/smTextureManager.h"

using namespace std;




///BEGIN TANSEL 



//#pragma comment( lib, "glu32.lib" )								// Search For GLu32.lib While Linking
//#pragma comment( lib, "vfw32.lib" )								// Search For VFW32.lib While Linking

//#pragma comment( lib, "vfw32.lib" )								// Search For VFW32.lib While Linking



GLhandleARB vSmokeObject,fSmokeObject;
GLhandleARB programSmokeObject;

GLint locSmokeTex[SMOKE_TEXTURES];
GLint locTipX;
GLint locTipY;
GLint locDepth;
GLint locSmokeType;


GLhandleARB vSparkObject,fSparkObject;
GLhandleARB programSparkObject;


char spark_textureGPUBinding[SMOKE_TEXTURES][100]={"myPerlin","alphaMap"};
GLint locSparkTime_X;
GLint locSparkmyNoise;

//first you need declare new variables allright? are you with me?
GLint locSparkmyNoise2;
GLint locSparkmyNoise3;
GLint locSparkTex[SPARK_TEXTURES]={-1,-1};

float time_X=0.02;



 QTime timeReloadTime;
 int interval=2000;
class smSparkEffect:public smShader{



};





//char    textureGPUBinding[SMOKE_TEXTURES][100]={"myTexture"};
//char  smokeVideoFileNames[SMOKE_TEXTURES][100]={"effects/test1.avi"};

char textureGPUBinding[SMOKE_TEXTURES][100]={"myTexture","myTexture1"};
//char  smokeVideoFileNames[SMOKE_TEXTURES][100]={"effects/test1.avi","effects/oldavismoketrimmeddvixlong.avi"};
char  smokeVideoFileNames[SMOKE_TEXTURES][100]={"effects/test1_upside.avi","effects/oldavismoketrimmeddvixlong_upside.avi"};
//char  smokeVideoFileNames[SMOKE_TEXTURES][100]={"effects/oldavismoketrimmeddvixlong.avi","effects/test1.avi"};
int  smokeTypes[SMOKE_MAXTEXTURES]={1,0,0,0,0};

//char textureGPUBinding[SMOKE_TEXTURES][100]={"myTexture","myTexture1","myTexture3"};
//char  smokeVideoFileNames[SMOKE_TEXTURES][100]={"effects/smoke.avi","effects/smoke1.avi","effects/smoke2.avi"};

//uncompressed
//char textureGPUBinding[SMOKE_TEXTURES][100]={"myTexture","myTexture1","myTexture3","myTexture4"};
//char  smokeVideoFileNames[SMOKE_TEXTURES][100]={"effects/smoketest.avi","effects/smoketest1.avi","effects/smoketest2.avi","effects/smoketest3.avi"};

//compressed
//char textureGPUBinding[SMOKE_TEXTURES][100]={"myTexture","myTexture1","myTexture3","myTexture4"};
//char  smokeVideoFileNames[SMOKE_TEXTURES][100]={"effects/smoketest.avi","effects/smoketest1.avi","effects/smoketest2.avi","effects/smoketest3.avi"};
using namespace cv;
VideoCapture caps[SMOKE_MAXTEXTURES];
Mat frames[SMOKE_MAXTEXTURES];
Mat framesCopy[SMOKE_MAXTEXTURES];




GLint locFadeEffect;
GLuint texName[SMOKE_TEXTURES];
smTimer timer;
//DataRecord*dataRecordTimings=new DataRecord(Output, "Data//SMOKE.txt");
void printInfoLog_Legacy(GLhandleARB obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

	glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetInfoLogARB(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
        free(infoLog);
    }
}


char* readStringFromFile(char *fn) {

	FILE *fp;
	char *content = NULL;
	int count=0;

	if (fn != NULL) {
		fp = fopen(fn,"rt");

		if (fp != NULL) {
      
			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);

			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count+1));
				count = fread(content,sizeof(char),count,fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}


class FrameGrabber:public QThread{

	int currentIndex[SMOKE_TEXTURES];
	SmokeVideo *smokeVideo;
	int textureNumber;
	int frame;
	bool fetchNextFrame;
	smTimer timer;
	bool running;
	bool closed;


public:

	~FrameGrabber(){
		cout<<"FrameGrabber Desctructor called"<<endl;
	
	}
	FrameGrabber(SmokeVideo *p_smokeVideo){
		smokeVideo=p_smokeVideo;
		//setAutoDelete(false);
		fetchNextFrame=false;
	    frame=0;
		running=true;
		closed=false;

		




	}
	void stop(){
			running=false;
		
	}
	void closeAndWait(){
		while(1){
			if(closed==true)
				break;
		
		}
	
	}
	//set this before the 
	void setCurrentStream(int p_index,int p_frame){
		textureNumber=p_index;
		frame=p_frame;


	}
	void nextFrame(){
		fetchNextFrame=true;



	}



	


	virtual void run(){
		smTimer timer;
		smTimer timer1;
		smTimer timer2;
		int startnedn,increment,end;

		while(1){
			if(running==false)
				break;
			if(fetchNextFrame==false)
				continue;
			//cout<<"Framegrapper started.."<<endl;
			fetchNextFrame=false;
			timer.start();

			for(int i=0;i<SMOKE_TEXTURES;i++){
				textureNumber=i;
				caps[i]>>frames[i];
				if(!frames[i].data){
						caps[i].set(CV_CAP_PROP_POS_FRAMES,0);
						caps[i] >> frames[i]; // get a new frame from camera
				}
				smokeVideo->pdata2[i]=frames[i].data;
			}
			//Sleep(30);
			//std::this_thread::sleep_for(std::chrono::milliseconds(30));
			boost::this_thread::sleep(boost::posix_time::millisec(30));
		}
		closed=true;
	}
};



void createShader(){

	char *vs = NULL,*fs = NULL;


	
	programSmokeObject = glCreateProgramObjectARB();
	assert(programSmokeObject != 0);
	int errorCode = glGetError();

	vSmokeObject  = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	fSmokeObject  = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	//f2Object = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);


	vs = readStringFromFile("shaders/smokevert.cg"); // light_texture // specular //light_texture_per_vertex.vert  //bump_specular.vert
	fs = readStringFromFile("shaders/smokefrag.cg"); // light_texture // ambient //light_texture_per_vertex.frag  //bump_specular.frag

	const char * vv = vs;
	const char * ff = fs;

	glShaderSourceARB(vSmokeObject, 1, &vv, NULL);
	glShaderSourceARB(fSmokeObject, 1, &ff, NULL);

	free(vs);free(fs);

	glCompileShaderARB(vSmokeObject);
	glCompileShaderARB(fSmokeObject);
	// check if shader compiled
	GLint compiled = false;
	glGetObjectParameterivARB(vSmokeObject, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
	if (!compiled)
	{
		printInfoLog_Legacy(vSmokeObject);
		assert("something wrong during vertex shader compiled");
	}
	compiled = false;
	glGetObjectParameterivARB(fSmokeObject, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
	if (!compiled)
	{
		printInfoLog_Legacy(fSmokeObject);
		assert("something wrong during fragment shader compiled");
	}

	glAttachObjectARB(programSmokeObject, vSmokeObject);
	glAttachObjectARB(programSmokeObject, fSmokeObject);

	//// delete vertex & fragment object, no longer needed
	glDeleteObjectARB(vSmokeObject);
	glDeleteObjectARB(fSmokeObject);
	//glDeleteObjectARB(f2Object);


	glLinkProgramARB(programSmokeObject);
	GLint linked = false;
	glGetObjectParameterivARB(programSmokeObject, GL_OBJECT_LINK_STATUS_ARB, &linked);
	if (!linked)
	{

		assert("Shaders failed to link, exiting...");
	}
	// make sure position attrib is bound to location 0
	//glBindAttribLocationARB(programObject, 0, "position");

	glValidateProgramARB(programSmokeObject);
	GLint validated = false;
	glGetObjectParameterivARB(programSmokeObject, GL_OBJECT_VALIDATE_STATUS_ARB, &validated);
	if (!validated)
	{

		assert("Shaders failed to validate, exiting...");
	}	

	glUseProgramObjectARB(programSmokeObject);



	for(int i=0;i<SMOKE_TEXTURES;i++){
		locSmokeTex[i] = glGetUniformLocationARB(programSmokeObject, textureGPUBinding[i]);


	}
	locTipX=glGetUniformLocationARB(programSmokeObject, "normalizedToolTipX");
	locTipY=glGetUniformLocationARB(programSmokeObject, "normalizedToolTipY");
	locDepth=glGetUniformLocationARB(programSmokeObject, "locDepth");
	locSmokeType=glGetUniformLocationARB(programSmokeObject, "smokeType");




}

void smSpark::createSparkShader(){

	char *vs = NULL,*fs = NULL;

	programSparkObject = glCreateProgramObjectARB();
	assert(programSparkObject != 0);
	int errorCode = glGetError();

	vSparkObject  = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	fSparkObject  = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	//f2Object = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);


	vs = readStringFromFile("shaders/spark_vs.glsl"); // light_texture // specular //light_texture_per_vertex.vert  //bump_specular.vert
	fs = readStringFromFile("shaders/spark_fs.glsl"); // light_texture // ambient //light_texture_per_vertex.frag  //bump_specular.frag

	const char * vv = vs;
	const char * ff = fs;

	glShaderSourceARB(vSparkObject, 1, &vv, NULL);
	glShaderSourceARB(fSparkObject, 1, &ff, NULL);

	free(vs);free(fs);

	glCompileShaderARB(vSparkObject);
	glCompileShaderARB(fSparkObject);
	// check if shader compiled
	GLint compiled = false;
	glGetObjectParameterivARB(vSparkObject, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
	if (!compiled)
	{
		printInfoLog_Legacy(vSparkObject);
		assert("something wrong during vertex shader compiled");
	}
	compiled = false;
	glGetObjectParameterivARB(fSparkObject, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);
	if (!compiled)
	{
		printInfoLog_Legacy(fSparkObject);
		assert("something wrong during fragment shader compiled");
	}

	glAttachObjectARB(programSparkObject, vSparkObject);
	glAttachObjectARB(programSparkObject, fSparkObject);

	//// delete vertex & fragment object, no longer needed
	glDeleteObjectARB(vSparkObject);
	glDeleteObjectARB(fSparkObject);
	//glDeleteObjectARB(f2Object);


	glLinkProgramARB(programSparkObject);
	GLint linked = false;
	glGetObjectParameterivARB(programSparkObject, GL_OBJECT_LINK_STATUS_ARB, &linked);
	if (!linked)
	{

		assert("Shaders failed to link, exiting...");
	}
	// make sure position attrib is bound to location 0
	//glBindAttribLocationARB(programObject, 0, "position");

	glValidateProgramARB(programSparkObject);
	GLint validated = false;
	glGetObjectParameterivARB(programSparkObject, GL_OBJECT_VALIDATE_STATUS_ARB, &validated);
	if (!validated) {
		assert("Shaders failed to validate, exiting...");
	}
	glUseProgramObjectARB(programSparkObject);

	for(int i=0;i<SPARK_TEXTURES;i++){
		locSparkTex[i] = glGetUniformLocationARB(programSparkObject, spark_textureGPUBinding[i]);
	}
	locSparkTime_X=glGetUniformLocationARB(programSparkObject, "time_0_X");
	locSparkmyNoise=glGetUniformLocationARB(programSparkObject, "myNoise");
	locSparkmyNoise2=glGetUniformLocationARB(programSparkObject, "myNoise2");
	locSparkmyNoise3=glGetUniformLocationARB(programSparkObject, "myNoise3");
}

SmokeVideo::~SmokeVideo()
{
	delete frameGrapper;
}


SmokeVideo::SmokeVideo()
{
	for(int i=0;i<SMOKE_TEXTURES;i++)
		data[i]=0;
	spark=new smSpark();
}

smBool smSpark::isToolCloseToTissue(smVec3f p_tool){
	smInt totalMeshes;
	totalMeshes=this->enabledMeshes.size();
	smSurfaceMesh *mesh;
	isSparkEnabled=false;

	for(smInt i=0;i<totalMeshes;i++){
		mesh=enabledMeshes[i];
		for(smInt j=0;j<mesh->nbrVertices;j++){
			if(mesh->vertices[j].distance(p_tool)<2.0){
				isSparkEnabled=true;
				return isSparkEnabled;
			}
		}
	}
	return isSparkEnabled;
}

void smSpark::drawSpark(float p_toolX,float p_toolY,float p_toolZ,smDrawParam p_params){

	float random;
	float scaleX=1.0;
	float scaleY=2.0;
	float offsetY=1.0;
	float offsetZ=3.0;
	static smVec3f toolPos;
	 if (timeReloadTime.elapsed()>interval){
		timeReloadTime.start();
		createSparkShader();
    }
	toolPos.setValue(p_toolX,p_toolY,p_toolZ);
	if(!isToolCloseToTissue(toolPos))
		return;
	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
 	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	//glLoadIdentity();
	//gluPerspective(60,1.0,0.0001,1000);
	//gluOrtho2D(-1.0,1.0,-1.0,1.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	//glLoadIdentity();

	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LIGHTING);
	glDisable(GL_BLEND);
	//glDepthFunc(GL_EQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_VERTEX_PROGRAM_ARB);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glUseProgramObjectARB(programSparkObject);

	//here you need to put..these locSparkmyNoise defines the randomnnes of the shape
	glUniform1f(locSparkTime_X,time_X);
	random=(float)rand()/RAND_MAX;
	glUniform1f(locSparkmyNoise,random);
	random=(float)rand()/RAND_MAX;
	glUniform1f(locSparkmyNoise2,random);
	random=(float)rand()/RAND_MAX;
	glUniform1f(locSparkmyNoise3,random);
	time_X+=0.01;
	static int timer=1;
	static int time_MAX=10;
	timer++;
	float scaleRatio=(timer%time_MAX)/(float)time_MAX;
	if(timer>=time_MAX-1){
	  time_MAX=random*10+1;
	  timer=0;
	}

	scaleY=0.1+1.0*scaleRatio*scaleRatio;

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glUniform1iARB(locSparkTex[0], 1);
	glBindTexture(GL_TEXTURE_2D, smTextureManager::getOpenglTextureId("spknoise"));
	
	glActiveTexture(GL_TEXTURE1+1);
	glEnable(GL_TEXTURE_2D);
	glUniform1iARB(locSparkTex[1], 2);
	glBindTexture(GL_TEXTURE_2D, smTextureManager::getOpenglTextureId("alphaMap"));

		smMatrix33f rr;
		smMatrix33f rrT;
		smMatrix44f rrM;

		smFloat matrix[16];
		float randomxMove=(float)0.1*(rand()/RAND_MAX);
		float randomyMove=(float)rand()/RAND_MAX;
		int randomxSign=(float)rand();
		
		if(randomxSign%2==0)
			randomxMove=-randomxMove;
		else
			randomxMove=randomxMove;

		//p_params.rendererObject->camera()->getModelViewMatrix(matrix);
		glTranslatef(p_toolX+randomxMove,p_toolY-random*2.0,p_toolZ);
		glGetFloatv(GL_MODELVIEW_MATRIX, matrix);

		rr.setMatrixFromOpenGL(matrix);
		rrT=rr.getTranspose();
		rrT.getMatrixForOpenGL(matrix);

		smVec3f v[4]={
						smVec3f(-1.0f*scaleX,  (-1.0f-offsetY)*scaleY, -1.0f),
						smVec3f(1.0f*scaleX,   (-1.0f-offsetY)*scaleY, -1.0f),
						smVec3f(1.0f*scaleX,   (1.0f-offsetY)*scaleY, -1.0f),
						smVec3f(-1.0f*scaleX,  (1.0f-offsetY)*scaleY, -1.0f)
						};
		smVec3f d[4];
		d[0]=rrT*v[0];
		d[1]=rrT*v[1];
		d[2]=rrT*v[2];
		d[3]=rrT*v[3];

		glBegin(GL_QUADS);// Begin Drawing The Background (One Quad)
			glMultiTexCoord2f(GL_TEXTURE0_ARB,0.0f, 0.0f); glVertex3f(d[0].x,d[0].y,d[0].z);
			glMultiTexCoord2f(GL_TEXTURE0_ARB,1.0f, 0.0f); glVertex3f(d[1].x,d[1].y,d[1].z);
			glMultiTexCoord2f(GL_TEXTURE0_ARB,1.0f, 1.0f); glVertex3f(d[2].x,d[2].y,d[2].z);
			glMultiTexCoord2f(GL_TEXTURE0_ARB,0.0f, 1.0f); glVertex3f(d[3].x,d[3].y,d[3].z);
		glEnd();

		GLUquadricObj *qObj = gluNewQuadric();
		gluQuadricNormals(qObj, GLU_SMOOTH);
		gluQuadricTexture(qObj, GL_TRUE);
		glEnable(GL_TEXTURE_2D);

		matrix[12]=0;
		matrix[13]=0;
		matrix[14]=0;
		glMultMatrixf(matrix);
		gluSphere(qObj, scaleY, 20, 20);

		float modelview[16];
		float projection[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
		glGetFloatv(GL_PROJECTION_MATRIX, projection);

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glPopAttrib();

	glDisable(GL_VERTEX_PROGRAM_ARB);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	glUseProgramObjectARB(0);
}

void SmokeVideo::drawSmokeVideo( bool grapFrame, bool render,float fadeEffect, float normalizedToolTipX,float normalizedToolTipY,float depth,
								float toolX,float toolY,float toolZ,smDrawParam p_params)
{
	static int frameCounter=0;
	static float scale;
	

	if(fadeEffect<0.001)
		return;

	frameCounter++;
	spark->drawSpark(toolX,toolY,toolZ,p_params);
	glPushAttrib(GL_TEXTURE_BIT);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_VERTEX_PROGRAM_ARB);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glUseProgramObjectARB(programSmokeObject);
	glUniform1fARB(locFadeEffect, fadeEffect);	
	glUniform1f(locTipX,normalizedToolTipX);
	glUniform1f(locTipY,normalizedToolTipY);
	glUniform1iv(locSmokeType,SMOKE_MAXTEXTURES,this->smokeType);
	for(int i=0;i<SMOKE_TEXTURES;i++){
		glActiveTexture(GL_TEXTURE1+i);
		glEnable(GL_TEXTURE_2D);
		glUniform1iARB(locSmokeTex[i], i+1);
		glBindTexture(GL_TEXTURE_2D, texName[i]);
		if(grapFrame&&fadeEffect>0) {
			//frameGrapper->setCurrentStream(i,frame[i]);
			if(frames[i].data!=NULL)
				glTexSubImage2D (GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, frames[i].data);
		}
	}
	if(grapFrame&&fadeEffect>0){
		static int smokeFrameCounter=0;
		smokeFrameCounter++;
		if(smokeFrameCounter%2==0)
			frameGrapper->nextFrame();
	}

	glUniform1f(locDepth,depth);

	// Grab A Frame From The AVI

	if (bg&render)														// Is Background Visible?
	{
		glDisable(GL_DEPTH_TEST);	
		
		scale=1;

		glBegin(GL_QUADS);										// Begin Drawing The Background (One Quad)
		// Front Face	
		glMultiTexCoord2f(GL_TEXTURE0_ARB,0.0f, 0.0f); glVertex3f(-1.0f*scale, -1.0f*scale, -1.0f);
		glMultiTexCoord2f(GL_TEXTURE0_ARB,1.0f, 0.0f); glVertex3f( 1.0f*scale,  -1.0f*scale, -1.0f);
		glMultiTexCoord2f(GL_TEXTURE0_ARB,1.0f, 1.0f); glVertex3f( 1.0f*scale,   1.0f*scale, -1.0f);
		glMultiTexCoord2f(GL_TEXTURE0_ARB,0.0f, 1.0f); glVertex3f(-1.0f*scale,   1.0f*scale, -1.0f);
		glEnd();
	}

	glDisable(GL_VERTEX_PROGRAM_ARB);
	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	glUseProgramObjectARB(0);

	glEnable(GL_DEPTH_TEST);	
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glPopAttrib();

	for(int i=0;i<SMOKE_TEXTURES;i++){
		glActiveTexture(GL_TEXTURE1+i);
		glDisable(GL_TEXTURE_2D);
	}


}

void SmokeVideo::CloseAVI(void)												// Properly Closes The Avi File
{


}

void SmokeVideo::Deinitialize (void)										// Any User DeInitialization Goes Here
{
	frameGrapper->stop();
	frameGrapper->closeAndWait();

	//CloseAVI();													// Close The AVI File
}



void SmokeVideo::OpenAVI2(char* szFile, int textureNumber)										// Opens An AVI File (szFile)
{
	bool isfileOpened=caps[textureNumber].open(szFile);
	if(!isfileOpened){
		cout<<"ERROR: VIDEO cannot be opened"<<endl<<endl;
	}
	caps[textureNumber]>>frames[textureNumber];
	width=frames[textureNumber].cols;
	height=frames[textureNumber].rows;
	data[textureNumber]=frames[textureNumber].data;
}


void  SmokeVideo::Init2(){

}


void SmokeVideo::initFrameGraper(){
	frameGrapper=new FrameGrabber(this);
	frameGrapper->start();
}

void SmokeVideo::setSmokeTypes(){
	for(int i=0;i<SMOKE_MAXTEXTURES;i++){
		 this->smokeType[i]=smokeTypes[i];
	}
	quadSourceSmokeScale=10;
	quadEnvironmentSmokeScale=1.0;
}

bool SmokeVideo::Initialize ()					// Any GL Init Code & User Initialiazation Goes Here
{
	createShader();
	spark->createSparkShader();
	setSmokeTypes();

	//Init();//commented
	Init2();
	//glClearColor (0.0f, 0.0f, 0.0f, 0.5f);						// Black Background
	//glClearDepth (1.0f);										// Depth Buffer Setup
	//glDepthFunc (GL_LEQUAL);									// The Type Of Depth Testing (Less Or Equal)
	glEnable(GL_DEPTH_TEST);									// Enable Depth Testing
	glShadeModel (GL_SMOOTH);									// Select Smooth Shading
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			// Set Perspective Calculations To Most Accurate

	// Enable Texture Mapping
	glEnable(GL_TEXTURE_2D);	
	glGenTextures(SMOKE_TEXTURES, texName);

	for(int i=0;i<SMOKE_TEXTURES;i++){
		glBindTexture(GL_TEXTURE_2D, texName[i]);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);// Set Texture Max Filter
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);// Set Texture Min Filter

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);// Set Texture Min Filter
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);// Set Texture Min Filter

		OpenAVI2(smokeVideoFileNames[i],i);// Open The AVI File

		// Create The Texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data[i]);
	}
	initFrameGraper();

	return true;// Return TRUE (Initialization Successful)
}
