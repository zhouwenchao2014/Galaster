#include "vec3d.hh"


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
