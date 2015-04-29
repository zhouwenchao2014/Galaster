#ifndef _VEC3D_H_
#define _VEC3D_H_


#if defined(__SSE__) || defined(_M_IX86_FP)
#define SSE_INTRINSINC
#include <xmmintrin.h>
#endif
#include <cmath>
#include <algorithm>


// Baseline implementation for all real types
template <typename _float_type>
class vector3d {
public:
    vector3d(_float_type x, _float_type y, _float_type z) {
        v[0] = x;
        v[1] = y;
        v[2] = z;
    }
    typedef _float_type coord_type;
	static const vector3d<_float_type> zero;
    coord_type v[3];

    vector3d<_float_type>& operator *= (_float_type a) {
        v[0] *= a;
        v[1] *= a;
        v[2] *= a;
        return *this;
    }

    vector3d<_float_type>& operator += (const vector3d<_float_type> &r) {
        v[0] += r.v[0];
        v[1] += r.v[1];
        v[2] += r.v[2];
        return *this;
    }

    vector3d<_float_type>& operator -= (const vector3d<_float_type> &r) {
        v[0] -= r.v[0];
        v[1] -= r.v[1];
        v[2] -= r.v[2];
        return *this;
    }

    void bound(_float_type b) {
        v[0] = std::max(std::min(v[0], b), -b);
        v[1] = std::max(std::min(v[1], b), -b);
        v[2] = std::max(std::min(v[2], b), -b);
    }

    void coord(_float_type &x, _float_type &y, _float_type &z) {
        x = v[0];
        y = v[1];
        z = v[2];
    }

    _float_type mod(void) const {
        return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
    }
};

template <typename _float_type>
const vector3d<_float_type> vector3d<_float_type>::zero(.0, .0, .0);

template <typename _float_type>
vector3d<_float_type> operator * (_float_type a, const vector3d<_float_type> &x)
{
    vector3d<_float_type> res(x);
    res *= a;
    return res;
}

template <typename _float_type>
vector3d<_float_type> operator * (const vector3d<_float_type> &x, _float_type a)
{
    return a * x;
}


template <typename _float_type>
vector3d<_float_type> operator + (
    const vector3d<_float_type> &a, const vector3d<_float_type> &b)
{
    vector3d<_float_type> c(a);
    c += b;
    return c;
}

template <typename _float_type>
vector3d<_float_type> operator - (
    const vector3d<_float_type> &a, const vector3d<_float_type> &b)
{
    vector3d<_float_type> c(a);
    c -= b;
    return c;
}


// special optimization for SSE-enabled platforms
#ifdef SSE_INTRINSINC

template <>
class vector3d<float> {
public:
    vector3d(float x, float y, float z) {
        float s[] = {x, y, z, 0};
        v = _mm_load_ps(s);
    }
    typedef float coord_type;
	static const vector3d<float> zero;
    __m128 v;
};

const vector3d<float> vector3d<float>::zero(.0, .0, .0);


#endif


#endif /* _VEC3D_H_ */
