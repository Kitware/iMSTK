/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details.
*/

#include "imstkVecDataArray.h"

#include <gtest/gtest.h>

using namespace imstk;

template<typename T>
std::string
to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

///
/// \brief Assert that the vertices are within min and max bounds
///
template<typename T>
testing::AssertionResult
assertBounds(const VecDataArray<T, 3>& vertices,
             const Eigen::Vector<T, 3>& min, const Eigen::Vector<T, 3>& max)
{
    if (min[0] > max[0] || min[1] > max[1] || min[2] > max[2])
    {
        return testing::AssertionFailure() << "Min must be less than max";
    }
    for (int i = 0; i < vertices.size(); i++)
    {
        const Eigen::Vector<T, 3>& vertex = vertices[i];
        if (vertex[0] < min[0] || vertex[1] < min[1] || vertex[2] < min[2]
            || vertex[0] > max[0] || vertex[1] > max[1] || vertex[2] > max[2])
        {
            return testing::AssertionFailure() << "Vertex " << i << " is outside of bounds";
        }
    }
    return testing::AssertionSuccess();
}

///
/// \brief Assert that the displacement between the sets of vertices is beneath
/// minDisplacement
///
template<typename T>
testing::AssertionResult
assertMinDisplacement(const VecDataArray<T, 3>& verticesT1,
                      const VecDataArray<T, 3>& verticesT2, const T minDisplacement)
{
    if (verticesT1.size() != verticesT2.size())
    {
        return testing::AssertionFailure() << "Vertices size must be the same. " << std::endl <<
               "verticesT1 size is " << verticesT1.size() << std::endl <<
               "verticesT2 size is " << verticesT2.size();
    }
    for (int i = 0; i < verticesT1.size(); i++)
    {
        const double displacementMag = (verticesT2[i] - verticesT1[i]).norm();
        if (displacementMag >= minDisplacement)
        {
            return testing::AssertionFailure() << "Vertex " << i << " has displacement "
                                               << displacementMag << " which is larger than alotted " << minDisplacement;
        }
    }
    return testing::AssertionSuccess();
}