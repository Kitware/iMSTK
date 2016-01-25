#include <bandit/bandit.h>

#include "Core/Factory.h"

#include <iostream>

class abstract : public imstk::CoreClass
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

IMSTK_BEGIN_DYNAMIC_LOADER()
  IMSTK_BEGIN_ONLOAD(register_abstract_children)
    IMSTK_REGISTER_CLASS(imstk::CoreClass, abstract, A, 65);
    IMSTK_REGISTER_CLASS(imstk::CoreClass, abstract, B, 66);
  IMSTK_FINISH_ONLOAD()
IMSTK_FINISH_DYNAMIC_LOADER()

using namespace bandit;

go_bandit([](){
  describe("factory", []() {
    IMSTK_RUN_LOADER(register_abstract_children);

    it("shows 2 subclasses of \"abstract\"", [&]() {
        AssertThat(imstk::Factory<imstk::CoreClass>::optionsForClass("abstract").size(), Equals(2));
    });

    it("creates a non-nullptr default class instance", [&]() {
        AssertThat(!!imstk::Factory<imstk::CoreClass>::createDefault("abstract").get(), IsTrue());
    });

    it("creates the *proper* non-nullptr default class instance", [&]() {
        AssertThat(imstk::Factory<imstk::CoreClass>::createDefaultAs<abstract>("abstract")->stupid()[0], Equals('A'));
    });

    it("creates the proper non-nullptr *specified group* class instance", [&]() {
        AssertThat(imstk::Factory<imstk::CoreClass>::createSubclassForGroupAs<abstract>("abstract", 66)->stupid()[0], Equals('B'));
    });

    it("creates a non-nullptr instance given only a concrete class name", [&]() {
        AssertThat(imstk::Factory<imstk::CoreClass>::createConcreteClassAs<A>("A")->stupid()[0], Equals('A'));
    });

  });
});
