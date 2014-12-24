/*
****************************************************
                SimMedTK LICENSE
****************************************************

****************************************************
*/

#ifndef SMGEOMETRY_H
#define SMGEOMETRY_H

#include "smCore/smConfig.h"
#include "smUtilities/smVec3.h"

/// \brief a small number used for accuracy in collision computation
#define SMALL_NUM 0.00000001

//forward declaration
struct smSphere;
/// \brief  Simple Plane definition with unit normal and spatial location
struct smPlane
{
    smVec3f unitNormal;
    smVec3f pos;
    inline smFloat distance(smVec3f p_vector);
    inline smVec3f project(smVec3f p_vector);
};

/// \brief Axis Aligned bounding box declarions
struct smAABB
{
    /// \brief minimum x,y,z point
    smVec3<float> aabbMin;
    /// \brief maximum x,y,z point
    smVec3<float> aabbMax;

    /// \brief constrcutor. The default is set to origin for aabbMin and aabbMax
    inline smAABB()
    {
        aabbMin.setValue(0, 0, 0);
        aabbMax.setValue(0, 0, 0);

    }
    /// \brief center of the AABB
    inline smVec3f center()
    {
        return smVec3f((aabbMin.x + aabbMax.x) / 2.0,
                       (aabbMin.y + aabbMax.y) / 2.0,
                       (aabbMin.z + aabbMax.z) / 2.0);
    }

    /// \brief check if two AABB overlaps
    static inline smBool checkOverlap(smAABB &p_aabbA,  smAABB &p_aabbB)
    {

        if (p_aabbA.aabbMin.x > p_aabbB.aabbMax.x ||
                p_aabbA.aabbMax.x < p_aabbB.aabbMin.x ||
                p_aabbA.aabbMin.y > p_aabbB.aabbMax.y ||
                p_aabbA.aabbMax.y < p_aabbB.aabbMin.y ||
                p_aabbA.aabbMin.z > p_aabbB.aabbMax.z ||
                p_aabbA.aabbMax.z < p_aabbB.aabbMin.z)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    /// \brief set  p_aabb to the current one
    inline smAABB &operator=(smAABB &p_aabb)
    {
        aabbMax = p_aabb.aabbMax;
        aabbMin = p_aabb.aabbMin;
        return *this;
    }

    /// \brief scale the AABB
    inline smAABB &operator*(smFloat p_scale)
    {
        aabbMin = aabbMin * p_scale;
        aabbMax = aabbMax * p_scale;
        return *this;
    }
    /// \brief sub divides p_length will be used to create the slices
    void subDivide(smFloat  p_length , smInt p_divison, smAABB *p_aabb);
    /// \brief divides current AABB in x,y,z axes with specificed divisions. results are placed in p_aabb
    void subDivide(smInt p_divisionX, smInt p_divisionY, smInt p_divisionZ , smAABB *p_aabb);
    /// \brief divides current AABB in all axes with specificed p_division. results are placed in p_aabb
    inline void subDivide(smInt p_division, smAABB *p_aabb)
    {
        subDivide(p_division, p_division, p_division, p_aabb);
    }
    /// \brief returns half of X edge of AABB
    inline smFloat halfSizeX()
    {
        return (aabbMax.x - aabbMin.x) / 2.0;
    }
    /// \brief returns half of Y edge of AABB
    inline smFloat halfSizeY()
    {
        return (aabbMax.y - aabbMin.y) / 2.0;
    }
    /// \brief returns half of Z edge of AABB
    inline smFloat halfSizeZ()
    {
        return (aabbMax.z - aabbMin.z) / 2.0;
    }
    /// \brief expands aabb with p_factor
    inline void expand(smFloat p_factor)
    {
        aabbMin = aabbMin - (aabbMax - aabbMin) * p_factor / 2.0;
        aabbMax = aabbMax + (aabbMax - aabbMin) * p_factor / 2.0;
    }
};

/// \brief Collision utililites
class smCollisionUtils
{
public:
    /// \brief triangle and triangle collision retursn intersection and projection points
    static smBool tri2tri(smVec3<smFloat> &p_tri1Point1,
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
                          smVec3<smFloat> &p_projPoint2);

    /// \brief checks if the two triangles intersect
    static smBool tri2tri(smVec3<smFloat> &p_tri1Point1,
                          smVec3<smFloat> &p_tri1Point2,
                          smVec3<smFloat> &p_tri1Point3,
                          smVec3<smFloat> &p_tri2Point1,
                          smVec3<smFloat> &p_tri2Point2,
                          smVec3<smFloat> &p_tri2Point3);

    /// \brief check if the two AABB overlap returns encapsulating AABB of two
    static inline bool checkOverlapAABBAABB(smAABB &aabbA, smAABB &aabbB, smAABB &result)
    {

        if (aabbA.aabbMin.x > aabbB.aabbMax.x ||
                aabbA.aabbMax.x < aabbB.aabbMin.x ||
                aabbA.aabbMin.y > aabbB.aabbMax.y ||
                aabbA.aabbMax.y < aabbB.aabbMin.y ||
                aabbA.aabbMin.z > aabbB.aabbMax.z ||
                aabbA.aabbMax.z < aabbB.aabbMin.z)
        {
            return false;
        }
        else
        {
            result.aabbMin.x = SIMMEDTK_MAX(aabbA.aabbMin.x, aabbB.aabbMin.x);
            result.aabbMin.y = SIMMEDTK_MAX(aabbA.aabbMin.y, aabbB.aabbMin.y);
            result.aabbMin.z = SIMMEDTK_MAX(aabbA.aabbMin.z, aabbB.aabbMin.z);

            result.aabbMax.x = SIMMEDTK_MIN(aabbA.aabbMax.x, aabbB.aabbMax.x);
            result.aabbMax.y = SIMMEDTK_MIN(aabbA.aabbMax.y, aabbB.aabbMax.y);
            result.aabbMax.z = SIMMEDTK_MIN(aabbA.aabbMax.z, aabbB.aabbMax.z);
            return true;
        }
    }
    /// \brief check the AABBs overlap. returns true if they intersect
    static inline bool checkOverlapAABBAABB(smAABB &aabbA, smAABB &aabbB)
    {

        if (aabbA.aabbMin.x > aabbB.aabbMax.x ||
                aabbA.aabbMax.x < aabbB.aabbMin.x ||
                aabbA.aabbMin.y > aabbB.aabbMax.y ||
                aabbA.aabbMax.y < aabbB.aabbMin.y ||
                aabbA.aabbMin.z > aabbB.aabbMax.z ||
                aabbA.aabbMax.z < aabbB.aabbMin.z)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    /// \brief check if the point p_vertex is inside the AABB
    static inline bool checkOverlapAABBAABB(smAABB &aabbA, smVec3<smFloat> &p_vertex)
    {

        if (aabbA.aabbMin.x <= p_vertex.x && aabbA.aabbMax.x >= p_vertex.x &&
                aabbA.aabbMin.y <= p_vertex.y &&    aabbA.aabbMax.y >= p_vertex.y &&
                aabbA.aabbMin.z <= p_vertex.z &&    aabbA.aabbMax.z >= p_vertex.z)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    /// \brief  checks if the line intersects the tirangle. returns if it is true. the intersection is returned in  p_interSection
    static smBool checkLineTri(smVec3<smFloat> &p_linePoint1,
                               smVec3<smFloat> &p_linePoint2,
                               smVec3<smFloat> &p_tri1Point1,
                               smVec3<smFloat> &p_tri1Point2,
                               smVec3<smFloat> &p_tri1Point3,
                               smVec3<smFloat> &p_interSection);

    /// \brief checks if the triangles points are within the AABB
    static smBool checkAABBTriangle(smAABB &p_aabb, smVec3f v1, smVec3f v2, smVec3f v3);

    static inline smBool checkAABBPoint(smAABB &p_aabb, smVec3f p_v)
    {
        if (p_v.x >= p_aabb.aabbMin.x &&
                p_v.y >= p_aabb.aabbMin.y &&
                p_v.z >= p_aabb.aabbMin.z  &&
                p_v.x <=  p_aabb.aabbMax.x  &&
                p_v.y <= p_aabb.aabbMax.y &&
                p_v.z <= p_aabb.aabbMax.z)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
};

/// \brief sphere structure
struct smSphere
{

public:
    /// \brief center of sphere
    smVec3f center;
    /// \brief radius of sshere
    smFloat radius;
    /// \brief constructor
    inline smSphere()
    {
        center.setValue(0, 0, 0);
        radius = 1.0;
    }
    /// \brief sphere constructor with center and radius
    inline smSphere(smVec3f p_center, smFloat p_radius)
    {
        radius = p_radius;
        center = p_center;
    }
};

/// \brief cube
struct smCube
{
    /// \brief cube center
    smVec3f center;
    /// \brief cube length
    smFloat sideLength;

    /// \brief constructor
    inline smCube()
    {
        center.setValue(0, 0, 0);
        sideLength = 1.0;
    }
    /// \brief subdivides the cube in mulitple cube with given number of cubes identified for each axis with p_divisionPerAxis
    inline void subDivide(smInt p_divisionPerAxis, smCube*p_cube)
    {
        smVec3f minPoint;
        smFloat divLength = (sideLength / p_divisionPerAxis);
        smInt index = 0;
        minPoint.setValue(center.x - sideLength * 0.5, center.y - sideLength * 0.5, center.z - sideLength * 0.5);

        for (smInt ix = 0; ix < p_divisionPerAxis; ix++)
            for (smInt iy = 0; iy < p_divisionPerAxis; iy++)
                for (smInt iz = 0; iz < p_divisionPerAxis; iz++)
                {
                    p_cube[index].center.x = minPoint.x + divLength * ix + divLength * 0.5;
                    p_cube[index].center.y = minPoint.y + divLength * iy + divLength * 0.5;
                    p_cube[index].center.z = minPoint.z + divLength * iz + divLength * 0.5;
                    p_cube[index].sideLength = divLength;


                    index++;
                }
    }
    /// \brief expands the cube. increases the edge length with expansion*edge length
    inline void expand(smFloat p_expansion)
    {
        sideLength = sideLength + sideLength * p_expansion;
    }
    /// \brief returns the left most corner
    inline smVec3f leftMinCorner()
    {
        return smVec3f(center.x - sideLength * 0.5,
                       center.y - sideLength * 0.5,
                       center.z - sideLength * 0.5);
    }
    /// \brief returns right most corner
    inline smVec3f rightMaxCorner()
    {
        return smVec3f(center.x + sideLength * 0.5,
                       center.y + sideLength * 0.5,
                       center.z + sideLength * 0.5);
    }
    /// \brief returns the smallest sphere encapsulates the cube
    inline smSphere getCircumscribedSphere()
    {
        return smSphere(center, 0.866025 * sideLength);
    }
    /// \brief returns the  sphere with half edge of the cube as a radius
    inline smSphere getInscribedSphere()
    {
        return smSphere(center, sideLength * 0.5);
    }
    /// \brief get tangent sphere
    inline smSphere getTangent2EdgeSphere()
    {
        return smSphere(center, sideLength * 0.707106);
    }
};
/// \brief computes 3d ray intersction with triangle. The inserction point is given in barycentric coordinates.
int intersect3D_RayTriangleWithBarycentric(smVec3f P0, smVec3f P1, smVec3f V0,
        smVec3f V1, smVec3f V2, smVec3f *I,
        float &p_baryU, float &p_baryV,
        float &p_baryW, bool considerFrontFaceOnly);

#endif
