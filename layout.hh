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

    // 
    // [Take centroid vertices of spline edges into consideration] We need to merge
    // all vertices (including real vertices and virtual centroid vertices of spline
    // edges into one unique vertex array
    // 
    std::vector<vertex_type *> vs = this->vs;
    for (auto v : this->vs) {
        for (auto e : v->es) {
            auto *e_styled = dynamic_cast<edge_styled<_coord_type> *>(e);
            if (e_styled and e_styled->spline and e->a == v) {
                vs.push_back(e_styled->vspline);
            }
        }
    }

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
                if (dx.mod() < eps)
                    dx = vector3d_type(
                        rand_range(-eps, eps),
                        rand_range(-eps, eps),
                        rand_range(-eps, eps));
                F_r += fac * dx;
            }
        }

        // spring forces on v
        for (auto e : v->es) {
            auto e_styled = dynamic_cast<edge_styled<_coord_type> *>(e);
            bool is_spline_edge = (e_styled and e_styled->spline);
            if (e->a != e->b || is_spline_edge) {
                vertex_type *v2 = nullptr;
                if (is_spline_edge) {
                    v2 = e_styled->vspline;
                }
                else {
                    v2 = (e->a == v)? e->b: e->a;
                }

                auto dx = v->x - v2->x;
                F_p -= K * dx * e->strength;
                if (e->oriented) {
                    F_p += ((e->b == v)? 
                        vector3d_type(0, -0.4, 0):
                        vector3d_type(0,  0.4, 0));
                }
            }
        }

        // net force on v
        v->ddx_ = F_r + F_p;
    }
    
    // calculate velocity with verlet integration
    if (coarser) {
        for (auto v : vs) {
            v->ddx_ += dilation * (v->coarser? v->coarser->ddx: vector3d_type::zero);
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


#endif /* _LAYOUT_H_ */
