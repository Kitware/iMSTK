#ifndef SMFACTORY_H
#define SMFACTORY_H

#include "smCore/smCoreClass.h"

#include <map>
#include <set>
#include <string>

/**\brief A macro to register a concrete subclass of an abstract base class with smFactory.
  *
  * You should call this in a compilation unit's global scope (i.e., in an
  * implementation file - not a header- and outside any functions).
  * It defines a special function that will be called when a dynamic library
  * containing the compilation unit is loaded by the link loader.
  * This function registers the class with smFactory.
  *
  * This macro should only be used outside of the smCore library;
  * inside smCore, just modify smFactory so that s_catalog is
  * initialized with the proper entries directly. That's because
  * inside smCore, smFactory::s_catalog might not be initialized
  * before the first registration call is made.
  */
#ifndef WIN32
#  define SIMMEDTK_REGISTER_CLASS(BASECLASS,TARGETCLASS,SUBCLASS,GROUP) \
     static void TARGETCLASS##_##SUBCLASS##_registrar() __attribute__((constructor)); \
     static void TARGETCLASS##_##SUBCLASS##_registrar() \
     { \
       smFactory<BASECLASS>::registerClassConfiguration( \
         #TARGETCLASS, \
         #SUBCLASS, \
         []() { return std::shared_ptr<BASECLASS>(new SUBCLASS); }, \
         GROUP); \
     }
#else
#  include <windows.h>
#  define SIMMEDTK_REGISTER_CLASS(BASECLASS,TARGETCLASS,SUBCLASS,GROUP) \
     BOOL APIENTRY DllMain( \
       HMODULE hModule, DWORD reason_for_call, LPVOID lpReserved) \
     { \
       switch (reason_for_call) \
         { \
       case DLL_PROCESS_ATTACH: \
         smFactory<BASECLASS>::registerClassConfiguration( \
           #TARGETCLASS, \
           #SUBCLASS, \
           []() { return std::shared_ptr<BASECLASS>(new SUBCLASS); }, \
           GROUP); \
         break; \
       case DLL_THREAD_ATTACH: \
       case DLL_THREAD_DETACH: \
       case DLL_PROCESS_DETACH: /* TODO: Could unregister here */ \
       default: \
         break; \
         } \
       return TRUE; \
     }
#endif // WIN32

/**\brief A factory provides a way to discover and construct subclasses of abstract classes.
  *
  * Concrete subclasses of abstract bases should call
  * smFactory::registerClassConfiguration() in an initializer
  * function.
  * Once this is done, the abstract class name can be
  * used as a key to fetch a list of possible subclasses
  * to construct.
  * The list is sorted by the user-specified group number
  * (1000 by default) and then by subclass name; the group
  * number serves as a primitive priority that can be used
  * to prefer one subclass over another.
  * The group number can also be used to tie several
  * subclasses together. For instance, an abstract rendering
  * class may require an abstract window class of a matching
  * type (an OpenGL renderer requires a GLUT window, while a
  * text renderer requires a TTY window). In this case, the
  * OpenGL classes (both the renderer and window) would register
  * with the same group number (say, 10) while the text
  * classes would register with a different group number (say 50).
  * Asking for an object by group will then return subclasses
  * that make sense together. In the example above, the particular
  * numbers would ensure that OpenGL rendering would be preferred
  * above text rendering.
  */
template<typename T>
class smFactory
{
public:
  /// A function object whose signature returns a shared pointer to an smCoreClass instance.
  typedef std::function<std::shared_ptr<T>()> SharedPointerConstructor;

  /// An internal structure used by smFactory to track subclasses of abstract classes.
  struct smFactoryEntry
  {
    std::string subclassname;
    SharedPointerConstructor constructor;
    int group;

    bool operator < (const smFactoryEntry& other) const
      {
      return
        (this->group < other.group ||
         (this->group == other.group &&
          this->subclassname < other.subclassname)) ? true : false;
      }
  };

  /// A set of subclasses associated with one abstract base class.
  typedef std::set<smFactoryEntry> smFactoryConfigurationOptions;

  static void registerClassConfiguration(
    const std::string& classname,
    const std::string& subclassname,
    SharedPointerConstructor ctor,
    int group = 1000);

  static const smFactoryConfigurationOptions& optionsForClass(const std::string& targetClassname);

  static std::shared_ptr<T> createDefault(
    const std::string& targetClassname);

  static std::shared_ptr<T> createSubclass(
    const std::string& targetClassname,
    const std::string& subclassname);

  static std::shared_ptr<T> createSubclassForGroup(
    const std::string& targetClassname,
    int group);

  static std::shared_ptr<T> createConcreteClass(
    const std::string& targetClassname);

  template<typename U>
  static std::shared_ptr<U> createDefaultAs(const std::string& targetClassname)
    { return std::dynamic_pointer_cast<U>(createDefault(targetClassname)); }

  template<typename U>
  static std::shared_ptr<U> createSubclassAs(const std::string& targetClassname, const std::string& subclassname)
    { return std::dynamic_pointer_cast<U>(createSubclass(targetClassname, subclassname)); }

  template<typename U>
  static std::shared_ptr<U> createSubclassForGroupAs(const std::string& targetClassname, int group)
    { return std::dynamic_pointer_cast<U>(createSubclassForGroup(targetClassname, group)); }

  template<typename U>
  static std::shared_ptr<U> createConcreteClassAs(const std::string& targetClassname)
    { return std::dynamic_pointer_cast<U>(createConcreteClass(targetClassname)); }

protected:
  static std::map<std::string, smFactoryConfigurationOptions> s_catalog;
};

#include "smCore/smFactory.hpp"

#endif // SMFACTORY_H
