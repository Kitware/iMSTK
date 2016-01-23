// This file is part of the iMSTK project.
//
// Copyright (c) Kitware, Inc.
//
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

#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

/// \brief iMSTK version
#define IMSTKVERSION 1.0
/// \brief version date
#define IMSTKVERSION_TEXT "iMSTK Version 1.0-2009"

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
//        once we get iMSTK declspec'd and building dynamically
//        on Windows.
#if !defined(WIN32)
#  define IMSTK_IS_DYNAMIC
// #else
#  undef IMSTK_IS_DYNAMIC
#endif

#if !defined(WIN32) || !defined(IMSTK_IS_DYNAMIC)
#  if !defined(WIN32) && defined(IMSTK_IS_DYNAMIC) /* TODO: Handle mingw or gcc on windows */
#    define IMSTK_CONSTRUCTOR_ATTRIB static __attribute__((constructor))
#    define IMSTK_DESTRUCTOR_ATTRIB  static __attribute__((destructor))
#    define IMSTK_RUN_LOADER(CONTEXT) /* empty */
#  else
#    define IMSTK_CONSTRUCTOR_ATTRIB
#    define IMSTK_DESTRUCTOR_ATTRIB
#    define IMSTK_RUN_LOADER(CONTEXT) \
       extern void imstk_##CONTEXT##_onload(); \
       imstk_##CONTEXT##_onload()
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
  *   IMSTK_BEGIN_DYNAMIC_LOADER()
  *     IMSTK_BEGIN_ONLOAD(CONTEXT)
  *        ... code to run when loading ...
  *     IMSTK_FINISH_ONLOAD()
  *     IMSTK_BEGIN_ONUNLOAD(CONTEXT)
  *        ... code to run when unloading ...
  *     IMSTK_FINISH_ONUNLOAD()
  *   IMSTK_FINISH_DYNAMIC_LOADER()
  * </pre>
  *
  * The CONTEXT argument will be used in the name of a function on
  * some platforms, so stick to alphanumeric strings and underscores.
  */
#  define IMSTK_BEGIN_DYNAMIC_LOADER() /* empty */

#  define IMSTK_BEGIN_ONLOAD(CONTEXT) \
  IMSTK_CONSTRUCTOR_ATTRIB void imstk_##CONTEXT##_onload() {

#  define IMSTK_FINISH_ONLOAD() }

#  define IMSTK_BEGIN_ONUNLOAD(CONTEXT) \
  IMSTK_DESTRUCTOR_ATTRIB void imstk_##CONTEXT##_onunload() {

#  define IMSTK_FINISH_ONUNLOAD() }

#  define IMSTK_FINISH_DYNAMIC_LOADER() /* empty */

#else // _WIN32
#  include <windows.h>
#  define IMSTK_BEGIN_DYNAMIC_LOADER() \
  BOOL APIENTRY DllMain( \
       HMODULE hModule, DWORD reason_for_call, LPVOID lpReserved) \
     { \
       switch (reason_for_call) \
         {

#  define IMSTK_BEGIN_ONLOAD(CONTEXT) \
       case DLL_PROCESS_ATTACH:

#  define IMSTK_FINISH_ONLOAD() \
         break;

#  define IMSTK_BEGIN_ONUNLOAD(CONTEXT) \
       case DLL_PROCESS_ATTACH:

#  define IMSTK_FINISH_ONUNLOAD() \
         break;

#  define IMSTK_FINISH_DYNAMIC_LOADER() \
       default: \
         break; \
         } \
       return TRUE; \
     }
#endif // _WIN32

///iMSTK uses opengl now. So all API should have the definition.
#define IMSTK_OPENGL_SHADER 1



#endif // CORE_CONFIG_H
