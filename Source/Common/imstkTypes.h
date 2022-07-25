/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#pragma once

#include <limits>

namespace imstk
{
typedef unsigned char ScalarTypeId;
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

using CellTypeId = unsigned char;
#define IMSTK_VERTEX 0
#define IMSTK_EDGE 1
#define IMSTK_TRIANGLE 2
#define IMSTK_QUAD 3
#define IMSTK_TETRAHEDRON 4
#define IMSTK_HEXAHEDRON 5

#define IMSTK_CHAR_MAX std::numeric_limits<char>::max()
#define IMSTK_CHAR_MIN std::numeric_limits<char>::min()
#define IMSTK_UNSIGNED_CHAR_MAX std::numeric_limits<unsigned char>::max()
#define IMSTK_UNSIGNED_CHAR_MIN std::numeric_limits<unsigned char>::min()
#define IMSTK_SHORT_MAX std::numeric_limits<short>::max()
#define IMSTK_SHORT_MIN std::numeric_limits<short>::min()
#define IMSTK_UNSIGNED_SHORT_MAX std::numeric_limits<unsigned short>::max()
#define IMSTK_UNSIGNED_SHORT_MIN std::numeric_limits<unsigned short>::min()
#define IMSTK_INT_MAX std::numeric_limits<int>::max()
#define IMSTK_INT_MIN std::numeric_limits<int>::min()
#define IMSTK_UNSIGNED_INT_MAX std::numeric_limits<unsigned int>::max()
#define IMSTK_UNSIGNED_INT_MIN std::numeric_limits<unsigned int>::min()
#define IMSTK_LONG_MAX std::numeric_limits<long>::max()
#define IMSTK_LONG_MIN std::numeric_limits<long>::min()
#define IMSTK_UNSIGNED_LONG_MAX std::numeric_limits<unsigned long>::max()
#define IMSTK_UNSIGNED_LONG_MIN std::numeric_limits<unsigned long>::min()
#define IMSTK_FLOAT_MAX std::numeric_limits<float>::max()
#define IMSTK_FLOAT_MIN std::numeric_limits<float>::lowest()
#define IMSTK_FLOAT_EPS std::numeric_limits<float>::epsilon()
#define IMSTK_DOUBLE_MAX std::numeric_limits<double>::max()
#define IMSTK_DOUBLE_MIN std::numeric_limits<double>::lowest()
#define IMSTK_DOUBLE_EPS std::numeric_limits<double>::epsilon()
#define IMSTK_LONG_LONG_MAX std::numeric_limits<long>::max()
#define IMSTK_LONG_LONG_MIN std::numeric_limits<long>::min()
#define IMSTK_UNSIGNED_LONG_LONG_MAX std::numeric_limits<unsigned long long>::max()
#define IMSTK_UNSIGNED_LONG_LONG_MIN std::numeric_limits<unsigned long long>::min()
} // namespace imstk