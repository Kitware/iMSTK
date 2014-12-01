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
	\brief	    This Module is generic Math class. It provides general Math operations.

*****************************************************
*/

#ifndef SMMATH_H 
#define SMMATH_H 
#include "smCore/smConfig.h"
#include "smCore/smCoreClass.h"

#define SM_PI           3.141592653589793
#define SM_PI_HALF      1.57079632679489661923
#define SM_PI_QUARTER   0.78539816339744830961
#define SM_PI_TWO       6.28318530717958647692
#define SM_PI_INV       0.31830988618379067154

#define SM_MATRIX_PRECISION    0.000000001
#define SM_DEGREES2RADIANS(X)	X*0.0174532925
#define SM_RADIANS2DEGREES(X)	X*57.2957795

///generic math class
class smMath:smCoreClass{
	public:
		smMath(){
			type=SOFMIS_SMMATH;
		}
	static inline smInt pow(smInt p_base, smInt p_pow) {
		smInt res = 1;
		for(smInt i=0; i<p_pow; i++)
			res *= p_base;

		return res;
	}

	static inline smFloat interpolate(smInt current, smInt min, smInt max){
		if(current < min){
			return 0.0;
		}else if (current > max){
			return 1.0;
		}else{
			return (smFloat) (current-min)/ (smFloat) (max-min);
		}
	}
};

template<class T>
class smBaseHash:public smCoreClass{
public:
	smBaseHash(){
	}

	template<T>
	inline smUInt computeHash(smUInt p_tableSize,T p_x);

	template<T>
	inline smUInt computeHash(smUInt p_tableSize,T p_x,T p_y);

	template<T>
	inline smUInt computeHash(smUInt p_tableSize,T p_x,T p_y,T p_z);
};

template<class T>
class smCollisionHash:public smBaseHash<T>{

};

#endif
