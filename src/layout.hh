#ifndef _LAYOUT_H_
#define _LAYOUT_H_


#include "layer.hh"
#include "spatial_octree.hh"


// #define REPULSION_BRUTE_FORCE
#define REPULSION_OCTREE_THRESHOLD 2000


// 
// Figure out the size of the bounding box stretched by specified vertices
// 
template <typename _coord_type>
inline void bounding_box(
    const std::vector<vertex<_coord_type> *> &vs,
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

    x_min = xmin - 10; x_max = xmax + 10;
    y_min = ymin - 10; y_max = ymax + 10;
    z_min = zmin - 10; z_max = zmax + 10;
}


// 
// Apply numerical methods on the Lagrange Dynamics formed by the spring system
// defined by this graph.
// 

template <typename _coord_type>
vector3d<_coord_type> layer<_coord_type>::repulsion_force(
    vertex_type *v,
    const std::vector<vertex_type *> &vs)
{
    vector3d_type F_r = vector3d_type::zero;
    _coord_type reps = 2 / sqrt(eps);
    for (auto v2 : vs) {
        if (v != v2) {
            auto dx = v->x - v2->x;
            auto rdd = dx.rmod();
            auto denom = rdd;
            auto fac = f0 * (denom * denom * denom);
            if (rdd > reps) {
                fac = 1;
                dx = vector3d_type(
                    rand_range(-reps, reps),
                    rand_range(-reps, reps),
                    rand_range(-reps, reps));
            }
            F_r += fac * dx;
        }
    }
    return F_r;
}


template <typename _coord_type>
vector3d<_coord_type> layer<_coord_type>::spring_force(
    vertex_type *v1, vertex_type *v2,
    edge_type *e)
{
    vector3d_type F_p = vector3d_type::zero;
    auto dx = v1->x - v2->x;
    F_p -= K * dx * e->strength;
    if (e->oriented) {
        F_p += ((e->b == v1)? 
            vector3d_type(0, -0.4, 0):
            vector3d_type(0,  0.4, 0));
    }
    return F_p;
}


template <typename _coord_type>
void layer<_coord_type>::update_velocity(vertex_type *v, float_type dt)
{
    if (v->coarser) v->ddx_ += dilation * v->coarser->ddx;
    v->dx += float_type(0.5) * (v->ddx + v->ddx_) * dt;
    v->dx *= damping;
    v->ddx = v->ddx_;
}


template <typename _coord_type>
void layer<_coord_type>::apply_displacement(vertex_type *v, float_type dt)
{    
    v->delta = v->dx * dt + (float_type(0.5) * dt*dt) * v->ddx;
    v->delta.bound(3);
    v->x += v->delta;
}



template <typename _coord_type>
_coord_type layer<_coord_type>::layout(float_type dt)
{
    _coord_type max_ddx = 0;
    size_t n_vs = vs.size();

    // move vertices with verlet integration on this layer
    // for (auto v : vs) apply_displacement(v, dt);
#pragma omp parallel for
    for (size_t i = 0; i < n_vs; i++) {
        apply_displacement(vs[i], dt);
    }

#ifndef REPULSION_BRUTE_FORCE
    // construct spatial octree
    _coord_type x_min, x_max, y_min, y_max, z_min, z_max;
    bounding_box(vs, x_min, x_max, y_min, y_max, z_min, z_max);    
    spatial_octree<_coord_type> *t = spatial_octree<_coord_type>::alloc(
        nullptr, 
        x_min, x_max, 
        y_min, y_max, 
        z_min, z_max);
    for (auto v : vs) t->insert(v);
#endif

    // calculate force/acceleration with Lagrange Dynamics
#pragma omp parallel for
    for (size_t i = 0; i < n_vs; i++) {
        auto v = vs[i];
#ifndef REPULSION_BRUTE_FORCE
        vector3d_type F_r = (n_vs > REPULSION_OCTREE_THRESHOLD? 
            t->repulsion_force(v, f0, 1 / sqrt(eps)):
            repulsion_force(v, vs));
#else
        vector3d_type F_r = repulsion_force(v, vs);
#endif
        vector3d_type F_p = vector3d_type::zero;
        
        // spring forces on v
        for (auto e : v->es) {
            if (e->a != e->b) {
                vertex_type *v2 = (e->a == v)? e->b: e->a;
                F_p += spring_force(v, v2, e);
            }
        }

        // net force on v
        v->ddx_ = F_r + F_p;
        max_ddx = std::max(max_ddx, v->ddx_.mod());
    }

#ifndef REPULSION_BRUTE_FORCE
    t->recycle();
#endif

#pragma omp parallel for
    for (size_t i = 0; i < n_vs; i++) {
        this->update_velocity(vs[i], dt);
    }

    return max_ddx;
}


template <typename _coord_type>
_coord_type finest_layer<_coord_type>::layout(float_type dt)
{
    _coord_type max_ddx = 0;

    // 
    // [Take centroid vertices of spline edges into consideration] We need to stuff
    // all vertices (including real vertices and virtual centroid vertices of spline
    // edges) into one unique vertex array, thus the following vertex layout
    // algorithm will operate on all kinds of vertices regardless of whether the
    // vertex is a real styled vertex or edge centroid vertex.
    // 
    std::vector<vertex_type *> vs = this->vs;
    for (auto v : this->vs) {
        for (auto e : v->es) {
            auto e_styled = static_cast<edge_styled<_coord_type> *>(e);
            assert(e_styled != nullptr);
            if (e_styled->spline and e->a == v) {
                if (!e_styled->vspline) e_styled->set_spline();
                vs.push_back(e_styled->vspline);
            }
        }
    }
    
    // move vertices with verlet integration on this layer
    // for (auto v : vs) this->apply_displacement(v, dt);
    size_t n_vs = vs.size();
#pragma omp parallel for
    for (size_t i = 0; i < n_vs; i++) {
        this->apply_displacement(vs[i], dt);
    }

#ifndef REPULSION_BRUTE_FORCE
    // construct spatial octree
    _coord_type x_min, x_max, y_min, y_max, z_min, z_max;
    bounding_box(vs, x_min, x_max, y_min, y_max, z_min, z_max);
    spatial_octree<_coord_type> *t = spatial_octree<_coord_type>::alloc(
        nullptr, 
        x_min, x_max, 
        y_min, y_max, 
        z_min, z_max);
    for (auto v : vs) t->insert(v);
#endif

    // calculate force/acceleration with Lagrange Dynamics
#pragma omp parallel for
    for (size_t i = 0; i < n_vs; i++) {
        auto v = vs[i];
#ifndef REPULSION_BRUTE_FORCE
        vector3d_type F_r = (n_vs > REPULSION_OCTREE_THRESHOLD?
            t->repulsion_force(v, this->f0, 1 / sqrt(this->eps)):
            this->repulsion_force(v, vs));
#else
        vector3d_type F_r = this->repulsion_force(v, vs);
#endif
        vector3d_type F_p = vector3d_type::zero;

        // spring forces on v
        for (auto e : v->es) {
            auto e_styled = dynamic_cast<edge_styled<_coord_type> *>(e);
            bool is_spline_edge = (e_styled and e_styled->spline);
            if (e->a != e->b || is_spline_edge) {
                vertex_type *v2 = nullptr;
                if (is_spline_edge)
                    v2 = e_styled->vspline;
                else
                    v2 = (e->a == v)? e->b: e->a;
                F_p += this->spring_force(v, v2, e);
            }
        }

        // net force on v
        v->ddx_ = F_r + F_p;
        max_ddx = std::max(max_ddx, v->ddx_.mod());
    }

#ifndef REPULSION_BRUTE_FORCE
    t->recycle();
#endif

#pragma omp parallel for
    for (size_t i = 0; i < n_vs; i++) {
        this->update_velocity(vs[i], dt);
    }

    return max_ddx;
}



#endif /* _LAYOUT_H_ */
