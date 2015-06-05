#ifndef _VEC3D_H_
#define _VEC3D_H_


#if defined(__SSE__) || defined(_M_IX86_FP)
#define SSE_INTRINSINC
#ifdef __SSE4_1__
#include <smmintrin.h>
#else
#include <xmmintrin.h>
#endif
#include <type_traits>
#endif
#include <cmath>
#include <cstdlib>
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

    _float_type rmod(void) const {
        return 1 / mod();
    }

    vector3d<_float_type> normalized() const {
        _float_type k = 1 / mod();
        return k * (*this);
    }

    _float_type dot(const vector3d<_float_type> &rhs) const {
        return rhs.v[0] * v[0] + rhs.v[1] * v[1] + rhs.v[2] * v[2];
    }

    vector3d<_float_type> cross(const vector3d<_float_type> &rhs) const {
        _float_type x = v[1] * rhs.v[2] - v[2] * rhs.v[1];
        _float_type y = v[2] * rhs.v[0] - v[0] * rhs.v[2];
        _float_type z = v[0] * rhs.v[1] - v[1] * rhs.v[0];
        return vector3d<_float_type>(x, y, z);
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
    typedef typename std::aligned_storage<4, 16>::type sse_aligned[4];
    vector3d(float x, float y, float z) {
        sse_aligned data;
        float *s = reinterpret_cast<float *>(&data);
        s[0] = x;
        s[1] = y;
        s[2] = z;
        s[3] = 0;
        v = _mm_load_ps(s);
    }
    vector3d(__m128 v)
        : v(v) {
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
        sse_aligned data;
        float *buf = reinterpret_cast<float *>(&data);
        _mm_store_ps(buf, v);
        x = buf[0];
        y = buf[1];
        z = buf[2];
    }

    float mod(void) const {
        sse_aligned data;
        float *buf = reinterpret_cast<float *>(&data);
        __m128 t = v;
        t = _mm_mul_ps(t, t);
        _mm_store_ps(buf, t);
        return sqrt(buf[0] + buf[1] + buf[2]);
    }

    float rmod(void) const {
#ifdef __SSE4_1__
        float ret;
        __m128 t = v;
        t = _mm_mul_ps(t, t);
        t = _mm_hadd_ps(t, t);
        t = _mm_hadd_ps(t, t);
        t = _mm_rsqrt_ss(t);
        _mm_store_ss(&ret, t);
        return ret;
#else
        return 1 / mod();
#endif
    }

    vector3d<float> normalized() const {
        float k = 1 / mod();
        return k * (*this);
    }

    float dot(const vector3d<float> &rhs) const {
        sse_aligned data;
        float *buf = reinterpret_cast<float *>(&data);
        __m128 r = _mm_mul_ps(v, rhs.v);
        _mm_store_ps(buf, r);
        return buf[0] + buf[1] + buf[2];
    }

    vector3d<float> cross(const vector3d<float> &rhs) const {
        sse_aligned data0, data1;
        float *v0 = reinterpret_cast<float *>(&data0);
        float *v1 = reinterpret_cast<float *>(&data1);
        _mm_store_ps(v0, v);
        _mm_store_ps(v1, rhs.v);
        float x = v0[1] * v1[2] - v0[2] * v1[1];
        float y = v0[2] * v1[0] - v0[0] * v1[2];
        float z = v0[0] * v1[1] - v0[1] * v1[0];
        return vector3d<float>(x, y, z);
    }

    vector3d<float> cross2(const vector3d<float> &rhs) const {
        __m128 a = _mm_shuffle_ps(v, v, _MM_SHUFFLE(1,2,0,3));
        __m128 b = _mm_shuffle_ps(rhs.v, rhs.v, _MM_SHUFFLE(2,0,1,3));
        a = _mm_mul_ps(a, b);
        __m128 a1 = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2,0,1,3));
        __m128 b1 = _mm_shuffle_ps(rhs.v, rhs.v, _MM_SHUFFLE(1,2,0,3));
        a1 = _mm_mul_ps(a1, b1);
        a = _mm_sub_ps(a, a1);
        return vector3d<float>(a);
    }
};


#endif


#endif /* _VEC3D_H_ */
