#ifndef SMFACTORY_H
#define SMFACTORY_H

#include "Core/CoreClass.h"

#include <map>
#include <set>
#include <string>

/**\brief A macro to register a concrete subclass of an abstract base class with Factory.
  *
  * A macro to help register a concrete subclass with a factory.
  * It should be called inside dynamic loader macros like so:
  * <pre>
  *   SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  *     SIMMEDTK_BEGIN_ONLOAD(register_base_class_children)
  *       SIMMEDTK_REGISTER_CLASS(baseClassName,abstractClassName,concreteClassName1,integerClassGroupId);
  *       SIMMEDTK_REGISTER_CLASS(baseClassName,abstractClassName,concreteClassName2,integerClassGroupId);
  *       ...
  *     SIMMEDTK_FINISH_ONLOAD()
  *   SIMMEDTK_FINISH_DYNAMIC_LOADER()
  * </pre>
  * If you build SimMedTK with static libraries, then your
  * application should call (for the example above)
  * <pre>simmedtk_register_base_class_children_onload();</pre>
  * during your application's startup.
  *
  * This macro should only be used outside of the Core library;
  * inside Core, just modify Factory so that s_catalog is
  * initialized with the proper entries directly. That's because
  * inside Core, Factory::s_catalog might not be initialized
  * before the first registration call is made.
  *
  * A more specific example is the way the Rendering library
  * registers the Viewer class as a concrete child of ViewerBase:
  * <pre>
  *   SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  *     SIMMEDTK_BEGIN_ONLOAD(register_viewer_children)
  *       SIMMEDTK_REGISTER_CLASS(CoreClass,ViewerBase,Viewer,0);
  *     SIMMEDTK_FINISH_ONLOAD()
  *   SIMMEDTK_FINISH_DYNAMIC_LOADER()
  * </pre>
  * When SimMedTK is built as dynamic libraries, this registration is
  * automatic whenever linking to Rendering. Versions with a static
  * Rendering library require you to call
  * simmedtk_register_viewer_children_onload()
  * at startup.
  */
#define SIMMEDTK_REGISTER_CLASS(BASECLASS,TARGETCLASS,SUBCLASS,GROUP) \
       Factory<BASECLASS>::registerClassConfiguration( \
         #TARGETCLASS, \
         #SUBCLASS, \
         []() { return std::shared_ptr<BASECLASS>(new SUBCLASS); }, \
         GROUP);

#define RegisterFactoryClass(BASECLASS,SUBCLASS,GROUP) \
    SIMMEDTK_BEGIN_DYNAMIC_LOADER() \
        SIMMEDTK_BEGIN_ONLOAD(register_IOMeshVTKDelegate) \
        SIMMEDTK_REGISTER_CLASS(BASECLASS, BASECLASS, SUBCLASS, GROUP) \
        SIMMEDTK_FINISH_ONLOAD() \
    SIMMEDTK_FINISH_DYNAMIC_LOADER()

/**\brief A factory provides a way to discover and construct subclasses of abstract classes.
  *
  * Concrete subclasses of abstract bases should call
  * Factory::registerClassConfiguration() in an initializer
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
class Factory
{
public:
  /// A function object whose signature returns a shared pointer to an CoreClass instance.
  typedef std::function<std::shared_ptr<T>()> SharedPointerConstructor;

  /// An internal structure used by Factory to track subclasses of abstract classes.
  struct FactoryEntry
  {
    std::string subclassname;
    SharedPointerConstructor constructor;
    int group;

    bool operator < (const FactoryEntry& other) const
      {
      return
        (this->group < other.group ||
         (this->group == other.group &&
          this->subclassname < other.subclassname)) ? true : false;
      }
  };

  /// A set of subclasses associated with one abstract base class.
  typedef std::set<FactoryEntry> FactoryConfigurationOptions;

  static void registerClassConfiguration(
    const std::string& classname,
    const std::string& subclassname,
    SharedPointerConstructor ctor,
    int group = 1000);

  static const FactoryConfigurationOptions& optionsForClass(const std::string& targetClassname);

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

  static std::shared_ptr<T> createConcreteClassForGroup(
    const std::string& targetClassname, int group);

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
  static std::map<std::string, FactoryConfigurationOptions>* s_catalog;
};

#include "Core/Factory.hpp"

#endif // SMFACTORY_H
