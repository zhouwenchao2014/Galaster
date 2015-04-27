#ifndef _VEC3D_H_
#define _VEC3D_H_


#if defined(__SSE__) || defined(_M_IX86_FP)
#define SSE_INTRINSINC
#include <xmmintrin.h>
#endif

template <typename _float_type>
class vector3d {
public:
    vector3d(_float_type x, _float_type y, _float_type z) {
        v[0] = x;
        v[1] = y;
        v[2] = z;
    }
    typedef _float_type coord_type;
    coord_type v[3];
};


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
    __m128 v;
};

#endif


#endif /* _VEC3D_H_ */
