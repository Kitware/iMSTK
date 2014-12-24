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

#ifndef SMMATRIX44_H
#define SMMATRIX44_H
#include "smCore/smConfig.h"
#include "smVec3.h"
#include "smVec4.h"
#include "smMath.h"
#include <iostream>

using namespace std;

template<class T>
class smMatrix33;

template<class T>
class smVec3;

/// \brief smMatrix44
///Matrix 4x4 Class which supports several matrix operations.
///Attention that indices of the matrix starts from 0
template <class T>
class smMatrix44
{
public:
    /// \brief members of the matrix.
    T e[4][4];

public:
    /// \brief Default constructore sets identity
    inline smMatrix44()
    {
        setIdentity();
    }

    /// \brief sets the matrix to indetity matrix
    inline void setIdentity()
    {
        e[0][0] = 1;
        e[0][1] = 0;
        e[0][2] = 0;
        e[0][3] = 0;
        e[1][0] = 0;
        e[1][1] = 1;
        e[1][2] = 0;
        e[1][3] = 0;
        e[2][0] = 0;
        e[2][1] = 0;
        e[2][2] = 1;
        e[2][3] = 0;
        e[3][0] = 0;
        e[3][1] = 0;
        e[3][2] = 0;
        e[3][3] = 1;
    }

    /// \brief returns the transpose of the matrix
    inline smMatrix44 getTranspose()
    {
        smMatrix44 ret;
        ret.e[0][0] = e[0][0];
        ret.e[0][1] = e[1][0];
        ret.e[0][2] = e[2][0];
        ret.e[0][3] = e[3][0];
        ret.e[1][0] = e[0][1];
        ret.e[1][1] = e[1][1];
        ret.e[1][2] = e[2][1];
        ret.e[1][3] = e[3][1];
        ret.e[2][0] = e[0][2];
        ret.e[2][1] = e[1][2];
        ret.e[2][2] = e[2][2];
        ret.e[2][3] = e[3][2];
        ret.e[3][0] = e[0][3];
        ret.e[3][1] = e[1][3];
        ret.e[3][2] = e[2][3];
        ret.e[3][3] = e[3][3];
        return ret;
    }

    ///  \fn  void setMatrixFromOpenGL(smFloat* p_m)
    ///  set rotation matrix
    /// \param p_m is the opengl matrix that is queried with glGet function
    inline void setMatrixFromOpenGL(T* p_m)
    {
        //set 1st column
        e[0][0] = p_m[0];
        e[1][0] = p_m[1];
        e[2][0] = p_m[2];
        e[3][0] = p_m[3];

        //set 2nd column
        e[0][1] = p_m[4];
        e[1][1] = p_m[5];
        e[2][1] = p_m[6];
        e[3][1] = p_m[7];

        //set 3rd column
        e[0][2] = p_m[8];
        e[1][2] = p_m[9];
        e[2][2] = p_m[10];
        e[3][2] = p_m[11];

        //set 4rd column
        e[0][3] = p_m[12];
        e[1][3] = p_m[13];
        e[2][3] = p_m[14];
        e[3][3] = p_m[15];
    }

    /// \fn void getMatrixForOpenGL(smFloat *p_m)
    /// \param p_m is matrix stored in 1D array in the way openGL returns
    ///gets the matrix convenient for opengl matrix array
    ///Be aware that this doesn't do the normalization of the columns
    template<typename K>
    inline void getMatrixForOpenGL(K *p_m)
    {
        //1st column is stored
        p_m[0] = e[0][0];
        p_m[1] = e[1][0];
        p_m[2] = e[2][0];
        p_m[3] = e[3][0];

        //2nd column is stored
        p_m[4] = e[0][1];
        p_m[5] = e[1][1];
        p_m[6] = e[2][1];
        p_m[7] = e[3][1];

        //3rd column is stored
        p_m[8] = e[0][2];
        p_m[9] = e[1][2];
        p_m[10] = e[2][2];
        p_m[11] = e[3][2];

        //4rd column is stored
        p_m[12] = e[0][3];
        p_m[13] = e[1][3];
        p_m[14] = e[2][3];
        p_m[15] = e[3][3];
    }

    /// \brief operator()() loading. So one can access the elements by paranthesis
    ///ex: A(1,2)=12;
    inline T &operator()(int i, int j)
    {
        return e[i][j];
    }

    /// \brief Set its values from array by colum major
    /// \param p_array assumes that array contains column major values
    /// Such as p_array[0], p_array[1], p_array[2], p_array[3] has (0,0), (1,0), (2,0), (0,1)
    inline void setValuesbyColumnMajor(smFloat *p_array)
    {
        e[0][0] = p_array[0];
        e[1][0] = p_array[1];
        e[2][0] = p_array[2];
        e[3][0] = p_array[3];

        e[0][1] = p_array[4];
        e[1][1] = p_array[5];
        e[2][1] = p_array[6];
        e[3][1] = p_array[7];

        e[0][2] = p_array[8];
        e[1][2] = p_array[9];
        e[2][2] = p_array[10];
        e[3][2] = p_array[11];

        e[0][3] = p_array[12];
        e[1][3] = p_array[13];
        e[2][3] = p_array[14];
        e[3][3] = p_array[15];
    }

    /// \brief Set its values from array by colum major
    /// \param p_array assumes that array contains column major values
    /// Such as matrix (0,0), (1,0), (2,0), (0,1) will be stored in p_array[0], p_array[1], p_array[2], p_array[3]
    inline void getValuesbyColumnMajor(smFloat *p_array)
    {
        p_array[0] = e[0][0];
        p_array[1] = e[1][0];
        p_array[2] = e[2][0];
        p_array[3] = e[3][0];

        p_array[4] = e[0][1];
        p_array[5] = e[1][1];
        p_array[6] = e[2][1];
        p_array[7] = e[3][1];

        p_array[8] = e[0][2];
        p_array[9] = e[1][2];
        p_array[10] = e[2][2];
        p_array[11] = e[3][2];

        p_array[12] = e[0][3];
        p_array[13] = e[1][3];
        p_array[14] = e[2][3];
        p_array[15] = e[3][3];
    }

    /// \brief get columns of the matrix with index starting from 0
    inline smVec3<T> getColumn(smInt p_index)
    {
        smVec3<T> ret;
        ret.x = e[0][p_index];
        ret.y = e[1][p_index];
        ret.z = e[2][p_index];
        return ret;
    }

    /// \brief get rows of the matrix with index starting from 0
    inline smVec3<T> getRow(smInt p_index)
    {
        smVec3<T> ret;
        ret.x = e[p_index][0];
        ret.y = e[p_index][1];
        ret.z = e[p_index][2];
        return ret;
    }

    /// \brief set row of the rotationa matrix with index starting from 0
    inline void setColumn(smVec3<T> &p_vec, smInt p_columnIndex)
    {
        e[0][p_columnIndex] = p_vec.x;
        e[1][p_columnIndex] = p_vec.y;
        e[2][p_columnIndex] = p_vec.z;
    }

    /// \brief operator for adding matrices
    inline smMatrix44 operator+(smMatrix44 &p_m)
    {
        smMatrix44 ret;
        ret.e[0][0] = e[0][0] + p_m.e[0][0];
        ret.e[0][1] = e[0][1] + p_m.e[0][1];
        ret.e[0][2] = e[0][2] + p_m.e[0][2];
        ret.e[0][3] = e[0][3] + p_m.e[0][3];
        ret.e[1][0] = e[1][0] + p_m.e[1][0];
        ret.e[1][1] = e[1][1] + p_m.e[1][1];
        ret.e[1][2] = e[1][2] + p_m.e[1][2];
        ret.e[1][3] = e[1][3] + p_m.e[1][3];
        ret.e[2][0] = e[2][0] + p_m.e[2][0];
        ret.e[2][1] = e[2][1] + p_m.e[2][1];
        ret.e[2][2] = e[2][2] + p_m.e[2][2];
        ret.e[2][3] = e[2][3] + p_m.e[2][3];
        ret.e[3][0] = e[3][0] + p_m.e[3][0];
        ret.e[3][1] = e[3][1] + p_m.e[3][1];
        ret.e[3][2] = e[3][2] + p_m.e[3][2];
        ret.e[3][3] = e[3][3] + p_m.e[3][3];

        return ret;
    }

    /// \brief operator for subtracting the matrices
    inline smMatrix44 operator-(smMatrix44 &p_m)
    {
        smMatrix44 ret;
        ret.e[0][0] = e[0][0] - p_m.e[0][0];
        ret.e[0][1] = e[0][1] - p_m.e[0][1];
        ret.e[0][2] = e[0][2] - p_m.e[0][2];
        ret.e[0][3] = e[0][3] - p_m.e[0][3];
        ret.e[1][0] = e[1][0] - p_m.e[1][0];
        ret.e[1][1] = e[1][1] - p_m.e[1][1];
        ret.e[1][2] = e[1][2] - p_m.e[1][2];
        ret.e[1][3] = e[1][3] - p_m.e[1][3];
        ret.e[2][0] = e[2][0] - p_m.e[2][0];
        ret.e[2][1] = e[2][1] - p_m.e[2][1];
        ret.e[2][2] = e[2][2] - p_m.e[2][2];
        ret.e[2][3] = e[2][3] - p_m.e[2][3];
        ret.e[3][0] = e[3][0] - p_m.e[3][0];
        ret.e[3][1] = e[3][1] - p_m.e[3][1];
        ret.e[3][2] = e[3][2] - p_m.e[3][2];
        ret.e[3][3] = e[3][3] - p_m.e[3][3];
        return ret;
    }


    /// \brief returns the determinant of the matrix
    inline T getDeterminant()
    {
        return (e[0][0] * e[1][1] - e[1][0] * e[0][1]) * (e[2][2] * e[3][3] - e[3][2] * e[2][3])
               - (e[0][0] * e[2][1] - e[2][0] * e[0][1]) * (e[1][2] * e[3][3] - e[3][2] * e[1][3])
               + (e[0][0] * e[3][1] - e[3][0] * e[0][1]) * (e[1][2] * e[2][3] - e[2][2] * e[1][3])
               + (e[1][0] * e[2][1] - e[2][0] * e[1][1]) * (e[0][2] * e[3][3] - e[3][2] * e[0][3])
               - (e[1][0] * e[3][1] - e[3][0] * e[1][1]) * (e[0][2] * e[2][3] - e[2][2] * e[0][3])
               + (e[2][0] * e[3][1] - e[3][0] * e[2][1]) * (e[0][2] * e[1][3] - e[1][2] * e[0][3]);
    }

    /// \brief calculates the invers of the 4x4 by matrix
    ///if the matrix becomes singular in SM_MATRIX_PRECISION
    ///then it return identity matrix
    inline smMatrix44 getInverse()
    {
        smMatrix44 ret;
        smFloat oneOverDeterminant;
        oneOverDeterminant = 1 / getDeterminant();

        if (fabs(oneOverDeterminant) > SM_MATRIX_PRECISION)
        {
            ret.e[0][0] = oneOverDeterminant * (e[1][1] * (e[2][ 2] * e[3][3] - e[3][2] * e[2][3]) + e[2][1] * (e[3][2] * e[1][3] - e[1][2] * e[3][3]) + e[3][1] * (e[1][2] * e[2][3] - e[2][2] * e[1][3]));
            ret.e[1][0] = oneOverDeterminant * (e[1][2] * (e[2][ 0] * e[3][3] - e[3][0] * e[2][3]) + e[2][2] * (e[3][0] * e[1][3] - e[1][0] * e[3][3]) + e[3][2] * (e[1][0] * e[2][3] - e[2][0] * e[1][3]));
            ret.e[2][0] = oneOverDeterminant * (e[1][3] * (e[2][ 0] * e[3][1] - e[3][0] * e[2][1]) + e[2][3] * (e[3][0] * e[1][1] - e[1][0] * e[3][1]) + e[3][3] * (e[1][0] * e[2][1] - e[2][0] * e[1][1]));
            ret.e[3][0] = oneOverDeterminant * (e[1][0] * (e[3][ 1] * e[2][2] - e[2][1] * e[3][2]) + e[2][0] * (e[1][1] * e[3][2] - e[3][1] * e[1][2]) + e[3][0] * (e[2][1] * e[1][2] - e[1][1] * e[2][2]));
            ret.e[0][1] = oneOverDeterminant * (e[2][1] * (e[0][ 2] * e[3][3] - e[3][2] * e[0][3]) + e[3][1] * (e[2][2] * e[0][3] - e[0][2] * e[2][3]) + e[0][1] * (e[3][2] * e[2][3] - e[2][2] * e[3][3]));
            ret.e[1][1] = oneOverDeterminant * (e[2][2] * (e[0][ 0] * e[3][3] - e[3][0] * e[0][3]) + e[3][2] * (e[2][0] * e[0][3] - e[0][0] * e[2][3]) + e[0][2] * (e[3][0] * e[2][3] - e[2][0] * e[3][3]));
            ret.e[2][1] = oneOverDeterminant * (e[2][3] * (e[0][ 0] * e[3][1] - e[3][0] * e[0][1]) + e[3][3] * (e[2][0] * e[0][1] - e[0][0] * e[2][1]) + e[0][3] * (e[3][0] * e[2][1] - e[2][0] * e[3][1]));
            ret.e[3][1] = oneOverDeterminant * (e[2][0] * (e[3][ 1] * e[0][2] - e[0][1] * e[3][2]) + e[3][0] * (e[0][1] * e[2][2] - e[2][1] * e[0][2]) + e[0][0] * (e[2][1] * e[3][2] - e[3][1] * e[2][2]));
            ret.e[0][2] = oneOverDeterminant * (e[3][1] * (e[0][ 2] * e[1][3] - e[1][2] * e[0][3]) + e[0][1] * (e[1][2] * e[3][3] - e[3][2] * e[1][3]) + e[1][1] * (e[3][2] * e[0][3] - e[0][2] * e[3][3]));
            ret.e[1][2] = oneOverDeterminant * (e[3][2] * (e[0][ 0] * e[1][3] - e[1][0] * e[0][3]) + e[0][2] * (e[1][0] * e[3][3] - e[3][0] * e[1][3]) + e[1][2] * (e[3][0] * e[0][3] - e[0][0] * e[3][3]));
            ret.e[2][2] = oneOverDeterminant * (e[3][3] * (e[0][ 0] * e[1][1] - e[1][0] * e[0][1]) + e[0][3] * (e[1][0] * e[3][1] - e[3][0] * e[1][1]) + e[1][3] * (e[3][0] * e[0][1] - e[0][0] * e[3][1]));
            ret.e[3][2] = oneOverDeterminant * (e[3][0] * (e[1][ 1] * e[0][2] - e[0][1] * e[1][2]) + e[0][0] * (e[3][1] * e[1][2] - e[1][1] * e[3][2]) + e[1][0] * (e[0][1] * e[3][2] - e[3][1] * e[0][2]));
            ret.e[0][3] = oneOverDeterminant * (e[0][1] * (e[2][ 2] * e[1][3] - e[1][2] * e[2][3]) + e[1][1] * (e[0][2] * e[2][3] - e[2][2] * e[0][3]) + e[2][1] * (e[1][2] * e[0][3] - e[0][2] * e[1][3]));
            ret.e[1][3] = oneOverDeterminant * (e[0][2] * (e[2][ 0] * e[1][3] - e[1][0] * e[2][3]) + e[1][2] * (e[0][0] * e[2][3] - e[2][0] * e[0][3]) + e[2][2] * (e[1][0] * e[0][3] - e[0][0] * e[1][3]));
            ret.e[2][3] = oneOverDeterminant * (e[0][3] * (e[2][ 0] * e[1][1] - e[1][0] * e[2][1]) + e[1][3] * (e[0][0] * e[2][1] - e[2][0] * e[0][1]) + e[2][3] * (e[1][0] * e[0][1] - e[0][0] * e[1][1]));
            ret.e[3][3] = oneOverDeterminant * (e[0][0] * (e[1][ 1] * e[2][2] - e[2][1] * e[1][2]) + e[1][0] * (e[2][1] * e[0][2] - e[0][1] * e[2][2]) + e[2][0] * (e[0][1] * e[1][2] - e[1][1] * e[0][2]));
        }
        else
        {
            ret.e[0][0] = 1;
            ret.e[0][0] = 0;
            ret.e[0][0] = 0;
            ret.e[0][0] = 0;
            ret.e[0][0] = 0;
            ret.e[0][0] = 0;
            ret.e[0][0] = 0;
            ret.e[0][0] = 0;
            ret.e[0][0] = 0;
            ret.e[0][0] = 1;
            ret.e[0][0] = 1;
            ret.e[0][0] = 0;
            ret.e[0][0] = 0;
            ret.e[0][0] = 0;
            ret.e[0][0] = 0;
            ret.e[0][0] = 1;
        }

        return ret;
    }

    /// \brief Matrix equality operator
    template<class K>
    inline smMatrix44 &operator=(smMatrix44<K> &p_m)
    {
        e[0][0] = p_m.e[0][0];
        e[0][1] = p_m.e[0][1];
        e[0][2] = p_m.e[0][2];
        e[0][3] = p_m.e[0][3];
        e[1][0] = p_m.e[1][0];
        e[1][1] = p_m.e[1][1];
        e[1][2] = p_m.e[1][2];
        e[1][3] = p_m.e[1][3];
        e[2][0] = p_m.e[2][0];
        e[2][1] = p_m.e[2][1];
        e[2][2] = p_m.e[2][2];
        e[2][3] = p_m.e[2][3];
        e[3][0] = p_m.e[3][0];
        e[3][1] = p_m.e[3][1];
        e[3][2] = p_m.e[3][2];
        e[3][3] = p_m.e[3][3];
        return *this;
    }

    /// \brief multiplying the matrices
    template<class P>
    inline smMatrix44 operator*(smMatrix44<P> &p_m) const
    {
        smMatrix44 ret;

        ret.e[0][0] = e[0][0] * p_m.e[0][0] + e[0][1] * p_m.e[1][0] + e[0][2] * p_m.e[2][0] + e[0][3] * p_m.e[3][0];
        ret.e[0][1] = e[0][0] * p_m.e[0][1] + e[0][1] * p_m.e[1][1] + e[0][2] * p_m.e[2][1] + e[0][3] * p_m.e[3][1];
        ret.e[0][2] = e[0][0] * p_m.e[0][2] + e[0][1] * p_m.e[1][2] + e[0][2] * p_m.e[2][2] + e[0][3] * p_m.e[3][2];
        ret.e[0][3] = e[0][0] * p_m.e[0][3] + e[0][1] * p_m.e[1][3] + e[0][2] * p_m.e[2][3] + e[0][3] * p_m.e[3][3];

        ret.e[1][0] = e[1][0] * p_m.e[0][0] + e[1][1] * p_m.e[1][0] + e[1][2] * p_m.e[2][0] + e[1][3] * p_m.e[3][0];
        ret.e[1][1] = e[1][0] * p_m.e[0][1] + e[1][1] * p_m.e[1][1] + e[1][2] * p_m.e[2][1] + e[1][3] * p_m.e[3][1];
        ret.e[1][2] = e[1][0] * p_m.e[0][2] + e[1][1] * p_m.e[1][2] + e[1][2] * p_m.e[2][2] + e[1][3] * p_m.e[3][2];
        ret.e[1][3] = e[1][0] * p_m.e[0][3] + e[1][1] * p_m.e[1][3] + e[1][2] * p_m.e[2][3] + e[1][3] * p_m.e[3][3];

        ret.e[2][0] = e[2][0] * p_m.e[0][0] + e[2][1] * p_m.e[1][0] + e[2][2] * p_m.e[2][0] + e[2][3] * p_m.e[3][0];
        ret.e[2][1] = e[2][0] * p_m.e[0][1] + e[2][1] * p_m.e[1][1] + e[2][2] * p_m.e[2][1] + e[2][3] * p_m.e[3][1];
        ret.e[2][2] = e[2][0] * p_m.e[0][2] + e[2][1] * p_m.e[1][2] + e[2][2] * p_m.e[2][2] + e[2][3] * p_m.e[3][2];
        ret.e[2][3] = e[2][0] * p_m.e[0][3] + e[2][1] * p_m.e[1][3] + e[2][2] * p_m.e[2][3] + e[2][3] * p_m.e[3][3];

        ret.e[3][0] = e[3][0] * p_m.e[0][0] + e[3][1] * p_m.e[1][0] + e[3][2] * p_m.e[2][0] + e[3][3] * p_m.e[3][0];
        ret.e[3][1] = e[3][0] * p_m.e[0][1] + e[3][1] * p_m.e[1][1] + e[3][2] * p_m.e[2][1] + e[3][3] * p_m.e[3][1];
        ret.e[3][2] = e[3][0] * p_m.e[0][2] + e[3][1] * p_m.e[1][2] + e[3][2] * p_m.e[2][2] + e[3][3] * p_m.e[3][2];
        ret.e[3][3] = e[3][0] * p_m.e[0][3] + e[3][1] * p_m.e[1][3] + e[3][2] * p_m.e[2][3] + e[3][3] * p_m.e[3][3];
        return ret;
    }

    /// \brief operator for Matrix scalar multiplcation
    inline smMatrix44 operator*(smFloat p_scalar)
    {
        smMatrix44 ret;
        ret.e[0][0] = e[0][0] * p_scalar;
        ret.e[0][1] = e[0][1] * p_scalar;
        ret.e[0][2] = e[0][2] * p_scalar;
        ret.e[0][3] = e[0][3] * p_scalar;
        ret.e[1][0] = e[1][0] * p_scalar;
        ret.e[1][1] = e[1][1] * p_scalar;
        ret.e[1][2] = e[1][2] * p_scalar;
        ret.e[1][3] = e[1][3] * p_scalar;
        ret.e[2][0] = e[2][0] * p_scalar;
        ret.e[2][1] = e[2][1] * p_scalar;
        ret.e[2][2] = e[2][2] * p_scalar;
        ret.e[2][3] = e[2][3] * p_scalar;
        ret.e[3][0] = e[3][0] * p_scalar;
        ret.e[3][1] = e[3][1] * p_scalar;
        ret.e[3][2] = e[3][2] * p_scalar;
        ret.e[3][3] = e[3][3] * p_scalar;
        return ret;
    }

    inline friend ostream &operator<<(ostream &out, smMatrix44<T> &p)
    {
        out << p.e[0][0] << "," << p.e[0][1] << "," << p.e[0][2] << "," << p.e[0][3] << endl;
        out << p.e[1][0] << "," << p.e[1][1] << "," << p.e[1][2] << "," << p.e[1][3] << endl;
        out << p.e[2][0] << "," << p.e[2][1] << "," << p.e[2][2] << "," << p.e[2][3] << endl;
        out << p.e[3][0] << "," << p.e[3][1] << "," << p.e[3][2] << "," << p.e[3][3] << endl;
        return out;
    }

    template <class K>
    inline smVec3<K> operator*(smVec3<K> p_v)
    {
        smVec3<K> ret;
        ret.x = e[0][0] * p_v.x + e[0][1] * p_v.y + e[0][2] * p_v.z + e[0][3];
        ret.y = e[1][0] * p_v.x + e[1][1] * p_v.y + e[1][2] * p_v.z + e[1][3];
        ret.z = e[2][0] * p_v.x + e[2][1] * p_v.y + e[2][2] * p_v.z + e[2][3];
        return ret;
    }

    template <class K>
    inline smVec4<K> operator*(smVec4<K> p_v)
    {
        smVec4<K> ret;
        ret.x = e[0][0] * p_v.x + e[0][1] * p_v.y + e[0][2] * p_v.z + e[0][3] * p_v.w;
        ret.y = e[1][0] * p_v.x + e[1][1] * p_v.y + e[1][2] * p_v.z + e[1][3] * p_v.w;
        ret.z = e[2][0] * p_v.x + e[2][1] * p_v.y + e[2][2] * p_v.z + e[2][3] * p_v.w;
        ret.w = e[3][0] * p_v.x + e[3][1] * p_v.y + e[3][2] * p_v.z + e[3][3] * p_v.w;
        return ret;
    }

    template <class K>
    inline void setTranslation(K p_x, K p_y, K p_z)
    {
        e[0][3] = p_x;
        e[1][3] = p_y;
        e[2][3] = p_z;
    }

    template <class K>
    inline smMatrix44 operator*(smMatrix33<K> &p_m) const
    {
        smMatrix44 temp;
        temp.e[0][0] = p_m.e[0][0];
        temp.e[0][1] = p_m.e[0][1];
        temp.e[0][2] = p_m.e[0][2];
        temp.e[1][0] = p_m.e[1][0];
        temp.e[1][1] = p_m.e[1][1];
        temp.e[1][2] = p_m.e[1][2];
        temp.e[2][0] = p_m.e[2][0];
        temp.e[2][1] = p_m.e[2][1];
        temp.e[2][2] = p_m.e[2][2];

        return (*this) * temp;
    }
    template <class P>
    inline smMatrix33<P> operator=(smMatrix44<T> &p_m) const
    {
        smMatrix33<P> temp;
        temp.e[0][0] = p_m.e[0][0];
        temp.e[0][1] = p_m.e[0][1];
        temp.e[0][2] = p_m.e[0][2];
        temp.e[1][0] = p_m.e[1][0];
        temp.e[1][1] = p_m.e[1][1];
        temp.e[1][2] = p_m.e[1][2];
        temp.e[2][0] = p_m.e[2][0];
        temp.e[2][1] = p_m.e[2][1];
        temp.e[2][2] = p_m.e[2][2];

        return temp;
    }

    friend class smMatrix33<T>;
    friend class smVec3<T>;
};

typedef smMatrix44<smFloat> smMatrix44f;
typedef smMatrix44<smDouble> smMatrix44d;

#endif
