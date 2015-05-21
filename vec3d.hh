#ifndef _VEC3D_H_
#define _VEC3D_H_


#if defined(__SSE__) || defined(_M_IX86_FP)
#define SSE_INTRINSINC
#include <xmmintrin.h>
#endif
#include <cmath>
#include <algorithm>


inline double rand_range(double from, double to) {
    double n = to - from;
    return rand() * n / RAND_MAX + from;
}


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

    void coord(_float_type &x, _float_type &y, _float_type &z) const {
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

    vector3d<float>& operator *= (float a) {
        __m128 va = _mm_set1_ps(a);
        v = _mm_mul_ps(v, va);
        return *this;
    }

    vector3d<float>& operator += (const vector3d<float> &r) {
        v = _mm_add_ps(v, r.v);
        return *this;
    }

    vector3d<float>& operator -= (const vector3d<float> &r) {
        v = _mm_sub_ps(v, r.v);
        return *this;
    }

    void bound(float b) {
        __m128 vb = _mm_set1_ps(b);
        __m128 vnb = _mm_set1_ps(-b);
        v = _mm_min_ps(v, vb);
        v = _mm_max_ps(v, vnb);
    }

    void coord(float &x, float &y, float &z) const {
        float buf[4];
        _mm_store_ps(buf, v);
        x = buf[0];
        y = buf[1];
        z = buf[2];
    }

    float mod(void) const {
        float buf[4];
        __m128 t = v;
        t = _mm_mul_ps(t, t);
        _mm_store_ps(buf, t);
        return sqrt(buf[0] + buf[1] + buf[2]);
    }
};

const vector3d<float> vector3d<float>::zero(.0, .0, .0);

vector3d<float> operator * (float a, const vector3d<float> &x)
{
    vector3d<float> res(x);
    res *= a;
    return res;
}

vector3d<float> operator * (const vector3d<float> &x, float a)
{
    return a * x;
}

vector3d<float> operator + (
    const vector3d<float> &a, const vector3d<float> &b)
{
    vector3d<float> c(a);
    c += b;
    return c;
}

vector3d<float> operator - (
    const vector3d<float> &a, const vector3d<float> &b)
{
    vector3d<float> c(a);
    c -= b;
    return c;
}

#endif


#endif /* _VEC3D_H_ */
