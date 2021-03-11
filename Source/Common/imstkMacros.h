/*=========================================================================

   Library: iMSTK

   Copyright (c) Kitware, Inc. & Center for Modeling, Simulation,
   & Imaging in Medicine, Rensselaer Polytechnic Institute.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

=========================================================================*/

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
}