/*
*******************************************************************
				  SIMMEDTK LICENSE
********************************************************************
*/
#ifndef SMVEC3_H
#define SMVEC3_H
#include "smCore/smConfig.h"
#include "math.h"

template<class T>
class smMatrix44;

///Vector class. It has all required functions for
///vector-vector, vector-scalar operations.
template <typename T>
class smVec3{
public:

	///x component of the vector
	T x;
	///y component of the vector
	T y;
	///z component of the vector
	T z;

	///Default constructor
	inline smVec3(){
		x=0.0;
		y=0.0;
		z=0.0;
	}

	///Member constructor
	inline smVec3(T p_x,T p_y, T p_z){
		x=p_x;
		y=p_y;
		z=p_z;
	}

	/// length of the vector
	inline T module() {
		return (T)sqrt((x*x + y*y + z*z));
	}

	///generate the unit vector
	inline smVec3 unit(){
		smVec3 ret;
		T n=module();
		if(n==0.0f) n=1.0;
		ret.setValue(x/n,y/n,z/n);
		return ret;
	};

	///setter function for member x,y,z
	inline void setValue(T p_x, T p_y, T p_z){
		x = p_x;
		y = p_y;
		z = p_z;
	}

	///normalize the vector
	inline void normalize(){
		smFloat l =  module();
		if (l==0) {
			x = 0.0;
			y = 0.0;
			z = 0.0;
			return;
		}
		x = x / l ;
		y = y / l ;
		z = z / l ;
	}

	///dot product of the two vectors
	inline T dot( smVec3<T> &p_v ) const {
		return ( x * p_v.x + y * p_v.y + z * p_v.z );
	}

	///cross product of two vectors
	inline smVec3 cross(smVec3 p_point) const{
		smVec3 ret;
		ret.x=y*p_point.z-z*p_point.y;
		ret.y=z*p_point.x-x*p_point.z;
		ret.z=x*p_point.y-y*p_point.x;
		return ret;
	}

	///distance between two vectors
	inline T distance(smVec3 &p_point){
		return (*this-p_point).module();
	}

	///vector-scalar multiplication
	inline smVec3 operator*(T p_f) const {
		smVec3 mult;
		mult.x = x * p_f;
		mult.y = y * p_f;
		mult.z = z * p_f;
		return mult;
	}

	inline friend smVec3 operator *( smReal p_real , const smVec3<T> & p_vector){
		return smVec3(p_real * p_vector.x,p_real * p_vector.y,p_real * p_vector.z);
	}

	inline friend smVec3 operator -( smReal p_real , const smVec3<T> & p_vector){
		return smVec3(p_real - p_vector.x,p_real - p_vector.y,p_real - p_vector.z);
	}

	inline  smVec3 operator -( ) const{
		return smVec3( - x, - y, - z);
	}

	inline friend smVec3 operator +( smReal p_real , const smVec3<T> & p_vector){
		return smVec3(p_real + p_vector.x,p_real + p_vector.y,p_real + p_vector.z);
	}

	inline friend smVec3 operator /( smReal p_real , const smVec3<T> & p_vector){
		return smVec3(p_real / p_vector.x,p_real / p_vector.y,p_real / p_vector.z);
	}

	///vector divided by scalar
	inline smVec3 operator/(T p_f) const {
		smVec3 div;
		div.x = x / p_f;
		div.y = y / p_f;
		div.z = z / p_f;
		return div;
	}

	///vector cross product with another vector
	inline smVec3 operator*( const smVec3 &p_v ) const {
		smVec3 vv;
		vv.x = y * p_v.z - (z * p_v.y);
		vv.y = z * p_v.x - (x * p_v.z);
		vv.z = x * p_v.y - (y * p_v.x);
		return ( vv );
	}

	///vector addition with another vector
	inline smVec3 operator+( const smVec3 &p_v ) {
		smVec3 vv;
		vv.x = x + p_v.x;
		vv.y = y + p_v.y;
		vv.z = z + p_v.z;
		return (vv);
	}

	///vector subraction from another vector
	inline smVec3 operator-( const smVec3 &p_v ) {
		smVec3 vv;
		vv.x = x - p_v.x;
		vv.y = y - p_v.y;
		vv.z = z - p_v.z;
		return (vv);
	}

	///add vector and make it equal to itself
	inline smVec3 &operator+=( const smVec3 &p_v ) {
		x += p_v.x;
		y += p_v.y;
		z += p_v.z;
		return (*this);
	}

	///subtract vector make it equal to itself
	inline smVec3 &operator-=( const smVec3 &p_v ) {
		x -= p_v.x;
		y -= p_v.y;
		z -= p_v.z;
		return (*this);
	}

	///make it equal to itself
	template<typename K>
	inline smVec3<T> &operator=( const smVec3<K> &p_v ){
		x = p_v.x;
		y = p_v.y;
		z = p_v.z;
		return  ( *this);
	}

	//returns the angle between vectors
	inline T angle(smVec3 &p_vec){
		smFloat length=p_vec.module()*module();
		if(length!=0)
		acosf(dot(p_vec)/length);
	}

	///allows accesing elements with [] operator
	///ex: vec[0]=12;
	///be aware there is no index checking  due to prevent slowdowns.
	inline volatile T &operator[](int i){
		return (&x)[i];
	}

	inline smBool operator >(smVec3 &p_v){
		smBool biggerThan;
		if(this->module()>p_v.module())
			biggerThan=true;
		else
			biggerThan=false;
		return biggerThan;
	}

	inline smBool operator <(smVec3 &p_v){
		smBool lessThan;
		if(this->module()<p_v.module())
			lessThan=true;
		else
			lessThan=false;

		return lessThan;
	}

	///returns the absolute value and returns the vector
	inline smVec3 absolute(){
		return smVec3(abs(x),abs(y),abs(z));
	}

	template <class P>
	inline smVec3<T> &operator=(smMatrix44<P> &p_m){
		x=p_m.e[0][3];
		y=p_m.e[1][3];
		z=p_m.e[2][3];
		return *this;
	}
};

typedef smVec3<smFloat> smVec3f;
typedef smVec3<smDouble> smVec3d;

#endif
