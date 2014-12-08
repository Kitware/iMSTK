#ifndef SMQUAT_H
#define SMQUAT_H

#include "smUtilities/smVec3.h"
#include "smUtilities/smMatrix33.h"

/// \brief  quaternion class
template<typename T>
class smQuat {
public:
	/// \brief w of quaternion
	T w;
	/// \brief  vector; z,y,z->i,j,k
	smVec3<T> v;

	/// \brief init default quanternion
	inline smQuat() {
		v.x = v.y = v.z = 0.0; w = 1.0;
	}
	/// \brief use w and v vector to init
	inline smQuat(T p_w, smVec3<T> p_v) {
		w=p_w;
		v=p_v;
	}
	/// \brief set value for w, i,j,k 
	inline void setValue(T p_w, T p_x, T p_y, T p_z){
		w=p_w;
		v.x=p_x;
		v.y=p_y;
		v.z=p_z;
	}
	/// \brief  dot product of quaternions
	inline T	dot(smQuat p) const{
		T dot;
		dot = w*p.w +p.v.dot(v);
		return dot;
	}
	/// \brief returns unit quaternion
	inline smQuat unit() {
		smQuat a;
		T mag;
		mag=sqrt(w*w+v.dot(v));
		a.w=w/mag;
		a.x=v.x/mag;
		a.y=v.y/mag;
		a.z=v.z/mag;
		return a;
	}
	/// \brief log of the quaternion. returns the vector part
	inline smVec3<T> log() {
		// If q = cos(A)+sin(A)*(x*i+y*j+z*k) where (x,y,z) is unit length, then
		// log(q) = A*(x*i+y*j+z*k).  If sin(A) is near zero, use log(q) =
		// sin(A)*(x*i+y*j+z*k) since sin(A)/A has limit 1.
		smVec3<T> ret;
		if(fabs(w)<1.0){
			T fAngle = acos(w);
			T fSin = sin(fAngle);
			if(fabs(fSin) >= 0.00001){
				smFloat fCoeff = fAngle/fSin;
				ret.x = fCoeff * v.x;
				ret.y = fCoeff * v.y;
				ret.z = fCoeff * v.z;
				return ret;
			}
		}
		ret.x = v.x;
		ret.y = v.y;
		ret.z = v.z;

		return ret;
	}
	/// \brief  exponential computation (unit quaternion). returns a quaternion 
	inline smQuat exp() const{
		T a = v.module();
		T sina = sin(a);
		T cosa = cos(a);
		smQuat ret;

		ret.w = cosa;
		if (a > 0)
		{
			ret.v.x = sina * v.x / a;
			ret.v.y = sina * v.y / a;
			ret.v.z = sina * v.z / a;
		}
		else {
			ret.v.x = ret.v.y = ret.v.z = 0;
		}
		return ret;
	}
	/// \brief inverse operation
	inline smQuat inv() {
		smQuat ret;
		ret.w = w;
		ret.v.x = -v.x;
		ret.v.y = -v.y;
		ret.v.z = -v.z;
		return ret;
	}
	/// \brief multiplication operation with quaternion
	inline smQuat operator*(smQuat q) const {
		smQuat ret;
		ret.w = w*q.w-v.x*q.v.x-v.y*q.v.y-v.z*q.v.z;
		ret.v.x = w*q.v.x+v.x*q.w+v.y*q.v.z-v.z*q.v.y;
		ret.v.y = w*q.v.y+v.y*q.w+v.z*q.v.x-v.x*q.v.z;
		ret.v.z = w*q.v.z+v.z*q.w+v.x*q.v.y-v.y*q.v.x;
		return ret;
	}
	/// \brief multiplication with scalar 
	inline smQuat operator*(T f) const {
		smQuat ret;
		ret.w = w*f;
		ret.v.x = v.x*f;
		ret.v.y = v.y*f;
		ret.v.z = v.z*f;
		return ret;
	}
	/// \brief addition operation with quaternion
	inline smQuat operator+(smQuat p_q) const {
		smQuat ret;
		ret.w = w+p_q.w;
		ret.v = ret.v+p_q.v;
		return ret;
	}
	/// \brief set quaternion
	inline smQuat &operator=( const smQuat p_q ) {
		w=p_q.w;
		v=p_q.v;
		return (*this);
	}
	/// \brief generates opengl matrix using quaternion
	void getMatrixForOpenGL(T *p_m){
		p_m[0] = 1.0f - 2.0f * ( v.y * v.y + v.z * v.z );
		p_m[1] = 2.0f * (v.x * v.y + v.z * w);
		p_m[2] = 2.0f * (v.x * v.z - v.y * w);
		p_m[3] = 0.0f;
		p_m[4] = 2.0f * ( v.x * v.y - v.z * w );
		p_m[5] = 1.0f - 2.0f * ( v.x * v.x + v.z * v.z );
		p_m[6] = 2.0f * (v.z * v.y + v.x * w );
		p_m[7] = 0.0f;
		p_m[8] = 2.0f * ( v.x * v.z + v.y * w );
		p_m[9] = 2.0f * ( v.y * v.z - v.x * w );
		p_m[10] = 1.0f - 2.0f * ( v.x * v.x + v.y * v.y );
		p_m[11] = 0.0f;
		p_m[12] = 0;
		p_m[13] = 0;
		p_m[14] = 0;
		p_m[15] = 1.0f;
	};
	/// \brief creates a quaternion from axis-angle pair
	template<typename K>
	inline void fromAxisAngle(const smVec3<K> &p_axis, smFloat p_radian){
		w= cos(p_radian/2.0);
		v=p_axis*sin(p_radian/2.0);
	}

	template<typename K>
	inline void toAxisAngle(smVec3<K> &axis, T &angle) const{
		angle = acos(w)*2;
		axis = v* 1.0/sin(angle);
	}
	/// \brief linear interpolation with using quaternion1 and quaternion 2. t is the interpolation value 
	static inline smQuat lerp(const smQuat &q1, const smQuat &q2, smFloat t){
		return (q1*(1-t) + q2*t).unit();
	}
	/// \brief spherical interpolation with using quaternion1 and quaternion 2. t is the interpolation value 
	static inline smQuat slerp(const smQuat &q1, const smQuat &q2, smFloat t){
		smQuat q3;
		smFloat dot = q1.dot(q2);
		/*	dot = cos(theta)
			if (dot < 0), q1 and q2 are more than 90 degrees apart,
			so we can invert one to reduce spinning	*/
		if (dot < 0){
			dot = -dot;
			q3 = -q2;
		} else q3 = q2;

		if (dot < 0.95f){
			smFloat angle = acosf(dot);
			return (q1*sinf(angle*(1-t)) + q3*sinf(angle*t))/sinf(angle);
		}
		else // if the angle is small, use linear interpolation
			return lerp(q1,q3,t);
	}
	/// \brief retruns conjugate quaternion
	inline smQuat getConjugate(){
		smQuat ret(w,-v);
		return ret;
	}
	/// \brief takes conjugate
	inline void conjugate(){
		v=-v;
	}
	/// \brief rotate the vector using quaternion
	template <typename K>
	inline smVec3<K> rotate(const smVec3<K> &v){
		smQuat V(0.0, v);
		return (*this * V * getConjugate()).v;
	}
};

typedef smQuat<smFloat> smQuatf;
typedef smQuat<smDouble> smQuatd;

#endif
