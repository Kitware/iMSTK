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

/// \brief SimMedTK version
#define SIMMEDTKVERSION 1.0
/// \brief version date
#define SIMMEDTKVERSION_TEXT "SimMedTK Version 1.0-2009"

#undef _UNICODE

#ifdef _WIN32
#ifndef MINGW32
#  define NOMINMAX
#endif
#  include <windows.h>
// #   define inline  __forceinline
// #   pragma inline_recursion(on)
// #   pragma inline_depth(255)
#endif

#include <GL/glew.h>
#ifndef __APPLE__
#  include <GL/gl.h> // for GLfloat, etc.
#else
#  include <OpenGL/gl.h> // for GLfloat, etc.
#endif

namespace core {

enum
{
    MaxFilenameLength = 500,
    MaxShaderVariableName = 255
};

/// \brief class types. Each class should have type of itself
enum class ClassType
{
    Reserved,
    Math,
    Matrix33,
    Matrix44,
    Shader,
    Errorlog,
    Displaytext,
    Sceneboject,
    Scene,
    Sdk,
    Dispathcer,
    Viewer,
    Simulator,
    CollisionDetection,
    Mesh,
    StaticSceneObject,
    ObjectSimulator,
    PbdSimulator,
    FemSceneobject,
    Vbo,
    MemoryBlock,
    PhsyxObject,
    Interface,
    DoubleBuffer,
    Synchronization,
    PbdSceneObject,
    PbdSurfaceSceneObject,
    PbdVolumeSceneObject,
    StylusSceneObject,
    StylusRigidSceneObject,
    StylusSeformableSceneObject,
    PtSceneObject,
    LoopSceneObject,
    LoopBaseSceneObject,
    ThreadSceneObject,
    VesselSceneObject,
    GauzeSceneObject,
    SpongeSceneObject,
    RigidNotescholySceneObject,
    SoldierSceneObject,
    VegaFemSceneObject,
    Unknown
};


/// \brief these are the dispather return events
enum class ResultDispatcher
{
    Success,
    Failure
};

/// \brief caller should send when it begin/end frames
enum class CallerState
{
    BeginFrame,
    EndFrame,
    BeginObjectFrame,
    EndObjectFrame
};

/// \brief Events of the dispatcher.not used and implemented yet.
enum class DispatcherEvent
{
    UpdateScene
};
/// \brief drawing order of the objects
enum class ClassDrawOrder
{
    BeforeObjects,
    AfterObjects
};

} // namespace core
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

#else // _WIN32
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
#endif // _WIN32

///SimMedTK uses opengl now. So all API should have the definition.
#define SIMMEDTK_OPENGL_SHADER 1



#endif // SMCONFIG_H
