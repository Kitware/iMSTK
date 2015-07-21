// This file is part of the SimMedTK project.
// Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
//                        Rensselaer Polytechnic Institute
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//---------------------------------------------------------------------------
//
// Authors:
//
// Contact:
//---------------------------------------------------------------------------

#include "CollisionMoller.h"

// SimMedTK includes
#include "Core/Matrix.h"
#include "External/moller.h"
#include "External/moller2.h"

bool CollisionMoller::tri2tri( core::Vec3d &tri1Point1,
                                   core::Vec3d &tri1Point2,
                                   core::Vec3d &tri1Point3,
                                   core::Vec3d &tri2Point1,
                                   core::Vec3d &tri2Point2,
                                   core::Vec3d &tri2Point3,
                                   double &depth,
                                   core::Vec3d &contactPoint,
                                   core::Vec3d &normal)
{
    int coplanar;
    core::Vec3d intersectionPoint2;
    short tri1SinglePointIndex;
    short tri2SinglePointIndex;
    core::Vec3d projPoint1;
    core::Vec3d projPoint2;
    if ( tri_tri_intersect_with_isectline_penetrationDepth( tri1Point1.data(),
                                                            tri1Point2.data(),
                                                            tri1Point3.data(),
                                                            tri2Point1.data(),
                                                            tri2Point2.data(),
                                                            tri2Point3.data(),
                                                            &coplanar,
                                                            contactPoint.data(),
                                                            intersectionPoint2.data(),
                                                            tri1SinglePointIndex,
                                                            tri2SinglePointIndex,
                                                            projPoint1.data(),
                                                            projPoint2.data(),
                                                            depth,
                                                            normal.data()
                                                          ) == 1 )
    {
        return true;
    }
    return false;
}

bool CollisionMoller::tri2tri( core::Vec3d &p_tri1Point1,
                                   core::Vec3d &p_tri1Point2,
                                   core::Vec3d &p_tri1Point3,
                                   core::Vec3d &p_tri2Point1,
                                   core::Vec3d &p_tri2Point2,
                                   core::Vec3d &p_tri2Point3,
                                   int &coplanar,
                                   core::Vec3d &p_intersectionPoint1,
                                   core::Vec3d &p_intersectionPoint2,
                                   short &p_tri1SinglePointIndex,
                                   short &p_tri2SinglePointIndex,
                                   core::Vec3d &p_projPoint1,
                                   core::Vec3d &p_projPoint2 )
{
    double depth;
    core::Vec3d normal;
    if ( tri_tri_intersect_with_isectline_penetrationDepth( p_tri1Point1.data(),
                                                            p_tri1Point2.data(),
                                                            p_tri1Point3.data(),
                                                            p_tri2Point1.data(),
                                                            p_tri2Point2.data(),
                                                            p_tri2Point3.data(),
                                                            &coplanar,
                                                            p_intersectionPoint1.data(),
                                                            p_intersectionPoint2.data(),
                                                            p_tri1SinglePointIndex,
                                                            p_tri2SinglePointIndex,
                                                            p_projPoint1.data(),
                                                            p_projPoint2.data(),depth,normal.data() ) == 1 )
    {
        return true;
    }
    return false;
}

bool CollisionMoller::tri2tri( core::Vec3d &p_tri1Point1, core::Vec3d &p_tri1Point2, core::Vec3d &p_tri1Point3, core::Vec3d &p_tri2Point1, core::Vec3d &p_tri2Point2, core::Vec3d &p_tri2Point3 )
{
    return ( tri_tri_intersect( p_tri1Point1.data(), p_tri1Point2.data(),
                                p_tri1Point3.data(), p_tri2Point1.data(),
                                p_tri2Point2.data(), p_tri2Point3.data() ) == 1 ? true : false );
}

bool CollisionMoller::checkOverlapAABBAABB( AABB &aabbA, AABB &aabbB, AABB &result )
{

    if ( aabbA.aabbMin[0] > aabbB.aabbMax[0] ||
         aabbA.aabbMax[0] < aabbB.aabbMin[0] ||
         aabbA.aabbMin[1] > aabbB.aabbMax[1] ||
         aabbA.aabbMax[1] < aabbB.aabbMin[1] ||
         aabbA.aabbMin[2] > aabbB.aabbMax[2] ||
         aabbA.aabbMax[2] < aabbB.aabbMin[2] )
    {
        return false;
    }
    else
    {
        result.aabbMin[0] = std::max( aabbA.aabbMin[0], aabbB.aabbMin[0] );
        result.aabbMin[1] = std::max( aabbA.aabbMin[1], aabbB.aabbMin[1] );
        result.aabbMin[2] = std::max( aabbA.aabbMin[2], aabbB.aabbMin[2] );

        result.aabbMax[0] = std::min( aabbA.aabbMax[0], aabbB.aabbMax[0] );
        result.aabbMax[1] = std::min( aabbA.aabbMax[1], aabbB.aabbMax[1] );
        result.aabbMax[2] = std::min( aabbA.aabbMax[2], aabbB.aabbMax[2] );
        return true;
    }
}

bool CollisionMoller::checkOverlapAABBAABB( const AABB &aabbA, const AABB &aabbB )
{

    const core::Vec3d &min = aabbA.getMin();
    const core::Vec3d &max = aabbA.getMax();

    if ( min[0] > aabbB.aabbMax[0] ||
         max[0] < aabbB.aabbMin[0] ||
         min[1] > aabbB.aabbMax[1] ||
         max[1] < aabbB.aabbMin[1] ||
         min[2] > aabbB.aabbMax[2] ||
         max[2] < aabbB.aabbMin[2] )
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool CollisionMoller::checkOverlapAABBAABB( AABB &aabbA, core::Vec3d &p_vertex )
{

    if ( aabbA.aabbMin[0] <= p_vertex[0] && aabbA.aabbMax[0] >= p_vertex[0] &&
         aabbA.aabbMin[1] <= p_vertex[1] && aabbA.aabbMax[1] >= p_vertex[1] &&
         aabbA.aabbMin[2] <= p_vertex[2] && aabbA.aabbMax[2] >= p_vertex[2] )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CollisionMoller::checkAABBPoint( const AABB &p_aabb, const core::Vec3d &p_v )
{
    if ( p_v[0] >= p_aabb.aabbMin[0] &&
         p_v[1] >= p_aabb.aabbMin[1] &&
         p_v[2] >= p_aabb.aabbMin[2] &&
         p_v[0] <= p_aabb.aabbMax[0] &&
         p_v[1] <= p_aabb.aabbMax[1] &&
         p_v[2] <= p_aabb.aabbMax[2] )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CollisionMoller::checkLineTri( core::Vec3d &p_linePoint1, core::Vec3d &p_linePoint2, core::Vec3d &p_tri1Point1, core::Vec3d &p_tri1Point2, core::Vec3d &p_tri1Point3, core::Vec3d &p_interSection )
{

    //First find that ray intersect the polygon or not
    int i1, i2;
    core::Vec3d U, V;
    double plane[4], P[3], u[3], v[3], alpha, beta;

    //Now find the Normal vector
    U = p_tri1Point2 - p_tri1Point1;
    V = p_tri1Point3 - p_tri1Point1;

    core::Vec3d cProd;
    cProd = U.cross( V );
    cProd.normalize();

    plane[0] = cProd[0];
    plane[1] = cProd[1];
    plane[2] = cProd[2];
    plane[3] = -( ( plane[0] * p_tri1Point1[0] + plane[1] * p_tri1Point1[1] + plane[2] * p_tri1Point1[2] ) );

    //Now we got the plane equation and we can test whether our ray intersect or not
    double d1, d2, d, t;
    core::Vec3d dir;

    d1 = p_linePoint1.dot( cProd ) + plane[3];
    d2 = p_linePoint2.dot( cProd ) + plane[3];
    dir = p_linePoint2 - p_linePoint1;
    d = dir.dot( cProd );

    //if d1 and d2 are of same sign then the line segment didn't intersect
    //If they are opposite sign then there is a potential intersection
    if ( ( d1 * d2 < 0.0 ) )
    {

        //Now the ray intersect the polygon but we have to test whether the intersection point
        //lie inside the triangle
        t = -( d1 ) / d;

        //Find the dominant Normal vector
        if ( ( fabs( plane[0] ) >= fabs( plane[1] ) ) && ( fabs( plane[0] ) >= fabs( plane[2] ) ) )
        {
            // X direction is dominant
            i1 = 1;
            i2 = 2;
        }
        else if ( ( fabs( plane[1] ) >= fabs( plane[0] ) ) && ( fabs( plane[1] ) >= fabs( plane[2] ) ) )
        {
            //Y direction is dominant
            i1 = 0;
            i2 = 2;
        }
        else
        {
            //Z direction is dominant
            i1 = 0;
            i2 = 1;
        }

        //Now we broke down to 2D problem
        P[0] = p_linePoint1[i1] + t * dir[i1];
        P[1] = p_linePoint1[i2] + t * dir[i2];

        u[0] = P[0] - p_tri1Point1[i1];
        v[0] = P[1] - p_tri1Point1[i2];
        u[1] = p_tri1Point2[i1] - p_tri1Point1[i1];
        v[1] = p_tri1Point2[i2] - p_tri1Point1[i2];
        u[2] = p_tri1Point3[i1] - p_tri1Point1[i1];
        v[2] = p_tri1Point3[i2] - p_tri1Point1[i2];

        //find alpha and beta
        /* calculate and compare barycentric coordinates */
        if ( u[1] == 0 )
        {
            beta = u[0] / u[2];
            alpha = ( v[0] - beta * v[2] ) / v[1];
        }
        else
        {
            beta = ( v[0] * u[1] - u[0] * v[1] ) / ( v[2] * u[1] - u[2] * v[1] );
            alpha = ( u[0] - beta * u[2] ) / u[1];
        }

        if ( ( alpha >= 0 ) && ( beta >= 0 ) && ( alpha + beta <= 1 ) )
        {
            //this is a valid intersection
            //store the intersection point and vertices and polygon number
            p_interSection[0] = p_linePoint1[0] + t * dir[0];
            p_interSection[1] = p_linePoint1[1] + t * dir[1];
            p_interSection[2] = p_linePoint1[2] + t * dir[2];
            return ( true );
        }
    }

    return false;
}

bool CollisionMoller::checkAABBTriangle( AABB &p_aabb, core::Vec3d &v1, core::Vec3d &v2, core::Vec3d &v3 )
{

    core::Vec3d boxhalfsize;
    core::Vec3d boxCenter;
    Matrix33d triverts;
    triverts << v1[0],v1[1],v1[2],
                v2[0],v2[1],v2[2],
                v3[0],v3[1],v3[2];
    boxhalfsize[0] = p_aabb.halfSizeX();
    boxhalfsize[1] = p_aabb.halfSizeY();
    boxhalfsize[2] = p_aabb.halfSizeZ();

    boxCenter = p_aabb.center();

    if ( triBoxOverlap( boxCenter.data(), boxhalfsize.data(), triverts ) == 1 )
    {
        return true;
    }
    return false;
}
