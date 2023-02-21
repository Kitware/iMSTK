/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

namespace imstk
{
#define imstkNotUsed(x)

#define imstkSetMacro(name, dataType)       \
    virtual void set ## name(dataType _arg) \
    {                                       \
        if (this->m_ ## name != _arg)       \
        {                                   \
            this->m_ ## name = _arg;        \
        }                                   \
    }
#define imstkGetMacro(name, dataType) \
    virtual dataType get ## name() { return this->m_ ## name; }

#define imstkSetGetMacro(funcName, varName, dataType)              \
    void set ## funcName(const dataType& _arg) { varName = _arg; } \
    const dataType& get ## funcName() const { return varName; }

#define IMSTK_TYPE_NAME(name)                                      \
    static const std::string getStaticTypeName() { return #name; } \
    const std::string getTypeName() const override { return #name; }

// \todo Switch to template type lists
///
/// \brief Maps ScalarType type to templated function call
///
#define TemplateMacroCase(typeN, type, call) \
case typeN: { using IMSTK_TT = type; call; }; break
#define TemplateMacro(call)                                        \
    TemplateMacroCase(IMSTK_CHAR, char, call);                     \
    TemplateMacroCase(IMSTK_UNSIGNED_CHAR, unsigned char, call);   \
    TemplateMacroCase(IMSTK_SHORT, short, call);                   \
    TemplateMacroCase(IMSTK_UNSIGNED_SHORT, unsigned short, call); \
    TemplateMacroCase(IMSTK_INT, int, call);                       \
    TemplateMacroCase(IMSTK_UNSIGNED_INT, unsigned int, call);     \
    TemplateMacroCase(IMSTK_LONG, long, call);                     \
    TemplateMacroCase(IMSTK_UNSIGNED_LONG, unsigned long, call);   \
    TemplateMacroCase(IMSTK_FLOAT, float, call);                   \
    TemplateMacroCase(IMSTK_DOUBLE, double, call);                 \
    TemplateMacroCase(IMSTK_LONG_LONG, long long, call);           \
    TemplateMacroCase(IMSTK_UNSIGNED_LONG_LONG, unsigned long long, call)

///
/// \brief Returns scalar type given template
///
#define TypeTemplateMacro(templateType)                                              \
    (std::is_same<templateType, char>::value ? IMSTK_CHAR :                          \
     (std::is_same<templateType, unsigned char>::value ? IMSTK_UNSIGNED_CHAR :       \
      (std::is_same<templateType, short>::value ? IMSTK_SHORT :                      \
       (std::is_same<templateType, unsigned short>::value ? IMSTK_UNSIGNED_SHORT :   \
        (std::is_same<templateType, int>::value ? IMSTK_INT :                        \
         (std::is_same<templateType, unsigned int>::value ? IMSTK_UNSIGNED_INT :     \
          (std::is_same<templateType, long>::value ? IMSTK_LONG :                    \
           (std::is_same<templateType, unsigned long>::value ? IMSTK_UNSIGNED_LONG : \
            (std::is_same<templateType, float>::value ? IMSTK_FLOAT :                \
             (std::is_same<templateType, double>::value ? IMSTK_DOUBLE :             \
              (std::is_same<templateType, long long>::value ? IMSTK_LONG_LONG :      \
               (std::is_same<templateType, unsigned long long>::value ? IMSTK_UNSIGNED_LONG_LONG : 0))))))))))))
} // namespace imstk

// See https://www.fluentcpp.com/2019/08/30/how-to-disable-a-warning-in-cpp/
// When adding new warnings remember to add the DISABLE_ macro
// for all three sections MSVC, GCC/CLANG, other
#if defined(_MSC_VER)
#define DISABLE_WARNING_PUSH           __pragma(warning( push ))
#define DISABLE_WARNING_POP            __pragma(warning( pop ))
#define DISABLE_WARNING(warningNumber) __pragma(warning( disable : warningNumber ))

#define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER    DISABLE_WARNING(4100)
#define DISABLE_WARNING_UNREFERENCED_FUNCTION            DISABLE_WARNING(4505)
#define DISABLE_WARNING_HIDES_CLASS_MEMBER               DISABLE_WARNING(4458)
#define DISABLE_WARNING_PADDING                          DISABLE_WARNING(4324)
// other warnings you want to deactivate...

// Not seen in msvc or not checked, fix when working with windows

#elif defined(__GNUC__) || defined(__clang__)
#define DO_PRAGMA(X) _Pragma(#X)
#define DISABLE_WARNING_PUSH           DO_PRAGMA(GCC diagnostic push)
#define DISABLE_WARNING_POP            DO_PRAGMA(GCC diagnostic pop)
#define DISABLE_WARNING(warningName)   DO_PRAGMA(GCC diagnostic ignored #warningName)

#define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER    DISABLE_WARNING(-Wunused - parameter)
#define DISABLE_WARNING_UNREFERENCED_FUNCTION            DISABLE_WARNING(-Wunused - function)
// other warnings you want to deactivate...

// Not seen in gcc or not checked, fix when working with linux
#define DISABLE_WARNING_HIDES_CLASS_MEMBER
#define DISABLE_WARNING_PADDING

#else
#define DISABLE_WARNING_PUSH
#define DISABLE_WARNING_POP
#define DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
#define DISABLE_WARNING_UNREFERENCED_FUNCTION
#define DISABLE_WARNING_HIDES_CLASS_MEMBER
// other warnings you want to deactivate...

#endif