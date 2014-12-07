/*
****************************************************
				SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMCONFIG_H
#define SMCONFIG_H 
#undef _UNICODE
#include <iostream>
#include <cstdint>
using namespace std;
/// \brief opengl rendering version
#define SOFMIS_RENDERER_OPENGL	1.0
/// \brief Windows definition is here
#ifdef _WIN32 
	#define SOFMIS_OPERATINGSYSTEM_WINDOWS
#endif
#ifdef __linux__
	#define SOFMIS_OPERATINGSYSTEM_LINUX
#endif
/// \brief defines the threading mechanism
#define SOFMIS_THREAD_OPENMP
//#define SOFMIS_THREAD_QTTHREAD

#ifdef SOFMIS_THREAD_OPENMP

#endif
/// \brief sofmis version 
#define SOFMISVERSION 1.0
/// \brief version date
#define SOFMISVERSION_TEXT "SoFMIS Version 1.0-2009"
/// \brief QT specific declarations
#undef QT_NO_CAST_FROM_ASCII
#undef QT_NO_CAST_TO_ASCII

#include <QString>
#include "float.h"

/// \brief General type definitions
#define smInt			int
#define smFloat			float
#define smDouble		double
#define smShort			short
#define smChar			char	//note that smChar is used for pointer iteration over the memory block.Don't change this unless you know what you're doing
#define smBool			bool
#define smLongInt		long int
#define smLongDouble	long double

/// \brief string. currently uses QString
class smString:public QString{
public:
	//smChar *c_str();
	smString & operator=(smChar*p_param);
};

/// \brief  unified variable declarations
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

/// \brief type info
enum SOFMIS_TYPEINFO{
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
/// \brief maximum file name length 
#define SOFMIS_MAX_FILENAME_LENGTH   500
/// \brief maximum error text
#define SOFMIS_MAX_ERRORLOG_TEXT     500
/// \brief maximum number of errors
#define SOFMIS_MAX_ERRORLOG          100
/// \brief total number characters diplayed
#define SOFMIS_MAX_DISPLAYTEXT       500
/// \brief number of events that are supported
#define SOFMIS_MAX_EVENTSBUFFER		 300
/// \brief number of modules
#define SOFMIS_MAX_MODULES			  40
/// \brief shader variable length
#define SOFMIS_MAX_SHADERVARIABLENAME  255

/// \brief class types. Each class should have type of itself
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

/// \brief these are the dispather return events
enum smDispathcerResult{
	SOFMIS_DISPATCHER_OK,
	SOFMIS_DISPATCHER_ERROR,
};

/// \brief caller should send when it begin/end frames 
enum smCallerState{
	SOFMIS_CALLERSTATE_BEGINFRAME,
	SOFMIS_CALLERSTATE_ENDFRAME,
	SOFMIS_CALLERSTATE_BEGINOBJECTFRAME,
	SOFMIS_CALLERSTATE_ENDOBJECTFRAME,

};

/// \brief Events of the dispatcher.not used and implemented yet.
enum smDispatcherEvent{
	SOFMIS_EVENT_UPDATESCENE,
};
/// \brief drawing order of the objects
enum smClassDrawOrder{
	SOFMIS_DRAW_BEFOREOBJECTS,
	SOFMIS_DRAW_AFTEROBJECTS
};

///SOFMIS uses opengl now. So all API should have the definition.
#define SOFMIS_OPENGL_SHADER 1
/// \brief unicode is not used.
#undef _UNICODE
#define NOMINMAX
/// \brief OS specific routines
#ifdef SOFMIS_OPERATINGSYSTEM_WINDOWS
	#define inline  __forceinline
	#pragma inline_recursion(on)
	#pragma inline_depth( [255] )
#endif
/// \brief  fast min, max
#define SOFMIS_MIN(X,Y) (X<Y?X:Y)
/// \brief  fast min, max
#define SOFMIS_MAX(X,Y) (X>Y?X:Y)
/// \brief  operator overloading for smString
ostream &operator<<(ostream &p_os,smString &p_param);

class smSDK;
class smViewer;
class smScene;
class smPipe;
/// \brief  indices array for fast access with index 
template<typename T> class smIndiceArray;
/// \brief  used  in  pipe
typedef smIndiceArray<smPipe>  smIndiceArrayPipe;

#endif


