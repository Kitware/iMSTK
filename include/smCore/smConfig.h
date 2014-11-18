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
    \brief	    This Module declares framework overall configuration params. It should be included by all
                Header files in the framwork. Any definition that is not for all core framework 
                should not be stated here.

*****************************************************
*/

#ifndef SMCONFIG_H
#define SMCONFIG_H 
#undef _UNICODE
#include <iostream>
#include <cstdint>
using namespace std;

#define SOFMIS_RENDERER_OPENGL	1.0
//Windows implementation is here
#ifdef _WIN32 
	#define SOFMIS_OPERATINGSYSTEM_WINDOWS
#endif
#ifdef __linux__
	#define SOFMIS_OPERATINGSYSTEM_LINUX
#endif

#define SOFMIS_THREAD_OPENMP
//#define SOFMIS_THREAD_QTTHREAD

#ifdef SOFMIS_THREAD_OPENMP

#endif

#define SOFMISVERSION    1.0
#define SOFMISVERSION_TEXT  "SoFMIS Version 1.0-2009"

#undef QT_NO_CAST_FROM_ASCII
#undef QT_NO_CAST_TO_ASCII

#include <QString>
//#include <string>
#include "float.h"
///General type definitions
#define smInt			int        
#define smFloat			float   
#define smDouble		double 
#define smShort			short
#define smChar			char		//note that smChar is used for pointer iteration over the memory block.Don't change this unless you know what you're doing
#define smBool			bool
#define smLongInt		long int
#define smLongDouble	long double
//#define smString		string
//#define smString    QString

class smString:public QString{
	public:
	//smChar *c_str();
	smString & operator=(smChar*p_param);
};


#define smUInt32     uint32_t
#define smUInt64     uint64_t
#define smUInt       unsigned int        
#define smUFloat     unsigned float   
#define smUDouble    unsigned double 
#define smUShort     unsigned short
#define smUChar      unsigned char		//note that smChar is used for pointer iteration over the memory block.Don't change this unless you know what you're doing  
#define smReal			smFloat  

#define smLongInt	 long int
#define smLongFloat	 long float
#define smLongDouble long double



///Type declarations for OPENGL
#define smGLReal		GLfloat
#define smGLFloat		GLfloat
#define smGLDouble		GLdouble
#define smGLInt			GLint
#define smGLUInt		GLuint


///Type conversion to OPENGL
#define smGLFloatType   GL_FLOAT
#define smGLRealType    GL_FLOAT
#define smGLIntType		GL_INT
#define smGLUIntType	GL_UNSIGNED_INT
#define smGLUShortType  GL_UNSIGNED_SHORT

#define smMAXFLOAT		FLT_MAX


enum SOFMIS_TYPEINFO
{
	SOFMIS_TYPE_INT,
	SOFMIS_TYPE_FLOAT,
	SOFMIS_TYPE_DOUBLE,
	SOFMIS_TYPE_SHORT,
	SOFMIS_TYPE_BOOL,
	SOFMIS_TYPE_LONGINT,
	SOFMIS_TYPE_LONGDOUBLE,
	SOFMIS_TYPE_STRING,
	SOFMIS_TYPE_UFLOAT,
	SOFMIS_TYPE_UDOUBLE,
	SOFMIS_TYPE_USHORT,
	SOFMIS_TYPE_UCHAR,
	SOFMIS_TYPE_REAL,
	SOFMIS_TYPE_GLFLOAT,
	SOFMIS_TYPE_GLDOUBLE,
	SOFMIS_TYPE_GLINT,
	SOFMIS_TYPE_GLUINT
};






#define SOFMIS_MAX_FILENAME_LENGTH   500
#define SOFMIS_MAX_ERRORLOG_TEXT     500
#define SOFMIS_MAX_ERRORLOG          100
#define SOFMIS_MAX_DISPLAYTEXT       500
#define SOFMIS_MAX_EVENTSBUFFER		 300
#define SOFMIS_MAX_MODULES			  40
#define SOFMIS_MAX_SHADERVARIABLENAME  255

//Each class should have type of itself
enum smClassType{
    SOFMIS_RESERVED,           
    SOFMIS_SMVEC3,           
    SOFMIS_SMMATH,           
    SOFMIS_SMATRIX33,        
    SOFMIS_SMATRIX44,        
    SOFMIS_SMSHADER,
    SOFMIS_SMERRORLOG,
    SOFMIS_SMDISPLAYTEXT,
    SOFMIS_SMSCENEBOJECT,
    SOFMIS_SMSCENE,
    SOFMIS_SMSDK,
    SOFMIS_SMDISPATHCER,
    SOFMIS_SMVIEWER,
    SOFMIS_SMSIMULATOR,
    SOFMIS_SMCOLLISIONDETECTION,
    SOFMIS_SMMESH,
    SOFMIS_SMSTATICSCENEOBJECT,
	SOFMIS_SMOBJECTSIMULATOR,
	SOFMIS_SMPBDSIMULATOR,
	SOFMIS_SMFEMSCENEOBJECT,
	SOFMIS_SMVBO,
	SOFMIS_SMMEMORYBLOCK,
	SOFMIS_SMPHSYXOBJECT,
	SOFMIS_SMINTERFACE,
	SOFMIS_SMDOUBLEBUFFER,
	SOFMIS_SMSYNCHRONIZATION,
	SOFMIS_SMPBDSCENEOBJECT,
	SOFMIS_SMPBDSURFACESCENEOBJECT,
	SOFMIS_SMPBDVOLUMESCENEOBJECT,
	SOFMIS_SMSTYLUSSCENEOBJECT,
	SOFMIS_SMSTYLUSRIGIDSCENEOBJECT,
	SOFMIS_SMSTYLUSDEFORMABLESCENEOBJECT,
	SOFMIS_SMPTSCENEOBJECT,
	SOFMIS_SMLOOPSCENEOBJECT,
	SOFMIS_SMLOOPBASESCENEOBJECT,

	SOFMIS_SMTHREADSCENEOBJECT,
	SOFMIS_SMVESSELSCENEOBJECT,
	
	SOFMIS_SMGAUZESCENEOBJECT,
	SOFMIS_SMSPONGESCENEOBJECT,

	SOFMIS_SMRIGIDNOTESCHOLYSCENEOBJECT,

	SOFMIS_SMSOLDIERSCENEOBJECT
 };

///these are the dispather return events
enum smDispathcerResult{
    SOFMIS_DISPATCHER_OK,
    SOFMIS_DISPATCHER_ERROR,
};

///caller should send when it calls 
enum smCallerState{
    SOFMIS_CALLERSTATE_BEGINFRAME,
    SOFMIS_CALLERSTATE_ENDFRAME,
    SOFMIS_CALLERSTATE_BEGINOBJECTFRAME,
    SOFMIS_CALLERSTATE_ENDOBJECTFRAME,

};
///Events of the dispatcher.not used and implemented yet.
enum smDispatcherEvent{
    SOFMIS_EVENT_UPDATESCENE,
};


enum smClassDrawOrder{
  SOFMIS_DRAW_BEFOREOBJECTS,
  SOFMIS_DRAW_AFTEROBJECTS
};


///SOFMIS uses opengl now. So all API should have the definition.
#define SOFMIS_OPENGL_SHADER  1


#undef _UNICODE
#define NOMINMAX


#ifdef SOFMIS_OPERATINGSYSTEM_WINDOWS
	#define inline  __forceinline
	#pragma inline_recursion(on)
	#pragma inline_depth( [255] )
#endif

#define SOFMIS_MIN(X,Y) (X<Y?X:Y)
#define SOFMIS_MAX(X,Y) (X>Y?X:Y)



 ostream &operator<<(ostream &p_os,smString &p_param);


 class smSDK;
 class smViewer;
 class smScene;
 class smPipe;
 template<typename T> class smIndiceArray;
 typedef smIndiceArray<smPipe>  smIndiceArrayPipe;

#endif


