// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

// The following comment must be present for doxygen to generate
// documentation for global macros and functions:
/*!\file Core configuration macros and typedefs.
 */

#ifndef SMCONFIG_H
#define SMCONFIG_H
#undef _UNICODE

/// \brief Windows definition is here
#ifdef _WIN32
#define SIMMEDTK_OPERATINGSYSTEM_WINDOWS
#endif
#ifdef __linux__
#define SIMMEDTK_OPERATINGSYSTEM_LINUX
#endif

#ifdef SIMMEDTK_OPERATINGSYSTEM_WINDOWS
#ifndef MINGW32
    #define NOMINMAX
#endif
#include <windows.h>
#endif

#include <iostream>
#include <cstdint>
#include <GL/glew.h>
#ifndef __APPLE__
#  include <GL/gl.h> // for GLfloat, etc.
#else
#  include <OpenGL/gl.h> // for GLfloat, etc.
#endif

/// \brief opengl rendering version
#define SIMMEDTK_RENDERER_OPENGL    1.0
/// \brief defines the threading mechanism
#define SIMMEDTK_THREAD_OPENMP
//#define SIMMEDTK_THREAD_QTTHREAD

#ifdef SIMMEDTK_THREAD_OPENMP

#endif
/// \brief SimMedTK version
#define SIMMEDTKVERSION 1.0
/// \brief version date
#define SIMMEDTKVERSION_TEXT "SimMedTK Version 1.0-2009"

#include "float.h"

/// \brief General type definitions
#define smInt           int
#define smFloat         float
#define smDouble        double
#define smShort         short
#define smChar          char    //note that smChar is used for pointer iteration over the memory block.Don't change this unless you know what you're doing
#define smBool          bool
#define smLongInt       long int
#define smLongDouble    long double
#define smString        std::string

/// \brief  unified variable declarations
#define smUInt32     uint32_t
#define smUInt64     uint64_t
#define smUInt       unsigned int
#define smUFloat     unsigned float
#define smUDouble    unsigned double
#define smUShort     unsigned short
#define smUChar      unsigned char      //note that smChar is used for pointer iteration over the memory block.Don't change this unless you know what you're doing
#define smReal          smFloat

#define smLongInt    long int
#define smLongFloat  long float
#define smLongDouble long double

///Type declarations for OPENGL
#define smGLReal        GLfloat
#define smGLFloat       GLfloat
#define smGLDouble      GLdouble
#define smGLInt         GLint
#define smGLUInt        GLuint

///Type conversion to OPENGL
#define smGLFloatType   GL_FLOAT
#define smGLRealType    GL_FLOAT
#define smGLIntType     GL_INT
#define smGLUIntType    GL_UNSIGNED_INT
#define smGLUShortType  GL_UNSIGNED_SHORT
#define smMAXFLOAT      FLT_MAX

/// \brief type info
enum SIMMEDTK_TYPEINFO
{
    SIMMEDTK_TYPE_INT,
    SIMMEDTK_TYPE_FLOAT,
    SIMMEDTK_TYPE_DOUBLE,
    SIMMEDTK_TYPE_SHORT,
    SIMMEDTK_TYPE_BOOL,
    SIMMEDTK_TYPE_LONGINT,
    SIMMEDTK_TYPE_LONGDOUBLE,
    SIMMEDTK_TYPE_STRING,
    SIMMEDTK_TYPE_UFLOAT,
    SIMMEDTK_TYPE_UDOUBLE,
    SIMMEDTK_TYPE_USHORT,
    SIMMEDTK_TYPE_UCHAR,
    SIMMEDTK_TYPE_REAL,
    SIMMEDTK_TYPE_GLFLOAT,
    SIMMEDTK_TYPE_GLDOUBLE,
    SIMMEDTK_TYPE_GLINT,
    SIMMEDTK_TYPE_GLUINT
};
/// \brief maximum file name length
#define SIMMEDTK_MAX_FILENAME_LENGTH   500
/// \brief maximum error text
#define SIMMEDTK_MAX_ERRORLOG_TEXT     500
/// \brief maximum number of errors
#define SIMMEDTK_MAX_ERRORLOG          100
/// \brief total number characters diplayed
#define SIMMEDTK_MAX_DISPLAYTEXT       500
/// \brief number of events that are supported
#define SIMMEDTK_MAX_EVENTSBUFFER        300
/// \brief number of modules
#define SIMMEDTK_MAX_MODULES              40
/// \brief shader variable length
#define SIMMEDTK_MAX_SHADERVARIABLENAME  255

/// \brief class types. Each class should have type of itself
enum smClassType
{
    SIMMEDTK_RESERVED,
    SIMMEDTK_SMVEC3,
    SIMMEDTK_SMMATH,
    SIMMEDTK_SMATRIX33,
    SIMMEDTK_SMATRIX44,
    SIMMEDTK_SMSHADER,
    SIMMEDTK_SMERRORLOG,
    SIMMEDTK_SMDISPLAYTEXT,
    SIMMEDTK_SMSCENEBOJECT,
    SIMMEDTK_SMSCENE,
    SIMMEDTK_SMSDK,
    SIMMEDTK_SMDISPATHCER,
    SIMMEDTK_SMVIEWER,
    SIMMEDTK_SMSIMULATOR,
    SIMMEDTK_SMCOLLISIONDETECTION,
    SIMMEDTK_SMMESH,
    SIMMEDTK_SMSTATICSCENEOBJECT,
    SIMMEDTK_SMOBJECTSIMULATOR,
    SIMMEDTK_SMPBDSIMULATOR,
    SIMMEDTK_SMFEMSCENEOBJECT,
    SIMMEDTK_SMVBO,
    SIMMEDTK_SMMEMORYBLOCK,
    SIMMEDTK_SMPHSYXOBJECT,
    SIMMEDTK_SMINTERFACE,
    SIMMEDTK_SMDOUBLEBUFFER,
    SIMMEDTK_SMSYNCHRONIZATION,
    SIMMEDTK_SMPBDSCENEOBJECT,
    SIMMEDTK_SMPBDSURFACESCENEOBJECT,
    SIMMEDTK_SMPBDVOLUMESCENEOBJECT,
    SIMMEDTK_SMSTYLUSSCENEOBJECT,
    SIMMEDTK_SMSTYLUSRIGIDSCENEOBJECT,
    SIMMEDTK_SMSTYLUSDEFORMABLESCENEOBJECT,
    SIMMEDTK_SMPTSCENEOBJECT,
    SIMMEDTK_SMLOOPSCENEOBJECT,
    SIMMEDTK_SMLOOPBASESCENEOBJECT,
    SIMMEDTK_SMTHREADSCENEOBJECT,
    SIMMEDTK_SMVESSELSCENEOBJECT,
    SIMMEDTK_SMGAUZESCENEOBJECT,
    SIMMEDTK_SMSPONGESCENEOBJECT,
    SIMMEDTK_SMRIGIDNOTESCHOLYSCENEOBJECT,
    SIMMEDTK_SMSOLDIERSCENEOBJECT,
    SIMMEDTK_SMVEGAFEMSCENEOBJECT,
    SIMMEDTK_SMSCENEOBJECT_TYPE_UNKNOWN
};

/// \brief these are the dispather return events
enum smDispathcerResult
{
    SIMMEDTK_DISPATCHER_OK,
    SIMMEDTK_DISPATCHER_ERROR,
};

/// \brief caller should send when it begin/end frames
enum smCallerState
{
    SIMMEDTK_CALLERSTATE_BEGINFRAME,
    SIMMEDTK_CALLERSTATE_ENDFRAME,
    SIMMEDTK_CALLERSTATE_BEGINOBJECTFRAME,
    SIMMEDTK_CALLERSTATE_ENDOBJECTFRAME,

};

/// \brief Events of the dispatcher.not used and implemented yet.
enum smDispatcherEvent
{
    SIMMEDTK_EVENT_UPDATESCENE,
};
/// \brief drawing order of the objects
enum smClassDrawOrder
{
    SIMMEDTK_DRAW_BEFOREOBJECTS,
    SIMMEDTK_DRAW_AFTEROBJECTS
};

// FIXME: This is currently hardwired into CMake, but must change
//        once we get SimMedTK declspec'd and building dynamically
//        on Windows.
#if !defined(WIN32)
#  define SIMMEDTK_IS_DYNAMIC
#else
#  undef SIMMEDTK_IS_DYNAMIC
#endif

#if !defined(WIN32) || !defined(SIMMEDTK_IS_DYNAMIC)
#  if !defined(WIN32) && defined(SIMMEDTK_IS_DYNAMIC) /* TODO: Handle mingw or gcc on windows */
#    define SIMMEDTK_CONSTRUCTOR_ATTRIB static __attribute__((constructor))
#    define SIMMEDTK_DESTRUCTOR_ATTRIB  static __attribute__((destructor))
#    define SIMMEDTK_RUN_LOADER(CONTEXT) /* empty */
#  else
#    define SIMMEDTK_CONSTRUCTOR_ATTRIB
#    define SIMMEDTK_DESTRUCTOR_ATTRIB
#    define SIMMEDTK_RUN_LOADER(CONTEXT) \
       extern void simmedtk_##CONTEXT##_onload(); \
       simmedtk_##CONTEXT##_onload()
#  endif

/**\brief Invoke methods at the time a dynamic library is invoked.
  *
  * You should call these macros in a compilation unit's global scope
  * (i.e., in an implementation file - not a header- and outside any
  * functions).
  * It defines a special function that will be called when a dynamic
  * library containing the compilation unit is loaded by the link loader.
  *
  * The dynamic loader macros can be called at most once per compilation unit.
  *
  * <pre>
  *   SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  *     SIMMEDTK_BEGIN_ONLOAD(CONTEXT)
  *        ... code to run when loading ...
  *     SIMMEDTK_FINISH_ONLOAD()
  *     SIMMEDTK_BEGIN_ONUNLOAD(CONTEXT)
  *        ... code to run when unloading ...
  *     SIMMEDTK_FINISH_ONUNLOAD()
  *   SIMMEDTK_FINISH_DYNAMIC_LOADER()
  * </pre>
  *
  * The CONTEXT argument will be used in the name of a function on
  * some platforms, so stick to alphanumeric strings and underscores.
  */
#  define SIMMEDTK_BEGIN_DYNAMIC_LOADER() /* empty */

#  define SIMMEDTK_BEGIN_ONLOAD(CONTEXT) \
  SIMMEDTK_CONSTRUCTOR_ATTRIB void simmedtk_##CONTEXT##_onload() {

#  define SIMMEDTK_FINISH_ONLOAD() }

#  define SIMMEDTK_BEGIN_ONUNLOAD(CONTEXT) \
  SIMMEDTK_DESTRUCTOR_ATTRIB void simmedtk_##CONTEXT##_onunload() {

#  define SIMMEDTK_FINISH_ONUNLOAD() }

#  define SIMMEDTK_FINISH_DYNAMIC_LOADER() /* empty */

#else // SIMMEDTK_OPERATINGSYSTEM_WINDOWS
#  include <windows.h>
#  define SIMMEDTK_BEGIN_DYNAMIC_LOADER() \
  BOOL APIENTRY DllMain( \
       HMODULE hModule, DWORD reason_for_call, LPVOID lpReserved) \
     { \
       switch (reason_for_call) \
         {

#  define SIMMEDTK_BEGIN_ONLOAD(CONTEXT) \
       case DLL_PROCESS_ATTACH:

#  define SIMMEDTK_FINISH_ONLOAD() \
         break;

#  define SIMMEDTK_BEGIN_ONUNLOAD(CONTEXT) \
       case DLL_PROCESS_ATTACH:

#  define SIMMEDTK_FINISH_ONUNLOAD() \
         break;

#  define SIMMEDTK_FINISH_DYNAMIC_LOADER() \
       default: \
         break; \
         } \
       return TRUE; \
     }
#endif // SIMMEDTK_OPERATINGSYSTEM_WINDOWS

///SimMedTK uses opengl now. So all API should have the definition.
#define SIMMEDTK_OPENGL_SHADER 1
/// \brief unicode is not used.
#undef _UNICODE
#define NOMINMAX
/// \brief OS specific routines
#ifdef SIMMEDTK_OPERATINGSYSTEM_WINDOWS
#define inline  __forceinline
#pragma inline_recursion(on)
#pragma inline_depth(255)
#endif
/// \brief  fast min, max
#define SIMMEDTK_MIN(X,Y) (X<Y?X:Y)
/// \brief  fast min, max
#define SIMMEDTK_MAX(X,Y) (X>Y?X:Y)

class smSDK;
class smViewer;
class smScene;
class smPipe;
/// \brief  indices array for fast access with index
template<typename T> class smIndiceArray;
/// \brief  used  in  pipe
typedef smIndiceArray<smPipe>  smIndiceArrayPipe;

#endif // SMCONFIG_H
