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

#include "smCollision/smCollisionUtilities.h"

int intersect3D_RayTriangleWithBarycentric( smVec3d P0, smVec3d P1, smVec3d V0, smVec3d V1, smVec3d V2, smVec3d *I, float &p_baryU, float &p_baryV, float &p_baryW, bool considerFrontFaceOnly )
{

    smVec3d u, v, n;             // triangle vectors
    smVec3d dir, w0, w;          // ray vectors
    float r, a, b;               // params to calc ray-plane intersect

    // get triangle edge vectors and plane normal
    u = V1 - V0;
    v = V2 - V0;
    n = u.cross( v );             // cross product

    if ( n.norm() < std::numeric_limits<float>::min() ) // triangle is degenerate
    {
        return -1;    // do not deal with this case
    }

    dir = P1 - P0;                 // ray direction vector
    w0 = P0 - V0;
    a = -n.dot( w0 );

    if ( considerFrontFaceOnly && a > 0 ) //consider only front face of the trignale if is chosen
    {
        return -1;
    }

    b = n.dot( dir );

    if ( fabs( b ) < std::numeric_limits<float>::min() )    // ray is  parallel to triangle plane
    {
        if ( a == 0 )               // ray lies in triangle plane
        {
            return 2;
        }
        else
        {
            return 0;    // ray disjoint from plane
        }
    }

    // get intersect point of ray with triangle plane
    r = a / b;

    if ( r < 0.0 )                  // ray goes away from triangle
    {
        return 0;    // => no intersect
    }

    // for a segment, also test if (r > 1.0) => no intersect

    *I = P0 + r * dir;             // intersect point of ray and plane

    // is I inside T?
    float uu, uv, vv, wu, wv, D;
    uu = u.dot( u );
    uv = u.dot( v );
    vv = v.dot( v );
    w = *I - V0;
    wu = w.dot( u );
    wv = w.dot( v );
    D = uv * uv - uu * vv;

    // get and test parametric coords
    float s, t;
    s = ( uv * wv - vv * wu ) / D;

    if ( s < 0.0 || s > 1.0 )       // I is outside T
    {
        return 0;
    }

    t = ( uv * wu - uu * wv ) / D;

    if ( t < 0.0 || ( s + t ) > 1.0 ) // I is outside T
    {
        return 0;
    }

    p_baryU = 1.0 - ( s + t );
    p_baryV = s;
    p_baryW = t;
    return 1;                       // I is in T
}
