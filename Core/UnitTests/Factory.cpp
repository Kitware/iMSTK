#include <bandit/bandit.h>

#include "Core/Factory.h"

#include <iostream>

class abstract : public CoreClass
{
public:
  virtual std::string stupid() const = 0;
};

class A : public abstract
{
public:
  virtual std::string stupid() const { return "A"; }
};

class B : public abstract
{
public:
  virtual std::string stupid() const { return "B"; }
};

SIMMEDTK_BEGIN_DYNAMIC_LOADER()
  SIMMEDTK_BEGIN_ONLOAD(register_abstract_children)
    SIMMEDTK_REGISTER_CLASS(CoreClass, abstract, A, 65);
    SIMMEDTK_REGISTER_CLASS(CoreClass, abstract, B, 66);
  SIMMEDTK_FINISH_ONLOAD()
SIMMEDTK_FINISH_DYNAMIC_LOADER()

using namespace bandit;

go_bandit([](){
  describe("factory", []() {
    SIMMEDTK_RUN_LOADER(register_abstract_children);

    it("shows 2 subclasses of \"abstract\"", [&]() {
      AssertThat(Factory<CoreClass>::optionsForClass("abstract").size(), Equals(2));
    });

    it("creates a non-NULL default class instance", [&]() {
      AssertThat(!!Factory<CoreClass>::createDefault("abstract").get(), IsTrue());
    });

    it("creates the *proper* non-NULL default class instance", [&]() {
      AssertThat(Factory<CoreClass>::createDefaultAs<abstract>("abstract")->stupid()[0], Equals('A'));
    });

    it("creates the proper non-NULL *specified group* class instance", [&]() {
      AssertThat(Factory<CoreClass>::createSubclassForGroupAs<abstract>("abstract", 66)->stupid()[0], Equals('B'));
    });

    it("creates a non-NULL instance given only a concrete class name", [&]() {
      AssertThat(Factory<CoreClass>::createConcreteClassAs<A>("A")->stupid()[0], Equals('A'));
    });

  });
});
