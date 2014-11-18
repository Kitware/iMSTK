/*
****************************************************
SOFMIS LICENSE

****************************************************

\author:    <http:\\acor.rpi.edu>
SOFMIS TEAM IN ALPHABATIC ORDER
Anderson Maciel, Ph.D.
Ganesh Sankaranarayanan, Ph.D.
Sreekanth A Venkata
Suvranu De, Ph.D.
Tansel Halic
Zhonghua Lu

\author:    Module by Tansel Halic


\version    1.0
\date       04/2009
\bug	    None yet
\brief	    This Module has the data strutures and some utility functions. 

*****************************************************
*/

#ifndef SMGEOMETRY_H
#define SMGEOMETRY_H
#include "smCore/smConfig.h"
#include "smUtilities/smVec3.h"

#define SMALL_NUM   0.00000001 

//forward declaration
struct smSphere;

struct smPlane{
	smVec3f unitNormal;
	smVec3f pos;
	inline smFloat distance(smVec3f p_vector);
	inline smVec3f project(smVec3f p_vector);
};

struct smAABB{
	smVec3<float> aabbMin;
	smVec3<float> aabbMax;

	inline smAABB(){
		aabbMin.setValue(0,0,0);
		aabbMax.setValue(0,0,0);

	}
	inline smVec3f center(){
		return smVec3f((aabbMin.x+aabbMax.x)/2.0,
		               (aabbMin.y+aabbMax.y)/2.0,
		               (aabbMin.z+aabbMax.z)/2.0);
	}
	static inline smBool checkOverlap( smAABB &p_aabbA,  smAABB &p_aabbB)
	{
		if( p_aabbA.aabbMin.x > p_aabbB.aabbMax.x || 
			p_aabbA.aabbMax.x < p_aabbB.aabbMin.x ||
			p_aabbA.aabbMin.y > p_aabbB.aabbMax.y ||
			p_aabbA.aabbMax.y < p_aabbB.aabbMin.y ||
			p_aabbA.aabbMin.z > p_aabbB.aabbMax.z ||
			p_aabbA.aabbMax.z < p_aabbB.aabbMin.z )
			return false;
		else
			return true;
	}
	inline smAABB &operator=(smAABB &p_aabb){
		aabbMax=p_aabb.aabbMax;
		aabbMin=p_aabb.aabbMin;
		return *this;
	}
	inline smAABB &operator*(smFloat p_scale){
		aabbMin=aabbMin*p_scale;
		aabbMax=aabbMax*p_scale;
		return *this;
	}

	void subDivide(smFloat  p_length ,smInt p_divison, smAABB *p_aabb);
	void subDivide(smInt p_divisionX,smInt p_divisionY, smInt p_divisionZ ,smAABB *p_aabb);

	inline void subDivide(smInt p_division,smAABB *p_aabb){
		subDivide(p_division,p_division,p_division,p_aabb);
	}
	inline smFloat halfSizeX(){
		return (aabbMax.x-aabbMin.x)/2.0;
	}
	inline smFloat halfSizeY(){
		return (aabbMax.y-aabbMin.y)/2.0;
	}

	inline smFloat halfSizeZ(){
		return (aabbMax.z-aabbMin.z)/2.0;
	}

	inline void expand(smFloat p_factor){
		aabbMin=aabbMin-(aabbMax-aabbMin)*p_factor/2.0;
		aabbMax=aabbMax+(aabbMax-aabbMin)*p_factor/2.0;
	}
};

class smCollisionUtils{
public:
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

	static smBool tri2tri(smVec3<smFloat> &p_tri1Point1,
	                      smVec3<smFloat> &p_tri1Point2,
	                      smVec3<smFloat> &p_tri1Point3,
	                      smVec3<smFloat> &p_tri2Point1,
	                      smVec3<smFloat> &p_tri2Point2,
	                      smVec3<smFloat> &p_tri2Point3);

	static inline bool checkOverlapAABBAABB( smAABB &aabbA,  smAABB &aabbB, smAABB &result)
	{
		if( aabbA.aabbMin.x > aabbB.aabbMax.x || 
			aabbA.aabbMax.x < aabbB.aabbMin.x ||
			aabbA.aabbMin.y > aabbB.aabbMax.y ||
			aabbA.aabbMax.y < aabbB.aabbMin.y ||
			aabbA.aabbMin.z > aabbB.aabbMax.z ||
			aabbA.aabbMax.z < aabbB.aabbMin.z ) {
			return false;
		}
		else{
			result.aabbMin.x=SOFMIS_MAX(aabbA.aabbMin.x,aabbB.aabbMin.x);
			result.aabbMin.y=SOFMIS_MAX(aabbA.aabbMin.y,aabbB.aabbMin.y);
			result.aabbMin.z=SOFMIS_MAX(aabbA.aabbMin.z,aabbB.aabbMin.z);

			result.aabbMax.x=SOFMIS_MIN(aabbA.aabbMax.x,aabbB.aabbMax.x);
			result.aabbMax.y=SOFMIS_MIN(aabbA.aabbMax.y,aabbB.aabbMax.y);
			result.aabbMax.z=SOFMIS_MIN(aabbA.aabbMax.z,aabbB.aabbMax.z);
			return true;
		}
	}

	static inline bool checkOverlapAABBAABB( smAABB &aabbA, smAABB &aabbB)
	{
		if( aabbA.aabbMin.x > aabbB.aabbMax.x ||
			aabbA.aabbMax.x < aabbB.aabbMin.x ||
			aabbA.aabbMin.y > aabbB.aabbMax.y ||
			aabbA.aabbMax.y < aabbB.aabbMin.y ||
			aabbA.aabbMin.z > aabbB.aabbMax.z ||
			aabbA.aabbMax.z < aabbB.aabbMin.z )
			return false;
		else
			return true;
	}

	static inline bool checkOverlapAABBAABB( smAABB &aabbA,smVec3<smFloat> &p_vertex)
	{
		if( aabbA.aabbMin.x <= p_vertex.x &&	aabbA.aabbMax.x >= p_vertex.x &&
			aabbA.aabbMin.y <= p_vertex.y &&	aabbA.aabbMax.y >= p_vertex.y &&
			aabbA.aabbMin.z <= p_vertex.z &&	aabbA.aabbMax.z >= p_vertex.z )
			return true;
		else
			return false;
	}

	static smBool checkLineTri(smVec3<smFloat> &p_linePoint1,
	                           smVec3<smFloat> &p_linePoint2,
	                           smVec3<smFloat> &p_tri1Point1,
	                           smVec3<smFloat> &p_tri1Point2,
	                           smVec3<smFloat> &p_tri1Point3,
	                           smVec3<smFloat> &p_interSection);

	static smBool checkAABBTriangle(smAABB &p_aabb,smVec3f v1,smVec3f v2,smVec3f v3);
	static inline smBool checkAABBPoint(smAABB &p_aabb,smVec3f p_v){
		if (p_v.x >= p_aabb.aabbMin.x &&
			p_v.y >= p_aabb.aabbMin.y &&
			 p_v.z>= p_aabb.aabbMin.z  &&
			p_v.x<=  p_aabb.aabbMax.x  &&
			p_v.y <= p_aabb.aabbMax.y &&
			p_v.z <= p_aabb.aabbMax.z )
			return true;
		else
			return false;
	}
};

struct smSphere{
public:
	smVec3f center;
	smFloat radius;

	inline smSphere(){
		center.setValue(0,0,0);
		radius=1.0;
	}
	inline smSphere(smVec3f p_center,smFloat p_radius){
		radius=p_radius;
		center=p_center;
	}
};

struct smCube{
	smVec3f center;
	smFloat sideLength;

	inline smCube(){
		center.setValue(0,0,0);
		sideLength=1.0;
	}

	inline void subDivide(smInt p_divisionPerAxis, smCube*p_cube){
		smVec3f minPoint;
		smFloat divLength=(sideLength/p_divisionPerAxis);
		smInt index=0;
		minPoint.setValue(center.x-sideLength*0.5,center.y-sideLength*0.5,center.z-sideLength*0.5);

		for(smInt ix=0;ix<p_divisionPerAxis;ix++)
			for(smInt iy=0;iy<p_divisionPerAxis;iy++)
				for(smInt iz=0;iz<p_divisionPerAxis;iz++)
				{
					p_cube[index].center.x=minPoint.x+divLength*ix+divLength*0.5;
					p_cube[index].center.y=minPoint.y+divLength*iy+divLength*0.5;
					p_cube[index].center.z=minPoint.z+divLength*iz+divLength*0.5;
					p_cube[index].sideLength=divLength;

					
					index++;
				}
	}
	inline void expand(smFloat p_expansion){
		sideLength=sideLength+sideLength*p_expansion;
	}
	inline smVec3f leftMinCorner(){
		return smVec3f(center.x-sideLength*0.5,center.y-sideLength*0.5,center.z-sideLength*0.5);
	}
	inline smVec3f rightMaxCorner(){
		return smVec3f(center.x+sideLength*0.5,center.y+sideLength*0.5,center.z+sideLength*0.5);
	}
	inline smSphere getCircumscribedSphere(){
		return smSphere(center,0.866025*sideLength);
	}
	inline smSphere getInscribedSphere(){
		return smSphere(center,sideLength*0.5);
	}
	inline smSphere getTangent2EdgeSphere(){
		return smSphere(center,sideLength*0.707106);
	}
};

int intersect3D_RayTriangleWithBarycentric( smVec3f P0, smVec3f P1, smVec3f V0, smVec3f V1, smVec3f V2, smVec3f *I,float &p_baryU, float &p_baryV, float &p_baryW,bool considerFrontFaceOnly);
#endif
