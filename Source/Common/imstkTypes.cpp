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

#include "imstkTypes.h"

namespace imstk
{
template<typename T>
static T
Variant::ToNumeric(const Variant& v)
{
    switch (v.type)
    {
    case IMSTK_CHAR:
        return static_cast<T>(v.Data.char_value);
        break;

    case IMSTK_UNSIGNED_CHAR:
        return static_cast<T>(v.Data.uchar_value);
        break;

    case IMSTK_SHORT:
        return static_cast<T>(v.Data.short_value);
        break;

    case IMSTK_UNSIGNED_SHORT:
        return static_cast<T>(v.Data.ushort_value);
        break;

    case IMSTK_INT:
        return static_cast<T>(v.Data.int_value);
        break;

    case IMSTK_UNSIGNED_INT:
        return static_cast<T>(v.Data.uint_value);
        break;

    case IMSTK_LONG:
        return static_cast<T>(v.Data.long_value);
        break;

    case IMSTK_UNSIGNED_LONG:
        return static_cast<T>(v.Data.ulong_value);
        break;

    case IMSTK_FLOAT:
        return static_cast<T>(v.Data.float_value);
        break;

    case IMSTK_DOUBLE:
        return static_cast<T>(v.Data.double_value);
        break;

    case IMSTK_LONG_LONG:
        return static_cast<T>(v.Data.longlong_value);
        break;

    case IMSTK_UNSIGNED_LONG_LONG:
        return static_cast<T>(v.Data.ulonglong_value);
        break;
    default:
        return static_cast<T>(0);
        break;
    }
    ;
}

char
Variant::getChar() const
{
    return ToNumeric<char>(*this);
}

unsigned char
Variant::getUnsignedChar() const
{
    return ToNumeric<unsigned char>(*this);
}

short
Variant::getShort() const
{
    return ToNumeric<short>(*this);
}

unsigned short
Variant::getUshort() const
{
    return ToNumeric<unsigned short>(*this);
}

int
Variant::getInt() const
{
    return ToNumeric<int>(*this);
}

unsigned int
Variant::getUnsignedInt() const
{
    return ToNumeric<unsigned int>(*this);
}

long
Variant::getLong() const
{
    return ToNumeric<long>(*this);
}

unsigned long
Variant::getUnsignedLong() const
{
    return ToNumeric<unsigned long>(*this);
}

float
Variant::getFloat() const
{
    return ToNumeric<float>(*this);
}

double
Variant::getDouble() const
{
    return ToNumeric<double>(*this);
}

long long
Variant::getLongLong() const
{
    return ToNumeric<long long>(*this);
}

unsigned long long
Variant::getUnsignedLongLong() const
{
    return ToNumeric<unsigned long long>(*this);
}
}