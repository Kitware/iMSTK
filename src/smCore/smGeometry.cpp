/*=========================================================================
 * Copyright (c) Center for Modeling, Simulation, and Imaging in Medicine,
 *                        Rensselaer Polytechnic Institute
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 /=========================================================================
 
 /**
  *  \brief
  *  \details
  *  \author
  *  \author
  *  \copyright Apache License, Version 2.0.
  */

#include "smCore/smGeometry.h"
#include "smCore/smConfig.h"
#include "smUtilities/smVec3.h"
#include "smExternal/moller.h"
#include "smExternal/moller2.h"

extern inline int tri_tri_intersect(float V0[3], float V1[3], float V2[3],
                                    float U0[3], float U1[3], float U2[3]);

extern inline int tri_tri_intersect_with_isectline_penetrationDepth(float V0[3],
        float V1[3], float V2[3], float U0[3], float U1[3], float U2[3],
        int *coplanar, float isectpt1[3], float isectpt2[3],
        short &vSinglePoint, short &uSinglePoint, float point1[3],
        float point2[3]);

extern inline int triBoxOverlap(float boxcenter[3], float boxhalfsize[3],
                                float triverts[3][3]);

void smAABB::subDivide(smFloat  p_length, smInt p_divison, smAABB *p_aabb)
{

    smInt index = 0;

    for (smInt ix = 0; ix < p_divison; ix++)
        for (smInt iy = 0; iy < p_divison; iy++)
            for (smInt iz = 0; iz < p_divison; iz++)
            {
                p_aabb[index].aabbMin.x = aabbMin.x + p_length * ix;
                p_aabb[index].aabbMin.y = aabbMin.y + p_length * iy;
                p_aabb[index].aabbMin.z = aabbMin.z + p_length * iz;

                p_aabb[index].aabbMax.x = p_aabb[index].aabbMin.x + p_length;
                p_aabb[index].aabbMax.y = p_aabb[index].aabbMin.y + p_length;
                p_aabb[index].aabbMax.z = p_aabb[index].aabbMin.z + p_length;
                index++;
            }
}

void smAABB::subDivide(smInt p_divisionX, smInt p_divisionY, smInt p_divisionZ ,
                       smAABB *p_aabb)
{

    smFloat stepX;
    smFloat stepY;
    smFloat stepZ;

    stepX = (aabbMax.x - aabbMin.x) / p_divisionX;
    stepY = (aabbMax.y - aabbMin.y) / p_divisionY;
    stepZ = (aabbMax.z - aabbMin.z) / p_divisionZ;
    smInt index = 0;

    for (smInt ix = 0; ix < p_divisionX; ix++)
        for (smInt iy = 0; iy < p_divisionY; iy++)
            for (smInt iz = 0; iz < p_divisionZ; iz++)
            {

                p_aabb[index].aabbMin.x = aabbMin.x + stepX * ix;
                p_aabb[index].aabbMin.y = aabbMin.y + stepY * iy;
                p_aabb[index].aabbMin.z = aabbMin.z + stepZ * iz;

                p_aabb[index].aabbMax.x = p_aabb[index].aabbMin.x + stepX;
                p_aabb[index].aabbMax.y = p_aabb[index].aabbMin.y + stepY;
                p_aabb[index].aabbMax.z = p_aabb[index].aabbMin.z + stepZ;
                index++;
            }
}

smBool smCollisionUtils::checkAABBTriangle(smAABB &p_aabb, smVec3f v1,
        smVec3f v2, smVec3f v3)
{

    smVec3f  boxhalfsize;
    smVec3f boxCenter;
    smFloat triverts[3][3];
    boxhalfsize.x = p_aabb.halfSizeX();
    boxhalfsize.y = p_aabb.halfSizeY();
    boxhalfsize.z = p_aabb.halfSizeZ();

    boxCenter = p_aabb.center();
    triverts[0][0] = v1.x;
    triverts[0][1] = v1.y;
    triverts[0][2] = v1.z;

    triverts[1][0] = v2.x;
    triverts[1][1] = v2.y;
    triverts[1][2] = v2.z;

    triverts[2][0] = v3.x;
    triverts[2][1] = v3.y;
    triverts[2][2] = v3.z;

    if (triBoxOverlap((float*)&boxCenter, (float*)&boxhalfsize, triverts) == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

smBool smCollisionUtils::tri2tri(smVec3<smFloat> &p_tri1Point1,
                                 smVec3<smFloat> &p_tri1Point2,
                                 smVec3<smFloat> &p_tri1Point3,
                                 smVec3<smFloat> &p_tri2Point1,
                                 smVec3<smFloat> &p_tri2Point2,
                                 smVec3<smFloat> &p_tri2Point3,
                                 smInt &coplanar,
                                 smVec3<smFloat> &p_intersectionPoint1,
                                 smVec3<smFloat> &p_intersectionPoint2,
                                 smShort &p_tri1SinglePointIndex,
                                 smShort &p_tri2SinglePointIndex,
                                 smVec3<smFloat> &p_projPoint1,
                                 smVec3<smFloat> &p_projPoint2)
{

    if (tri_tri_intersect_with_isectline_penetrationDepth((smFloat*)&p_tri1Point1,
            (smFloat*)&p_tri1Point2, (smFloat*)&p_tri1Point3,
            (smFloat*)&p_tri2Point1, (smFloat*)&p_tri2Point2,
            (smFloat*)&p_tri2Point3, &coplanar,
            (smFloat*)&p_intersectionPoint1, (smFloat*)&p_intersectionPoint2,
            p_tri1SinglePointIndex, p_tri2SinglePointIndex,
            (smFloat*)&p_projPoint1, (smFloat*)&p_projPoint2) == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

smBool smCollisionUtils::tri2tri(smVec3<smFloat> &p_tri1Point1,
                                 smVec3<smFloat> &p_tri1Point2,
                                 smVec3<smFloat> &p_tri1Point3,
                                 smVec3<smFloat> &p_tri2Point1,
                                 smVec3<smFloat> &p_tri2Point2,
                                 smVec3<smFloat> &p_tri2Point3)
{

    return (tri_tri_intersect((smFloat*)&p_tri1Point1, (smFloat*)&p_tri1Point2,
                              (smFloat*)&p_tri1Point3, (smFloat*)&p_tri2Point1,
                              (smFloat*)&p_tri2Point2, (smFloat*)&p_tri2Point3) == 1 ? true : false);
}


smBool smCollisionUtils::checkLineTri(smVec3<smFloat> &p_linePoint1,
                                      smVec3<smFloat> &p_linePoint2,
                                      smVec3<smFloat> &p_tri1Point1,
                                      smVec3<smFloat> &p_tri1Point2,
                                      smVec3<smFloat> &p_tri1Point3,
                                      smVec3<smFloat> &p_interSection)
{

    //First find that ray intersect the polygon or not
    smInt i1, i2;
    smVec3<smDouble> U, V;
    smDouble plane[4], P[3], u[3], v[3], alpha, beta;

    //Now find the Normal vector
    U = p_tri1Point2 - p_tri1Point1;
    V = p_tri1Point3 - p_tri1Point1;

    smVec3<smFloat> cProd;
    cProd = U.cross(V);
    cProd.normalize();

    plane[0] = cProd.x;
    plane[1] = cProd.y;
    plane[2] = cProd.z;
    plane[3] = -((plane[0] * p_tri1Point1[0] + plane[1] * p_tri1Point1[1] + plane[2] * p_tri1Point1[2]));

    //Now we got the plane equation and we can test whether our ray intersect or not
    smFloat d1, d2, d, t;
    smVec3<smFloat> dir;

    d1 = p_linePoint1.dot(cProd) + plane[3];
    d2 = p_linePoint2.dot(cProd) + plane[3];
    dir = p_linePoint2 - p_linePoint1;
    d = dir.dot(cProd);

    //if d1 and d2 are of same sign then the line segment didn't intersect
    //If they are opposite sign then there is a potential intersection
    if ((d1 * d2 < 0.0))
    {

        //Now the ray intersect the polygon but we have to test whether the intersection point
        //lie inside the triangle
        t = -(d1) / d;

        //Find the dominant Normal vector
        if ((fabs(plane[0]) >= fabs(plane[1])) && (fabs(plane[0]) >= fabs(plane[2])))
        {
            // X direction is dominant
            i1 = 1;
            i2 = 2;
        }
        else if ((fabs(plane[1]) >= fabs(plane[0])) && (fabs(plane[1]) >= fabs(plane[2])))
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
        if (u[1] == 0)
        {
            beta = u[0] / u[2];
            alpha = (v[0] - beta * v[2]) / v[1];
        }
        else
        {
            beta = (v[0] * u[1] - u[0] * v[1]) / (v[2] * u[1] - u[2] * v[1]);
            alpha = (u[0] - beta * u[2]) / u[1];
        }

        if ((alpha >= 0) && (beta >= 0) && (alpha + beta <= 1))
        {
            //this is a valid intersection
            //store the intersection point and vertices and polygon number
            p_interSection[0] = p_linePoint1[0] + t * dir[0];
            p_interSection[1] = p_linePoint1[1] + t * dir[1];
            p_interSection[2] = p_linePoint1[2] + t * dir[2];
            return (TRUE);
        }
    }

    return (FALSE);
}

/// \brief If the point is in the triangle, it will also send the barycentric coord.
int intersect3D_RayTriangleWithBarycentric(smVec3f P0, smVec3f P1, smVec3f V0,
        smVec3f V1, smVec3f V2, smVec3f *I,
        float &p_baryU, float &p_baryV,
        float &p_baryW,
        bool considerFrontFaceOnly)
{

    smVec3f u, v, n;             // triangle vectors
    smVec3f dir, w0, w;          // ray vectors
    float r, a, b;               // params to calc ray-plane intersect

    // get triangle edge vectors and plane normal
    u = V1 - V0;
    v = V2 - V0;
    n = u.cross(v);               // cross product

    if (n.module() < SMALL_NUM)   // triangle is degenerate
    {
        return -1;    // do not deal with this case
    }

    dir = P1 - P0;                 // ray direction vector
    w0 = P0 - V0;
    a = -n.dot(w0);

    if (considerFrontFaceOnly && a > 0) //consider only front face of the trignale if is chosen
    {
        return -1;
    }

    b = n.dot(dir);

    if (fabs(b) < SMALL_NUM)        // ray is  parallel to triangle plane
    {
        if (a == 0)                 // ray lies in triangle plane
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

    if (r < 0.0)                    // ray goes away from triangle
    {
        return 0;    // => no intersect
    }

    // for a segment, also test if (r > 1.0) => no intersect

    *I = P0 + r * dir;             // intersect point of ray and plane

    // is I inside T?
    float uu, uv, vv, wu, wv, D;
    uu = u.dot(u);
    uv = u.dot(v);
    vv = v.dot(v);
    w = *I - V0;
    wu = w.dot(u);
    wv = w.dot(v);
    D = uv * uv - uu * vv;

    // get and test parametric coords
    float s, t;
    s = (uv * wv - vv * wu) / D;

    if (s < 0.0 || s > 1.0)         // I is outside T
    {
        return 0;
    }

    t = (uv * wu - uu * wv) / D;

    if (t < 0.0 || (s + t) > 1.0)  // I is outside T
    {
        return 0;
    }

    p_baryU = 1.0 - (s + t);
    p_baryV = s;
    p_baryW = t;
    return 1;                       // I is in T
}
