/*
** This file is part of the Interactive Medical Simulation Toolkit (iMSTK)
** iMSTK is distributed under the Apache License, Version 2.0.
** See accompanying NOTICE for details. 
*/

%include <cpointer.i>
%include <arrays_csharp.i>

#ifdef SWIG_PINNED_ARRAY
    %csmethodmodifiers imstk::VecDataArray::setValues "public unsafe";
    %csmethodmodifiers imstk::VecDataArray::getValues "public unsafe";
    %csmethodmodifiers imstk::DataArray::setValues "public unsafe";
    %csmethodmodifiers imstk::DataArray::getValues "public unsafe";
#endif

#ifdef SWIG_PINNED_ARRAY
    %apply unsigned char FIXED[] {unsigned char* val}
    %apply unsigned char FIXED[] {const unsigned char* val}
    %apply int FIXED[] {int* val}
    %apply int FIXED[] {const int* val}
    %apply float FIXED[] {float * val}
    %apply float FIXED[] {const float * val}
    %apply double FIXED[] {double* val}
    %apply double FIXED[] {const double* val}
#else
    %apply unsigned char INPUT[] {const unsigned char* val}
    %apply unsigned char OUTPUT[] {unsigned char* val}
    %apply int INPUT[] {const int* val}
    %apply int OUTPUT[] {int* val}
    %apply float INPUT[] {const float * val}
    %apply float OUTPUT[] {float * val}
    %apply double INPUT[] {const double* val}
    %apply double OUTPUT[] {double* val}
#endif

%pointer_functions(int, intPtr)
%pointer_functions(float, floatPtr)
%pointer_functions(double, doublePtr)
%pointer_functions(unsigned char, ucharPtr)

%rename(getValue) imstk::VecDataArray::operator[] (const size_t pos) const;
%rename(setValue) imstk::VecDataArray::operator[] (const size_t pos);
%rename(getValue) imstk::DataArray::operator[] (const size_t pos) const;
%rename(setValue) imstk::DataArray::operator[] (const size_t pos);
%rename(getValue) imstk::Vec::operator[] (const int pos) const;
%rename(setValue) imstk::Vec::operator[] (const int pos);

%rename(Rotf) imstk::imstkRotf;
%rename(Rotd) imstk::imstkRotd;
%rename(Quatf) imstk::imstkQuatf;
%rename(Quatd) imstk::imstkQuatd;

%define %extend_VecDataArray(T, N)
    %extend imstk::VecDataArray<T, N>
    {
        void setValues(const T* val)
        {
            /* std::copy(val, val+$self->m_vecSize * N, $self->Base::m_data); */
            T* data = $self->DataArray<T>::getPointer();
            std::copy(val, val + $self->size() * N, data);
        }

        void setValues(const T* val, const int n)
        {
            T* data = $self->DataArray<T>::getPointer();
            CHECK($self->size()*N >= n) << "number of values are larger than the array size";
            std::copy(val, val + n, data);
        }

        void getValues(T* val)
        {
            T* data = $self->DataArray<T>::getPointer();
            std::copy(data, data+$self->size()*N, val);
        }
    };
%enddef
%define %extend_DataArray(T)
    %extend imstk::DataArray<T>
    {
        void setValues(const T* val)
        {
            /* std::copy(val, val+$self->m_vecSize, $self->Base::m_data); */
            T* data = $self->DataArray<T>::getPointer();
            std::copy(val, val + $self->size(), data);
        }

        void setValues(const T* val, const int n)
        {
            T* data = $self->DataArray<T>::getPointer();
            CHECK($self->size() >= n) << "number of values are larger than the array size";
            std::copy(val, val + n, data);
        }

        void getValues(T* val)
        {
            T* data = $self->DataArray<T>::getPointer();
            std::copy(data, data+$self->size(), val);
        }
    };
%enddef

%extend_VecDataArray(int, 2)
%extend_VecDataArray(int, 3)
%extend_VecDataArray(int, 4)
%extend_VecDataArray(float, 2)
%extend_VecDataArray(double, 2)
%extend_VecDataArray(double, 3)
%extend_VecDataArray(unsigned char, 3)

%extend_DataArray(float)
%extend_DataArray(double)
%extend_DataArray(int)
%extend_DataArray(unsigned char)

/*
 * TODO: use %define to simplify the process of wrapping Eigen types.
 * What we do here is define our own interface for Eigen as the Eigen types
 * themselves are quite cumbersome to wrap.
 */

%typemap(cscode) imstk::imstkQuatf
%{
    public static implicit operator SWIGTYPE_p_Eigen__QuaternionT_float_t (Quatf cs_data)
    {
        return cs_data.get();
    }

    public static implicit operator Quatf (SWIGTYPE_p_Eigen__QuaternionT_float_t eigen_data)
    {
        return Utils.eigen2cs_Quatf(eigen_data);
    }
%}

%typemap(cscode) imstk::imstkQuatd
%{
    public static implicit operator SWIGTYPE_p_Eigen__QuaternionT_double_t (Quatd cs_data)
    {
        return cs_data.get();
    }

    public static implicit operator Quatd (SWIGTYPE_p_Eigen__QuaternionT_double_t eigen_data)
    {
        return Utils.eigen2cs_Quatd(eigen_data);
    }
%}

#ifdef SWIGCSHARP
%inline
%{
namespace imstk
{
template<typename T, int N>
class Vec
{
public:
    using EigenData = Eigen::Matrix<T, N, 1>;
    Vec(const Vec& other) = default;
    Vec() = default;
    Vec(const T t0, const T t1)
    {
        m_data[0] = t0;
        m_data[1] = t1;
    }
    Vec(const T t0, const T t1, const T t2)
    {
        m_data[0] = t0;
        m_data[1] = t1;
        m_data[2] = t2;
    }
    Vec(const T t0, const T t1, const T t2, const T t3)
    {
        m_data[0] = t0;
        m_data[1] = t1;
        m_data[2] = t2;
        m_data[3] = t3;
    }
    Vec(const T t0, const T t1, const T t2, const T t3, const T t4, const T t5)
    {
        m_data[0] = t0;
        m_data[1] = t1;
        m_data[2] = t2;
        m_data[3] = t3;
        m_data[4] = t4;
        m_data[5] = t5;
    }
    // Vec(int i0, int i1, int i2, int i3)
    Vec(const EigenData& other) { m_data = other; }
    inline T& operator[](const int pos) { return m_data[pos]; }
    inline const T& operator[](const int pos) const { return m_data[pos]; }
    inline const EigenData& get() const { return m_data; }

private:
    EigenData m_data;
};

template <typename T, int N>
class Mat
{
public:
    using EigenData = Eigen::Matrix<T, N, N>;
    Mat (const EigenData& data) : m_data(data) { }
    Mat() : m_data(EigenData::Identity()) { }
    static Mat Identity () { return Mat (EigenData::Identity()); }
    inline const EigenData& get() const { return m_data; }

    inline void setValue(int row, int col, const T& val) { m_data(row, col) = val; }
    inline T getValue(int row, int col) const { return m_data(row, col); }

private:
    EigenData m_data;
};

class imstkRotf
{
public:
    using EigenData = Eigen::AngleAxisf;
    imstkRotf(float s, const imstk::Vec3f& vec): m_data(s, vec) { }
    const EigenData& get() const { return m_data; }

private:
    EigenData m_data;
};

class imstkRotd
{
public:
    using EigenData = Eigen::AngleAxisd;
    imstkRotd(double s, const imstk::Vec3d& vec): m_data(s, vec) { }
    inline const EigenData& get() const { return m_data; }

private:
    EigenData m_data;
};

class imstkQuatf
{
public:
    using EigenData = Eigen::Quaternion<float>;
    imstkQuatf(const EigenData& data) : m_data(data) { }
    imstkQuatf(const imstkRotf& rot) : m_data(rot.get()) { }
    imstkQuatf(float x, float y, float z, float w) : m_data(w, x, y, z) { }
    inline const EigenData& get() const { return m_data; }

    inline imstk::Mat3f toRotationMatrix() { return m_data.toRotationMatrix(); }
    inline float x() const { return m_data.x(); }
    inline float y() const { return m_data.y(); }
    inline float z() const { return m_data.z(); }
    inline float w() const { return m_data.w(); }

private:
    EigenData m_data;
};

class imstkQuatd
{
public:
    using EigenData = Eigen::Quaternion<double>;
    imstkQuatd(const EigenData& data) : m_data(data) { }
    imstkQuatd(const imstkRotd& rot) : m_data(rot.get()) { }
    imstkQuatd(double x, double y, double z, double w) : m_data(w, x, y, z) { }
    inline const EigenData& get() const { return m_data; }

    inline imstk::Mat3d toRotationMatrix() { return m_data.toRotationMatrix(); }
    inline double x() const { return m_data.x(); }
    inline double y() const { return m_data.y(); }
    inline double z() const { return m_data.z(); }
    inline double w() const { return m_data.w(); }

private:
    EigenData m_data;
};

template<typename T, int N>
Vec<T, N> vec_add(const Vec<T, N>& u, const Vec<T, N>& v) 
{
    Vec<T, N> ret(u);
    for (int i=0; i<N; ++i)
    {
        ret[i] += v[i];
    }
    return ret;
}

template<typename T, int N>
Vec<T, N> vec_subtract(const Vec<T, N>& u, const Vec<T, N>& v) 
{
    Vec<T, N> ret(u);
    for (int i=0; i<N; ++i)
    {
        ret[i] -= v[i];
    }
    return ret;
}

template<typename T, int N>
Vec<T, N> vec_scale(const Vec<T, N>& v, const T c) 
{
    Vec<T, N> ret(v);
    for (int i=0; i<N; ++i)
    {
        ret[i] *= c;
    }
    return ret;
}

template <typename T, int N>
Vec<T, N> vec_to_eigen(typename VecDataArray<T, N>::ValueType* v2, const Vec<T, N>& v1)
{
    /* *v2 = v1; */
    *v2 = v1.get();
    return v1;
}

Vec<double, 6> vec_to_eigen_6d(imstk::Vec6d* v2, const Vec<double, 6>& v1)
{
    /* *v2 = v1; */
    *v2 = v1.get();
    return v1;
}

template <typename T, int N>
Vec<T, N> vec_from_eigen(const typename VecDataArray<T, N>::ValueType& v1)
{
    return Vec<T, N>(v1);
}

Vec<double, 6> vec_from_eigen_6d(const imstk::Vec6d& v1)
{
    return Vec<double, 6>(v1);
}

template <typename T, int N>
void vecdataarray_push_back(VecDataArray<T, N>& vec, const Vec<T, N>& vi)
{
    vec.push_back(vi.get());
}

template <typename csType, typename EigenType>
csType eigen2cs(const EigenType& eigen_data)
{
    return csType(eigen_data);
}

template <typename csType, typename EigenType>
EigenType cs2Eigen(const csType& cs_data, EigenType* eigen_data)
{
    return *eigen_data = cs_data.get();
}

} /* end of namespace imstk */

%} // end of inline


%typemap(cscode) imstk::Vec<int, 2>
%{
    public int this[int i]
    {
        get => getValue(i);
        set => Utils.intPtr_assign(setValue(i), value);
    }

    public static Vec2i operator + (Vec2i u, Vec2i v)
    {
        return Utils.vec_add_2i(u, v);
    }
    public static Vec2i operator - (Vec2i u, Vec2i v)
    {
        return Utils.vec_subtract_2i(u, v);
    }
    public static Vec2i operator * (Vec2i v, int c)
    {
        return Utils.vec_scale_2i(v, c);
    }

    public static Vec2i operator * (int c, Vec2i v)
    {
        return Utils.vec_scale_2i(v, c);
    }

    public static implicit operator SWIGTYPE_p_Eigen__MatrixT_int_2_1_t (Vec2i v)
    {
        return v.get(); 
    }

    public static implicit operator Vec2i(SWIGTYPE_p_Eigen__MatrixT_int_2_1_t eigen_v)
    {
        return Utils.vec_from_eigen_2i(eigen_v);
    }
%}

%typemap(cscode) imstk::Vec<int, 3>
%{
    public int this[int i]
    {
        get => getValue(i);
        set => Utils.intPtr_assign(setValue(i), value);
    }

    public static Vec3i operator + (Vec3i u, Vec3i v)
    {
        return Utils.vec_add_3i(u, v);
    }
    public static Vec3i operator - (Vec3i u, Vec3i v)
    {
        return Utils.vec_subtract_3i(u, v);
    }
    public static Vec3i operator * (Vec3i v, int c)
    {
        return Utils.vec_scale_3i(v, c);
    }

    public static Vec3i operator * (int c, Vec3i v)
    {
        return Utils.vec_scale_3i(v, c);
    }

    public static implicit operator SWIGTYPE_p_Eigen__MatrixT_int_3_1_t (Vec3i v)
    {
        return v.get(); 
    }

    public static implicit operator Vec3i(SWIGTYPE_p_Eigen__MatrixT_int_3_1_t eigen_v)
    {
        return Utils.vec_from_eigen_3i(eigen_v);
    }
%}

%typemap(cscode) imstk::Vec<int, 4>
%{
    public int this[int i]
    {
        get => getValue(i);
        set => Utils.intPtr_assign(setValue(i), value);
    }

    public static Vec4i operator + (Vec4i u, Vec4i v)
    {
        return Utils.vec_add_4i(u, v);
    }
    public static Vec4i operator - (Vec4i u, Vec4i v)
    {
        return Utils.vec_subtract_4i(u, v);
    }
    public static Vec4i operator * (Vec4i v, int c)
    {
        return Utils.vec_scale_4i(v, c);
    }

    public static Vec4i operator * (int c, Vec4i v)
    {
        return Utils.vec_scale_4i(v, c);
    }

    public static implicit operator SWIGTYPE_p_Eigen__MatrixT_int_4_1_t (Vec4i v)
    {
        return v.get(); 
    }

    public static implicit operator Vec4i(SWIGTYPE_p_Eigen__MatrixT_int_4_1_t eigen_v)
    {
        return Utils.vec_from_eigen_4i(eigen_v);
    }
%}

%typemap(cscode) imstk::Vec<float, 2>
%{
    public float this[int i]
    {
        get => getValue(i);
        set => Utils.floatPtr_assign(setValue(i), value);
    }
    
    public static Vec2f operator + (Vec2f u, Vec2f v)
    {
        return Utils.vec_add_2f(u, v);
    }
    public static Vec2f operator - (Vec2f u, Vec2f v)
    {
        return Utils.vec_subtract_2f(u, v);
    }
    public static Vec2f operator * (Vec2f v, float c)
    {
        return Utils.vec_scale_2f(v, c);
    }

    public static Vec2f operator * (float c, Vec2f v)
    {
        return Utils.vec_scale_2f(v, c);
    }
    public static Vec2f operator / (Vec2f v, float c)
    {
        return Utils.vec_scale_2f(v, 1.0f / c);
    }

    public static Vec2f operator / (float c, Vec2f v)
    {
        return Utils.vec_scale_2f(v, 1.0f / c);
    }

    public static implicit operator SWIGTYPE_p_Eigen__MatrixT_float_2_1_t (Vec2f v)
    {
        return v.get(); 
    }

    public static implicit operator Vec2f(SWIGTYPE_p_Eigen__MatrixT_float_2_1_t eigen_v)
    {
        return Utils.vec_from_eigen_2f(eigen_v);
    }
%}

%typemap(cscode) imstk::Vec<double, 2>
%{
    public double this[int i]
    {
        get => getValue(i);
        set => Utils.doublePtr_assign(setValue(i), value);
    }
    
    public static Vec2d operator + (Vec2d u, Vec2d v)
    {
        return Utils.vec_add_2d(u, v);
    }
    public static Vec2d operator - (Vec2d u, Vec2d v)
    {
        return Utils.vec_subtract_2d(u, v);
    }
    public static Vec2d operator * (Vec2d v, double c)
    {
        return Utils.vec_scale_2d(v, c);
    }

    public static Vec2d operator * (double c, Vec2d v)
    {
        return Utils.vec_scale_2d(v, c);
    }

    public static Vec2d operator / (Vec2d v, double c)
    {
        return Utils.vec_scale_2d(v, 1.0 / c);
    }

    public static Vec2d operator / (double c, Vec2d v)
    {
        return Utils.vec_scale_2d(v, 1.0 / c);
    }

    public static implicit operator SWIGTYPE_p_Eigen__MatrixT_double_2_1_t (Vec2d v)
    {
        return v.get(); 
    }

    public static implicit operator Vec2d(SWIGTYPE_p_Eigen__MatrixT_double_2_1_t eigen_v)
    {
        return Utils.vec_from_eigen_2d(eigen_v);
    }
%}

%typemap(cscode) imstk::Vec<double, 3>
%{
    public double this[int i]
    {
        get => getValue(i);
        set => Utils.doublePtr_assign(setValue(i), value);
    }

    public static Vec3d operator + (Vec3d u, Vec3d v)
    {
        return Utils.vec_add_3d(u, v);
    }
    public static Vec3d operator - (Vec3d u, Vec3d v)
    {
        return Utils.vec_subtract_3d(u, v);
    }
    public static Vec3d operator * (Vec3d v, double c)
    {
        return Utils.vec_scale_3d(v, c);
    }

    public static Vec3d operator * (double c, Vec3d v)
    {
        return Utils.vec_scale_3d(v, c);
    }

    public static Vec3d operator / (Vec3d v, double c)
    {
        return Utils.vec_scale_3d(v, 1.0/c);
    }

    public static Vec3d operator / (double c, Vec3d v)
    {
        return Utils.vec_scale_3d(v, 1.0 / c);
    }

    public static implicit operator SWIGTYPE_p_Eigen__MatrixT_double_3_1_t (Vec3d v)
    {
        return v.get(); 
    }

    public static implicit operator Vec3d(SWIGTYPE_p_Eigen__MatrixT_double_3_1_t eigen_v)
    {
        return Utils.vec_from_eigen_3d(eigen_v);
    }
    public Vec3d normalized()
    {
        double nrm= getValue(0) * getValue(0) + getValue(1) * getValue(1) + getValue(2) * getValue(2);
        return this / System.Math.Sqrt(nrm);
    }
%}

%typemap(cscode) imstk::Vec<double, 6>
%{
    public double this[int i]
    {
        get => getValue(i);
        set => Utils.doublePtr_assign(setValue(i), value);
    }
    
    public static Vec6d operator + (Vec6d u, Vec6d v)
    {
        return Utils.vec_add_6d(u, v);
    }
    public static Vec6d operator - (Vec6d u, Vec6d v)
    {
        return Utils.vec_subtract_6d(u, v);
    }

    public static Vec6d operator * (Vec6d v, double c)
    {
        return Utils.vec_scale_6d(v, c);
    }

    public static Vec6d operator * (double c, Vec6d v)
    {
        return Utils.vec_scale_6d(v, c);
    }
    public static Vec6d operator / (Vec6d v, double c)
    {
        return Utils.vec_scale_6d(v, 1.0 / c);
    }

    public static Vec6d operator / (double c, Vec6d v)
    {
        return Utils.vec_scale_6d(v, 1.0 / c);
    }

    public static implicit operator SWIGTYPE_p_Eigen__MatrixT_double_6_1_t (Vec6d v)
    {
        return v.get(); 
    }

    public static implicit operator Vec6d(SWIGTYPE_p_Eigen__MatrixT_double_6_1_t eigen_v)
    {
        return Utils.vec_from_eigen_6d(eigen_v);
    }
%}

%typemap(cscode) imstk::Vec<unsigned char, 3>
%{
    /* public unsigned char this[int i] { */
    public byte this[int i]
    {
        get => getValue(i);
        set => Utils.ucharPtr_assign(setValue(i), value);
    }

    public static Vec3uc operator + (Vec3uc u, Vec3uc v)
    {
        return Utils.vec_add_3uc(u, v);
    }
    public static Vec3uc operator - (Vec3uc u, Vec3uc v)
    {
        return Utils.vec_subtract_3uc(u, v);
    }
    public static Vec3uc operator * (Vec3uc v, byte c)
    {
        return Utils.vec_scale_3uc(v, c);
    }

    public static Vec3uc operator * (byte c, Vec3uc v)
    {
        return Utils.vec_scale_3uc(v, c);
    }

    public static implicit operator SWIGTYPE_p_Eigen__MatrixT_unsigned_char_3_1_t (Vec3uc v)
    {
        return v.get(); 
    }

    public static implicit operator Vec3uc(SWIGTYPE_p_Eigen__MatrixT_unsigned_char_3_1_t eigen_v)
    {
        return Utils.vec_from_eigen_3uc(eigen_v);
    }
%}

%typemap(cscode) imstk::VecDataArray<int, 2>
%{
    public Vec2i this[uint i]
    {
        get => Utils.vec_from_eigen_2i(getValue(i));
        set => Utils.vec_to_eigen_2i(setValue(i), value);
    }
    public void push_back(Vec2i v)
    {
        Utils.vecdataarray_push_back_2i(this, v);
    }
%}

%typemap(cscode) imstk::VecDataArray<int, 3>
%{
    public Vec3i this[uint i]
    {
        get => Utils.vec_from_eigen_3i(getValue(i));
        set => Utils.vec_to_eigen_3i(setValue(i), value);
    }
    public void push_back(Vec3i v)
    {
        Utils.vecdataarray_push_back_3i(this, v);
    }
%}

%typemap(cscode) imstk::VecDataArray<int, 4>
%{
    public Vec4i this[uint i]
    {
        get => Utils.vec_from_eigen_4i(getValue(i));
        set => Utils.vec_to_eigen_4i(setValue(i), value);
    }
    public void push_back(Vec4i v)
    {
        Utils.vecdataarray_push_back_4i(this, v);
    }

%}

%typemap(cscode) imstk::VecDataArray<float, 2>
%{
    public Vec2f this[uint i]
    {
        get => Utils.vec_from_eigen_2f(getValue(i));
        set => Utils.vec_to_eigen_2f(setValue(i), value);
    }
    public void push_back(Vec2f v)
    {
        Utils.vecdataarray_push_back_2f(this, v);
    }
%}

%typemap(cscode) imstk::VecDataArray<double, 2>
%{
    public Vec2d this[uint i]
    {
        get => Utils.vec_from_eigen_2d(getValue(i));
        set => Utils.vec_to_eigen_2d(setValue(i), value);
    }
    public void push_back(Vec2d v)
    {
        Utils.vecdataarray_push_back_2d(this, v);
    }
%}

%typemap(cscode) imstk::VecDataArray<double, 3>
%{
    public Vec3d this[uint i]
    {
        get => Utils.vec_from_eigen_3d(getValue(i));
        set => Utils.vec_to_eigen_3d(setValue(i), value);
    }
    public void push_back(Vec3d v)
    {
        Utils.vecdataarray_push_back_3d(this, v);
    }
%}

%typemap(cscode) imstk::VecDataArray<unsigned char, 3>
%{
    public Vec3uc this[uint i]
    {
        get => Utils.vec_from_eigen_3uc(getValue(i));
        set => Utils.vec_to_eigen_3uc(setValue(i), value);
    }
    public void push_back(Vec3uc v)
    {
        Utils.vecdataarray_push_back_3uc(this, v);
    }
%}

%typemap(cscode) imstk::Mat<double, 3>
%{
    public static implicit operator SWIGTYPE_p_Eigen__MatrixT_double_3_3_t (Mat3d cs_data)
    {
        return cs_data.get();
    }

    public static implicit operator Mat3d (SWIGTYPE_p_Eigen__MatrixT_double_3_3_t eigen_data)
    {
        return Utils.eigen2cs_Mat3d(eigen_data);
    }
%}

%typemap(cscode) imstk::Mat<double, 4>
%{
    public static implicit operator SWIGTYPE_p_Eigen__MatrixT_double_4_4_t (Mat4d cs_data)
    {
        return cs_data.get();
    }

    public static implicit operator Mat4d (SWIGTYPE_p_Eigen__MatrixT_double_4_4_t eigen_data)
    {
        return Utils.eigen2cs_Mat4d(eigen_data);
    }
%}

/* Make the renamed function private */
%csmethodmodifiers operator[] "private";

#endif /* end of #ifdef SWIGCSHARP */

%include "../../Common/imstkMacros.h"
%include "../../Common/imstkTypes.h";
%include "../../Common/imstkMath.h";
%include "../../Common/imstkEventObject.h";
%include "../../Common/imstkAbstractDataArray.h";
%include "../../Common/imstkDataArray.h";
%template(DataArrayi) imstk::DataArray<int>;
%template(DataArrayf) imstk::DataArray<float>;
%template(DataArrayd) imstk::DataArray<double>;
%template(DataArrayuc) imstk::DataArray<unsigned char>;
%include "../../Common/imstkLogger.h";
#ifdef iMSTK_SYNCHRONOUS_LOGGING
%rename (Logger) LoggerSynchronous;
%include "../../Common/imstkLoggerSynchronous.h"
#else 
%rename (Logger) LoggerG3;
%include "../../Common/imstkLoggerG3.h"
#endif
%include "../../Common/imstkModule.h";
%include "../../Common/imstkModuleDriver.h";
%include "../../Common/imstkColor.h";
%include "../../Common/imstkVecDataArray.h";
%template(VecDataArray2i) imstk::VecDataArray<int, 2>;
%template(VecDataArray3i) imstk::VecDataArray<int, 3>;
%template(VecDataArray4i) imstk::VecDataArray<int, 4>;
%template(VecDataArray2f) imstk::VecDataArray<float, 2>;
%template(VecDataArray2d) imstk::VecDataArray<double, 2>;
%template(VecDataArray3d) imstk::VecDataArray<double, 3>;
%template(VecDataArray3uc) imstk::VecDataArray<unsigned char, 3>;

/*
 * Instantiation of Vec and VecDataArray
 */
%template(Vec2i) imstk::Vec<int, 2>;
%template(Vec3i) imstk::Vec<int, 3>;
%template(Vec4i) imstk::Vec<int, 4>;
%template(Vec2f) imstk::Vec<float, 2>;
%template(Vec2d) imstk::Vec<double, 2>;
%template(Vec3d) imstk::Vec<double, 3>;
%template(Vec6d) imstk::Vec<double, 6>;
%template(Vec3uc) imstk::Vec<unsigned char, 3>;

%template(vec_scale_2i) imstk::vec_scale<int, 2>;
%template(vec_scale_3i) imstk::vec_scale<int, 3>;
%template(vec_scale_4i) imstk::vec_scale<int, 4>;
%template(vec_scale_2f) imstk::vec_scale<float, 2>;
%template(vec_scale_2d) imstk::vec_scale<double, 2>;
%template(vec_scale_3d) imstk::vec_scale<double, 3>;
%template(vec_scale_6d) imstk::vec_scale<double, 6>;
%template(vec_scale_3uc) imstk::vec_scale<unsigned char, 3>;

%template(vec_add_2i) imstk::vec_add<int, 2>;
%template(vec_add_3i) imstk::vec_add<int, 3>;
%template(vec_add_4i) imstk::vec_add<int, 4>;
%template(vec_add_2f) imstk::vec_add<float, 2>;
%template(vec_add_2d) imstk::vec_add<double, 2>;
%template(vec_add_3d) imstk::vec_add<double, 3>;
%template(vec_add_6d) imstk::vec_add<double, 6>;
%template(vec_add_3uc) imstk::vec_add<unsigned char, 3>;

%template(vec_subtract_2i) imstk::vec_subtract<int, 2>;
%template(vec_subtract_3i) imstk::vec_subtract<int, 3>;
%template(vec_subtract_4i) imstk::vec_subtract<int, 4>;
%template(vec_subtract_2f) imstk::vec_subtract<float, 2>;
%template(vec_subtract_2d) imstk::vec_subtract<double, 2>;
%template(vec_subtract_3d) imstk::vec_subtract<double, 3>;
%template(vec_subtract_6d) imstk::vec_subtract<double, 6>;
%template(vec_subtract_3uc) imstk::vec_subtract<unsigned char, 3>;

%template(vec_to_eigen_2i) imstk::vec_to_eigen<int, 2>;
%template(vec_to_eigen_3i) imstk::vec_to_eigen<int, 3>;
%template(vec_to_eigen_4i) imstk::vec_to_eigen<int, 4>;
%template(vec_to_eigen_2f) imstk::vec_to_eigen<float, 2>;
%template(vec_to_eigen_2d) imstk::vec_to_eigen<double, 2>;
%template(vec_to_eigen_3d) imstk::vec_to_eigen<double, 3>;
%template(vec_to_eigen_3uc) imstk::vec_to_eigen<unsigned char, 3>;

%template(vec_from_eigen_2i) imstk::vec_from_eigen<int, 2>;
%template(vec_from_eigen_3i) imstk::vec_from_eigen<int, 3>;
%template(vec_from_eigen_4i) imstk::vec_from_eigen<int, 4>;
%template(vec_from_eigen_2f) imstk::vec_from_eigen<float, 2>;
%template(vec_from_eigen_2d) imstk::vec_from_eigen<double, 2>;
%template(vec_from_eigen_3d) imstk::vec_from_eigen<double, 3>;
%template(vec_from_eigen_3uc) imstk::vec_from_eigen<unsigned char, 3>;

%template(vecdataarray_push_back_2i) imstk::vecdataarray_push_back<int, 2>;
%template(vecdataarray_push_back_3i) imstk::vecdataarray_push_back<int, 3>;
%template(vecdataarray_push_back_4i) imstk::vecdataarray_push_back<int, 4>;
%template(vecdataarray_push_back_2f) imstk::vecdataarray_push_back<float, 2>;
%template(vecdataarray_push_back_2d) imstk::vecdataarray_push_back<double, 2>;
%template(vecdataarray_push_back_3d) imstk::vecdataarray_push_back<double, 3>;
%template(vecdataarray_push_back_3uc) imstk::vecdataarray_push_back<unsigned char, 3>;

%template(cs2Eigen_Quatf) imstk::cs2Eigen<imstk::imstkQuatf, Eigen::Quaternion<float> >;
%template(cs2Eigen_Quatd) imstk::cs2Eigen<imstk::imstkQuatd, Eigen::Quaternion<double> >;
%template(eigen2cs_Quatf) imstk::eigen2cs<imstk::imstkQuatf, Eigen::Quaternion<float> >;
%template(eigen2cs_Quatd) imstk::eigen2cs<imstk::imstkQuatd, Eigen::Quaternion<double> >;

%template(Mat3d) imstk::Mat<double, 3>;
%template(cs2Eigen_Mat3f) imstk::cs2Eigen<imstk::Mat<float, 3>, Eigen::Matrix<float, 3, 3> >;
%template(cs2Eigen_Mat3d) imstk::cs2Eigen<imstk::Mat<double, 3>, Eigen::Matrix<double, 3, 3> >;
%template(eigen2cs_Mat3f) imstk::eigen2cs<imstk::Mat<float, 3>, Eigen::Matrix<float, 3, 3> >;
%template(eigen2cs_Mat3d) imstk::eigen2cs<imstk::Mat<double, 3>, Eigen::Matrix<double, 3, 3> >;

%template(Mat4d) imstk::Mat<double, 4>;
%template(cs2Eigen_Mat4f) imstk::cs2Eigen<imstk::Mat<float, 4>, Eigen::Matrix<float, 4, 4> >;
%template(cs2Eigen_Mat4d) imstk::cs2Eigen<imstk::Mat<double, 4>, Eigen::Matrix<double, 4, 4> >;
%template(eigen2cs_Mat4f) imstk::eigen2cs<imstk::Mat<float, 4>, Eigen::Matrix<float, 4, 4> >;
%template(eigen2cs_Mat4d) imstk::eigen2cs<imstk::Mat<double, 4>, Eigen::Matrix<double, 4, 4> >;

%define %connectMacro(Name)
    %template(connect##Name##Internal) imstk::connect<imstk::Name>;

    // As the EventFunc is virtual, see std_function.i, we implement it in this
    // C# class
    %pragma(csharp) modulecode=%{
        public class Name##FuncImpl : Name##Func
        {
            public Name##FuncImpl(System.Action<Name> action)
            {
                this.action = action;
            }

            public override void call(Name e)
            {
                if (action != null) { action.Invoke(e); }
            }

            public System.Action<Name> action = null;
        }

        public static void connect##Name (EventObject obj, SWIGTYPE_p_f___std__string arg1, System.Action<Name> action)
        {
            Name##FuncImpl func = new Name##FuncImpl(action);
            connect##Name##Internal (obj, arg1, new Name##FuncStd(func));
        }
    %}
%enddef

%connectMacro(Event)
%connectMacro(KeyEvent)