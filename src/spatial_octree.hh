#ifndef _SPATIAL_OCTREE_H_
#define _SPATIAL_OCTREE_H_

#include "vertex_edge.hh"
#include <string.h>


template <typename _coord_type>
class spatial_octree
{
public:
    typedef vertex<_coord_type>    vertex_type;
    typedef vector3d<_coord_type>  vector3d_type;
    typedef _coord_type            float_type;

    spatial_octree(
        vertex_type *v,
        float_type x_min, float_type x_max,
        float_type y_min, float_type y_max,
        float_type z_min, float_type z_max)
        : v(v),
          n_vertices(0),
          c(0, 0, 0),
          x(0.5 * (x_min + x_max)),
          y(0.5 * (y_min + y_max)),
          z(0.5 * (z_min + z_max)),
          x_min(x_min), x_max(x_max),
          y_min(y_min), y_max(y_max),
          z_min(z_min), z_max(z_max)
    {
        memset(subspaces, 0, sizeof(subspaces));
        if (v) {
            n_vertices = 1;
            c = v->x;
        }
    }

    void reset(vertex_type *v,
        float_type x_min, float_type x_max,
        float_type y_min, float_type y_max,
        float_type z_min, float_type z_max)
    {
        this->v = v;
        this->x_min = x_min;
        this->x_max = x_max;
        this->y_min = y_min;
        this->y_max = y_max;
        this->z_min = z_min;
        this->z_max = z_max;
        this->x = 0.5 * (x_min + x_max);
        this->y = 0.5 * (y_min + y_max);
        this->z = 0.5 * (z_min + z_max);
        memset(subspaces, 0, sizeof(subspaces));
        if (v) {
            n_vertices = 1;
            c = v->x;
        }
        else {
            n_vertices = 0;
            c = vector3d_type::zero;
        }
    }

    ~spatial_octree(void) = delete;

    void recycle(void) {
        for (int i = 0; i < 8; i++) {
            if (subspaces[i]) subspaces[i]->recycle();
        }
        dealloc(this);
    }

    void insert(vertex_type *v)
    {
        float_type vx, vy, vz;
        v->x.coord(vx, vy, vz);

        if (vx > x_max || vx < x_min ||
            vy > y_max || vy < y_min || 
            vz > z_max || vz < z_min) {
            printf("v(%f, %f, %f) is out of the bounding box\n", vx, vy, vz);
            exit(-1);
        }

        // which subspace is this vertex in
        int lx = (vx >= x and vx <= x_max);
        int ly = (vy >= y and vy <= y_max);
        int lz = (vz >= z and vz <= z_max);
        int i_subspace = ((lx << 2) | (ly << 1) | lz);

        spatial_octree *subspace = subspaces[i_subspace];
        if (subspace == nullptr) {
            // this subspace is empty, insert this vertex here
            float_type v_xmin, v_xmax, v_ymin, v_ymax, v_zmin, v_zmax;
            if (lx) v_xmin = x, v_xmax = x_max;
            else    v_xmin = x_min, v_xmax = x;
            if (ly) v_ymin = y, v_ymax = y_max;
            else    v_ymin = y_min, v_ymax = y;
            if (lz) v_zmin = z, v_zmax = z_max;
            else    v_zmin = z_min, v_zmax = z;

            subspaces[i_subspace] = alloc(
                v, v_xmin, v_xmax, v_ymin, v_ymax, v_zmin, v_zmax);

            n_vertices += 1;
            c += v->x;
        }
        else if (subspace->v == nullptr) {
            // this subspace is subdivided into sub-subspaces, insert this vertex
            // into appropriate sub-subspace
            subspace->insert(v);
            n_vertices += 1;
            c += v->x;
        }
        else {
            // subspace already containing a vertex, split this subspace into 8
            // smaller sub-subspaces, lower subspace->v one level down, and try
            // inserting v again
            vertex_type *vv = subspace->v;
            if ((v->x - vv->x).mod() < 1e-6) return;

            subspace->v = nullptr;
            subspace->n_vertices = 0;
            subspace->c = vector3d_type::zero;
            subspace->insert(vv);
            subspace->insert(v);
        }
    }

    vector3d_type centroid(void) const {
        if (v) return v->x;
        else return _coord_type(1.0 / n_vertices) * c;
    }


    vector3d_type repulsion_force(const vertex_type *v2, float_type f0, float_type reps) const
    {
        if (v == v2) return vector3d_type::zero;

        vector3d_type cc = centroid();
        auto dx = v2->x - cc;
        auto rdd = dx.rmod();
        float_type l = vector3d_type(
            x_max - x_min, y_max - y_min, z_max - z_min).rmod();
        if (v or rdd < l) {
            auto denom = rdd;
            auto fac = f0 * (denom * denom * denom);
            if (rdd > 2 * reps) {
                fac = 1;
                dx = vector3d_type(
                    rand_range(-reps, reps),
                    rand_range(-reps, reps),
                    rand_range(-reps, reps));
            }
            return (n_vertices * fac) * dx;
        }
        else {
            vector3d_type F_r = vector3d_type::zero;
            for (int i = 0; i < 8; i++) {
                if (subspaces[i]) 
                    F_r += subspaces[i]->repulsion_force(
                        v2, f0, reps);
            }
            return F_r;
        }
    }

    // 
    // object pool for faster allocation/deallocation
    // 
    static std::vector<spatial_octree<_coord_type> *> objpool;

    static spatial_octree *alloc(
        vertex_type *v,
        float_type x_min, float_type x_max,
        float_type y_min, float_type y_max,
        float_type z_min, float_type z_max)
    {
        if (!objpool.empty()) {
            auto obj = objpool.back();
            objpool.pop_back();
            obj->reset(v, x_min, x_max, y_min, y_max, z_min, z_max);
            return obj;
        }
        else {
            return new spatial_octree<_coord_type>(v, 
                x_min, x_max,
                y_min, y_max,
                z_min, z_max);
        }
    }

    static void dealloc(spatial_octree<_coord_type> *t) {
        objpool.push_back(t);
    }


    vertex_type *v;
    int n_vertices;
    vector3d_type c;
    float_type x, y, z;
    float_type x_min, x_max, y_min, y_max, z_min, z_max;
    spatial_octree *subspaces[8];
};


template <typename _coord_type>
std::vector<spatial_octree<_coord_type> *> spatial_octree<_coord_type>::objpool;



#endif /* _SPATIAL_OCTREE_H_ */
