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
    // move vertices with verlet integration on this layer
    for (auto v : vs) {
        v->delta = v->dx * dt + (0.5 * dt*dt) * v->ddx;
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
            v->dx += 0.5 * (v->ddx + v->ddx_) * dt;
            v->dx *= damping;
            v->ddx = v->ddx_;
        }
    }
    else {
        for (auto v : vs) {
            v->dx += 0.5 * (v->ddx + v->ddx_) * dt;
            v->dx *= damping;
            v->ddx = v->ddx_;
        }
    }
}


#endif /* _LAYOUT_H_ */
