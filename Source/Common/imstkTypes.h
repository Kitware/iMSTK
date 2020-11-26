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

#include <limits>

namespace imstk
{
using ScalarType = unsigned char;
#define IMSTK_VOID 0
#define IMSTK_CHAR 1
#define IMSTK_UNSIGNED_CHAR 2
#define IMSTK_SHORT 3
#define IMSTK_UNSIGNED_SHORT 4
#define IMSTK_INT 5
#define IMSTK_UNSIGNED_INT 6
#define IMSTK_LONG 7
#define IMSTK_UNSIGNED_LONG 8
#define IMSTK_FLOAT 9
#define IMSTK_DOUBLE 10
#define IMSTK_LONG_LONG 11
#define IMSTK_UNSIGNED_LONG_LONG 12

#define IMSTK_CHAR_MAX std::numeric_limits<char>::max();
#define IMSTK_CHAR_MIN std::numeric_limits<char>::min();
#define IMSTK_UNSIGNED_CHAR_MAX std::numeric_limits<unsigned char>::max();
#define IMSTK_UNSIGNED_CHAR_MIN std::numeric_limits<unsigned char>::min();
#define IMSTK_SHORT_MAX std::numeric_limits<short>::max();
#define IMSTK_SHORT_MIN std::numeric_limits<short>::min();
#define IMSTK_UNSIGNED_SHORT_MAX std::numeric_limits<unsigned short>::max();
#define IMSTK_UNSIGNED_SHORT_MIN std::numeric_limits<unsigned short>::min();
#define IMSTK_INT_MAX std::numeric_limits<int>::max();
#define IMSTK_INT_MIN std::numeric_limits<int>::min();
#define IMSTK_UNSIGNED_INT_MAX std::numeric_limits<unsigned int>::max();
#define IMSTK_UNSIGNED_INT_MIN std::numeric_limits<unsigned int>::min();
#define IMSTK_LONG_MAX std::numeric_limits<long>::max();
#define IMSTK_LONG_MIN std::numeric_limits<long>::min();
#define IMSTK_UNSIGNED_LONG_MAX std::numeric_limits<unsigned long>::max();
#define IMSTK_UNSIGNED_LONG_MIN std::numeric_limits<unsigned long>::min();
#define IMSTK_FLOAT_MAX std::numeric_limits<float>::max();
#define IMSTK_FLOAT_MIN std::numeric_limits<float>::min();
#define IMSTK_DOUBLE_MAX std::numeric_limits<double>::max();
#define IMSTK_DOUBLE_MIN std::numeric_limits<double>::min();
#define IMSTK_LONG_LONG_MAX std::numeric_limits<long>::max();
#define IMSTK_LONG_LONG_MIN std::numeric_limits<long>::min();
#define IMSTK_UNSIGNED_LONG_LONG_MAX std::numeric_limits<unsigned long long>::max();
#define IMSTK_UNSIGNED_LONG_LONG_MIN std::numeric_limits<unsigned long long>::min();
}