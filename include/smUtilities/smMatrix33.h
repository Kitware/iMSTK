/*****************************************************
                  SIMMEDTK LICENSE
****************************************************

*****************************************************/

#ifndef SMMATRIX33_H
#define SMMATRIX33_H

#include "smUtilities/smVec3.h"
#include "smMatrix44.h"
#include "smMath.h"
#include "smCore/smCoreClass.h"
#include <iostream>

using namespace std;

/// \brief smMatrix33
///Matrix 3x3 Class which supports several matrix operations.
///Attention that indices of the matrix starts from 0
template <class T>
class smMatrix33{
public:
	/// \brief members of the matrix. they are column major
	T e[3][3];
public:
	/// \brief Default constructor creates identity matrix
	inline smMatrix33(){
		setIdentity();
	}
	/// \brief sets matrix to identity
	inline void setIdentity(){
		e[0][0]=1;e[0][1]=0;e[0][2]=0;
		e[1][0]=0;e[1][1]=1;e[1][2]=0;
		e[2][0]=0;e[2][1]=0;e[2][2]=1;
	}
	/// \brief returns the inverse of the matrix 
	inline smMatrix33 getInverse(){
		smMatrix33 ret;
		smFloat det;

		det=getDeterminant();
		if(fabs(det)>SM_MATRIX_PRECISION)
		{
			ret.e[0][0] =  (e[1][1]*e[2][2] - e[1][2]*e[2][1])/det;
			ret.e[0][1] = -(e[0][1]*e[2][2] - e[0][2]*e[2][1])/det;
			ret.e[0][2] =  (e[0][1]*e[1][2] - e[0][2]*e[1][1])/det;

			ret.e[1][0] = -(e[1][0]*e[2][2] - e[1][2]*e[2][0])/det;
			ret.e[1][1] =  (e[0][0]*e[2][2] - e[0][2]*e[2][0])/det;
			ret.e[1][2] = -(e[0][0]*e[1][2] - e[0][2]*e[1][0])/det;

			ret.e[2][0] =  (e[1][0]*e[2][1] - e[1][1]*e[2][0])/det;
			ret.e[2][1] = -(e[0][0]*e[2][1] - e[0][1]*e[2][0])/det;
			ret.e[2][2] =  (e[0][0]*e[1][1] - e[0][1]*e[1][0])/det;
		}
		else{
			ret.e[0][0] =  1.0; ret.e[0][1] =  0.0; ret.e[0][2] = 0.0;
			ret.e[1][0] =  0.0; ret.e[1][1] =  1.0;	ret.e[1][2] = 0.0;
			ret.e[2][0] =  0.0;	ret.e[2][1] =  0.0;	ret.e[2][2] = 1.0;
		}
		return ret;
	}

	/// \brief returns the transpose of the matrix
	inline smMatrix33 getTranspose(){
		smMatrix33 ret;
		ret.e[0][0] = e[0][0]; ret.e[0][1] = e[1][0]; ret.e[0][2] = e[2][0];
		ret.e[1][0] = e[0][1]; ret.e[1][1] = e[1][1]; ret.e[1][2] = e[2][1];
		ret.e[2][0] = e[0][2]; ret.e[2][1] = e[1][2]; ret.e[2][2] = e[2][2];
		return ret;
	}

	/// \fn  void setMatrixFromOpenGL(smFloat* p_m)
	///  set rotation matrix 
	/// \param p_m is the opengl matrix that is queried with glGet function
	inline void setMatrixFromOpenGL(smFloat* p_m){
		//set 1st column
		e[0][0]=p_m[0];
		e[1][0]=p_m[1];
		e[2][0]=p_m[2];

		//set 2nd column
		e[0][1]=p_m[4];
		e[1][1]=p_m[5];
		e[2][1]=p_m[6];

		//set 3rd column
		e[0][2]=p_m[8];
		e[1][2]=p_m[9];
		e[2][2]=p_m[10];
	}

	/// \brief gets the matrix convenient for opengl matrix array
	///Be aware that this doesn't do the normalization of the columns
	inline void getMatrixForOpenGL(smFloat *p_m){
		p_m[0]=e[0][0];
		p_m[1]=e[1][0];
		p_m[2]=e[2][0];
		p_m[3]=0.0;

		p_m[4]=e[0][1];
		p_m[5]=e[1][1];
		p_m[6]=e[2][1];
		p_m[7]=0.0;

		p_m[8] =e[0][2];
		p_m[9] =e[1][2];
		p_m[10]=e[2][2];
		p_m[11]=0.0;
	}

	/// \brief returns the determinant of the matrix
	inline smFloat getDeterminant(){
		smFloat determinant;
		determinant=e[0][0]*e[1][1]*e[2][2]
			+e[2][0]*e[0][1]*e[1][2]
			+e[1][0]*e[2][1]*e[0][2]
			-e[0][0]*e[2][1]*e[1][2]
			-e[1][0]*e[0][1]*e[2][2]
			-e[2][0]*e[1][1]*e[0][2];
		return determinant;
	}

	/// \brief scales each component of the matrix
	inline void scale(T p_scalar){
		e[0][0] = e[0][0]*p_scalar; e[0][1] = e[0][1]*p_scalar; e[0][2] = e[0][2]*p_scalar;
		e[1][0] = e[1][0]*p_scalar; e[1][1] = e[1][1]*p_scalar; e[1][2] = e[1][2]*p_scalar;
		e[2][0] = e[2][0]*p_scalar; e[2][1] = e[2][1]*p_scalar; e[2][2] = e[2][2]*p_scalar;
	}

	/// \brief operator for adding matrices
	inline smMatrix33 operator+(const smMatrix33 &p_m){
		smMatrix33 ret;
		ret.e[0][0] = e[0][0]+p_m.e[0][0]; ret.e[0][1] = e[0][1]+p_m.e[0][1]; ret.e[0][2] = e[0][2]+p_m.e[0][2];
		ret.e[1][0] = e[1][0]+p_m.e[1][0]; ret.e[1][1] = e[1][1]+p_m.e[1][1]; ret.e[1][2] = e[1][2]+p_m.e[1][2];
		ret.e[2][0] = e[2][0]+p_m.e[2][0]; ret.e[2][1] = e[2][1]+p_m.e[2][1]; ret.e[2][2] = e[2][2]+p_m.e[2][2];
		return ret;
	}

	/// \brief operator for subtracting the matrices
	inline smMatrix33 operator-(const smMatrix33 &p_m){
		smMatrix33 ret;
		ret.e[0][0] = e[0][0]-p_m.e[0][0]; ret.e[0][1] = e[0][1]-p_m.e[0][1]; ret.e[0][2] = e[0][2]-p_m.e[0][2];
		ret.e[1][0] = e[1][0]-p_m.e[1][0]; ret.e[1][1] = e[1][1]-p_m.e[1][1]; ret.e[1][2] = e[1][2]-p_m.e[1][2];
		ret.e[2][0] = e[2][0]-p_m.e[2][0]; ret.e[2][1] = e[2][1]-p_m.e[2][1]; ret.e[2][2] = e[2][2]-p_m.e[2][2];
		return ret;
	}

	/// \brief multiplying the matrices
	inline smMatrix33 operator*(const smMatrix33 &p_m)const{
		smMatrix33 ret;

		ret.e[0][0]=e[0][0]*p_m.e[0][0]+e[0][1]*p_m.e[1][0]+e[0][2]*p_m.e[2][0];
		ret.e[0][1]=e[0][0]*p_m.e[0][1]+e[0][1]*p_m.e[1][1]+e[0][2]*p_m.e[2][1];
		ret.e[0][2]=e[0][0]*p_m.e[0][2]+e[0][1]*p_m.e[1][2]+e[0][2]*p_m.e[2][2];

		ret.e[1][0]=e[1][0]*p_m.e[0][0]+e[1][1]*p_m.e[1][0]+e[1][2]*p_m.e[2][0];
		ret.e[1][1]=e[1][0]*p_m.e[0][1]+e[1][1]*p_m.e[1][1]+e[1][2]*p_m.e[2][1];
		ret.e[1][2]=e[1][0]*p_m.e[0][2]+e[1][1]*p_m.e[1][2]+e[1][2]*p_m.e[2][2];

		ret.e[2][0]=e[2][0]*p_m.e[0][0]+e[2][1]*p_m.e[1][0]+e[2][2]*p_m.e[2][0];
		ret.e[2][1]=e[2][0]*p_m.e[0][1]+e[2][1]*p_m.e[1][1]+e[2][2]*p_m.e[2][1];
		ret.e[2][2]=e[2][0]*p_m.e[0][2]+e[2][1]*p_m.e[1][2]+e[2][2]*p_m.e[2][2];
		return ret;
	}

	/// \brief Matrix assignment  operators
	inline smMatrix33 &operator=(const smMatrix33 &p_m){
		e[0][0]=p_m.e[0][0];e[0][1]=p_m.e[0][1];e[0][2]=p_m.e[0][2];
		e[1][0]=p_m.e[1][0];e[1][1]=p_m.e[1][1];e[1][2]=p_m.e[1][2];
		e[2][0]=p_m.e[2][0];e[2][1]=p_m.e[2][1];e[2][2]=p_m.e[2][2];
		return *this;
	}
	inline smMatrix33 &operator=(const smMatrix44<T> &p_m){
		e[0][0]=p_m.e[0][0];e[0][1]=p_m.e[0][1];e[0][2]=p_m.e[0][2];
		e[1][0]=p_m.e[1][0];e[1][1]=p_m.e[1][1];e[1][2]=p_m.e[1][2];
		e[2][0]=p_m.e[2][0];e[2][1]=p_m.e[2][1];e[2][2]=p_m.e[2][2];
		return *this;
	}


	/// \brief operator for Matrix scalar multiplcation
	inline smMatrix33 operator*(smFloat p_scalar){
		smMatrix33 ret;
		ret.e[0][0]=e[0][0]*p_scalar;ret.e[0][1]=e[0][1]*p_scalar;ret.e[0][2]=e[0][2]*p_scalar;
		ret.e[1][0]=e[1][0]*p_scalar;ret.e[1][1]=e[1][1]*p_scalar;ret.e[1][2]=e[1][2]*p_scalar;
		ret.e[2][0]=e[2][0]*p_scalar;ret.e[2][1]=e[2][1]*p_scalar;ret.e[2][2]=e[2][2]*p_scalar;
		return ret;
	}

	/// \brief operator()() loading. So one can access the elements by paranthesis
	///ex: A(1,2)=12;
	inline T &operator()(int i,int j){
		return e[i][j];
	}

	/// \brief Transforms the smVector with the smMatrix
	template <class L>
	inline smVec3<L> operator*(smVec3<L> p_v){
		smVec3<L> ret;
		ret.x=e[0][0]*p_v.x+e[0][1]*p_v.y+e[0][2]*p_v.z;
		ret.y=e[1][0]*p_v.x+e[1][1]*p_v.y+e[1][2]*p_v.z;
		ret.z=e[2][0]*p_v.x+e[2][1]*p_v.y+e[2][2]*p_v.z;
		return ret;
	}

	/// \brief Set its values from array by colum major
	/// \param p_array assumes that array contains column major values
	/// Such as p_array[0], p_array[1], p_array[2], p_array[3] has (0,0), (1,0), (2,0), (0,1)
	inline void setValuesbyColumnMajor(smFloat *p_array){
		e[0][0]=p_array[0];
		e[1][0]=p_array[1];
		e[2][0]=p_array[2];

		e[0][1]=p_array[3];
		e[1][1]=p_array[4];
		e[2][1]=p_array[5];

		e[0][2]=p_array[6];
		e[1][2]=p_array[7];
		e[2][2]=p_array[8];
	}
	/// \brief Set its values from array by colum major
	/// \param p_array assumes that array contains column major values
	/// Such as matrix (0,0), (1,0), (2,0), (0,1) will be stored in p_array[0], p_array[1], p_array[2], p_array[3]
	inline void getValuesbyColumnMajor(smFloat *p_array){
		p_array[0]= e[0][0];
		p_array[1]= e[1][0];
		p_array[2]= e[2][0];

		p_array[3]=e[0][1];
		p_array[4]=e[1][1];
		p_array[5]=e[2][1];

		p_array[6]=e[0][2];
		p_array[7]=e[1][2];
		p_array[8]=e[2][2];
	}

	/// \brief get columns of the matrix with index starting from 0
	inline smVec3<T> getColumn(smInt p_index){
		smVec3<T> ret;
		ret.x=e[0][p_index];
		ret.y=e[1][p_index];
		ret.z=e[2][p_index];
		return ret;
	}

	/// \brief get rows of the matrix with index starting from 0
	inline smVec3<T> getRow(smInt p_index){
		smVec3<T> ret;
		ret.x=e[p_index][0];
		ret.y=e[p_index][1];
		ret.z=e[p_index][2];
		return ret;
	}

	/// \brief rotate around X axis.
	///\param angle angle in Radians
	inline void rotAroundX(smDouble angle){
		smDouble c;
		smDouble s;
		c=cos(angle);
		s=sin(angle);
		e[0][0]=1;e[0][1]=0;e[0][2]=0;
		e[1][0]=0;e[1][1]=c;e[1][2]=-s;
		e[2][0]=0;e[2][1]=s;e[2][2]=c;
	}

	/// \brief rotate around Y axis.
	///\param angle angle in Radians
	inline void rotAroundY(smDouble angle){
		smDouble c;
		smDouble s;
		c=cos((smDouble)angle);
		s=sin((smDouble)angle);
		e[0][0]=c;  e[0][1]=0;  e[0][2]=s;
		e[1][0]=0;  e[1][1]=1;  e[1][2]=0;
		e[2][0]=-s; e[2][1]=0;  e[2][2]=c;
	}

	/// \brief rotate around Z axis.
	///\param angle angle in Radians
	inline void rotAroundZ(smDouble angle){
		smDouble c;
		smDouble s;
		c=cos(angle);
		s=sin(angle);
		e[0][0]=c; e[0][1]=-s; e[0][2]=0;
		e[1][0]=s; e[1][1]=c;  e[1][2]=0;
		e[2][0]=0; e[2][1]=0;  e[2][2]=1;
	}
	/// \brief putting matrix on console
	friend ostream &operator<<( ostream &out, smMatrix33 &p ) {
		out<<p.e[0][0]<<","<<p.e[0][1]<<","<<p.e[0][2]<<endl;
		out<<p.e[1][0]<<","<<p.e[1][1]<<","<<p.e[1][2]<<endl;
		out<<p.e[2][0]<<","<<p.e[2][1]<<","<<p.e[2][2]<<endl;
		return out;
	}
	/// \brief  assign 4x4 matrix 3x3 matrix
	template <class K>
	inline smMatrix33<T> &operator=(smMatrix44<K> &p_m){
		e[0][0]=p_m.e[0][0];e[0][1]=p_m.e[0][1];e[0][2]=p_m.e[0][2];
		e[1][0]=p_m.e[1][0];e[1][1]=p_m.e[1][1];e[1][2]=p_m.e[1][2];
		e[2][0]=p_m.e[2][0];e[2][1]=p_m.e[2][1];e[2][2]=p_m.e[2][2];

		return *this;
	}

	friend class smMatrix44<T>;
	friend class smVec3<T>;
};

typedef smMatrix33<smFloat> smMatrix33f;
typedef smMatrix33<smDouble> smMatrix33d;

#endif
