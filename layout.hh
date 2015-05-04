#ifndef _LAYOUT_H_
#define _LAYOUT_H_


#include "layer.hh"


// 
// Apply numerical methods on the Lagrange Dynamics formed by the spring system
// defined by this graph.
// 
template <typename _coord_type>
void layer<_coord_type>::layout(typename layer<_coord_type>::float_type dt)
{
    typedef typename layer<_coord_type>::float_type float_type;

    // move vertices with verlet integration on this layer
    for (auto v : vs) {
        v->delta = v->dx * dt + (float_type(0.5) * dt*dt) * v->ddx;
        v->delta.bound(3);
        v->x += v->delta;
    }

    // calculate force/acceleration with Lagrange Dynamics
#pragma omp parallel for
    for (auto v : vs) {
        vector3d_type F_r = vector3d_type::zero;
        vector3d_type F_p = vector3d_type::zero;

        // repulsion force on v
        for (auto v2 : vs) {
            if (v != v2) {
                auto dx = v->x - v2->x;
                auto dd = dx.mod();
                auto denom = eps + dd;
                auto fac = f0 / (denom * denom * denom);
                F_r += fac * dx;
            }
        }

        // spring forces on v
        for (auto e : v->es) {
            if (e->a != e->b) {
                vertex_type *v2 = (e->a == v)? e->b: e->a;
                auto dx = v->x - v2->x;
                F_p -= K * dx * e->strength;
                if (e->oriented) {
                    F_p += ((e->b == v)? 
                        vector3d_type(0,  2, 0):
                        vector3d_type(0, -2, 0));
                }
            }
        }

        // net force on v
        v->ddx_ = F_r + F_p;
    }
    
    // calculate velocity with verlet integration
    if (coarser) {
        for (auto v : vs) {
            v->ddx_ += dilation * v->coarser->ddx;
            v->dx += float_type(0.5) * (v->ddx + v->ddx_) * dt;
            v->dx *= damping;
            v->ddx = v->ddx_;
        }
    }
    else {
        for (auto v : vs) {
            v->dx += float_type(0.5) * (v->ddx + v->ddx_) * dt;
            v->dx *= damping;
            v->ddx = v->ddx_;
        }
    }
}


// 
// Figure out the size of the bounding box of the graph
// 
template <typename _coord_type>
void layer<_coord_type>::bounding_box(
    _coord_type &x_min, _coord_type &x_max,
    _coord_type &y_min, _coord_type &y_max,
    _coord_type &z_min, _coord_type &z_max)
{
    _coord_type xmin = -10, xmax = 10;
    _coord_type ymin = -10, ymax = 10;
    _coord_type zmin = -10, zmax = 10;

    for (auto v : vs) {
        _coord_type x, y, z;
        v->x.coord(x, y, z);
        xmin = std::min(xmin, x);
        xmax = std::max(xmax, x);
        ymin = std::min(ymin, y);
        ymax = std::max(ymax, y);
        zmin = std::min(zmin, z);
        zmax = std::max(zmax, z);
    }

    x_min = xmin; x_max = xmax;
    y_min = ymin; y_max = ymax;
    z_min = zmin; z_max = zmax;
}
    


#endif /* _LAYOUT_H_ */
