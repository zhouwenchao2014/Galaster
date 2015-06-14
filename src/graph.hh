#ifndef _GRAPH_H_
#define _GRAPH_H_


#include "layer.hh"
#include "rwlock.hh"


class graph_base {
public:
    virtual ~graph_base() {}
    virtual double layout(double dt) = 0;
    virtual void render(renderer method) = 0;
    virtual void randomize(void) = 0;
    virtual void bounding_box_gl(
        GLfloat &x_min, GLfloat &x_max,
        GLfloat &y_min, GLfloat &y_max,
        GLfloat &z_min, GLfloat &z_max) = 0;
};

template <typename _coord_type>
class graph : public graph_base
{
public:
    typedef _coord_type float_type;
    typedef vector3d<_coord_type> vector3d_type;
    typedef layer<_coord_type> layer_type;
    typedef vertex<_coord_type> vertex_type;
    typedef edge<_coord_type> edge_type;

    graph(int n_layers, 
        double f0, double K, double eps, double damping, double dilation)
    {
        layers.reserve(n_layers);
        layers.push_back(new finest_layer<_coord_type>(
                f0, K, eps, damping, dilation));
        for (int k = 1; k < n_layers; k++)
            layers.push_back(new layer_type(f0, K, eps, damping, dilation));

        for (int k = 1; k < n_layers; k++)
            layers[k - 1]->coarser = layers[k];

        g = layers[0];
    }
    graph(const graph &) = delete;

    ~graph(void) {
        // remove all vertices (and edges) in this graph before disposing all layers
        write_lock_guard l(lock);
        auto vs = g->vs;
        for (auto v : vs) {
            g->remove_vertex(v);
            delete v;
        }
        for (auto layer : layers) delete layer;
    }

    // 
    // interfaces exposed to other languages
    // 

    void add_vertex(vertex_type *v) {
        write_lock_guard l(lock);
        g->add_vertex(v);
    }
    void remove_vertex(vertex_type *v) {
        write_lock_guard l(lock);
        g->remove_vertex(v); 
    }
    edge_type *add_edge(edge_type *e) {
        write_lock_guard l(lock);
        return g->add_edge(e);
    }
    void remove_edge(edge_type *e) {
        write_lock_guard l(lock);
        g->remove_edge(e);
    }

    std::vector<vertex_type *> &vertex_list(void) { return g->vs; }

    virtual double layout(double dt)
    {
        read_lock_guard l(lock);
        double max_ddx = 0;
        for (auto i = layers.rbegin(); i != layers.rend(); ++i) {
            max_ddx = (*i)->layout((float_type) dt);
        }
        return max_ddx;
    }

    virtual void render(renderer method);
    virtual void render_solid(GLfloat *modelview);
    virtual void render_particle(GLfloat *modelview);

    // 
    // Randomize coordinates of vertices so that we can break the steady state and
    // redo the layout
    // 
    virtual void randomize(void)
    {
        write_lock_guard l(lock);
        for (auto v : g->vs) {
            float_type r = 5;
            v->x = vector3d_type(
                rand_range(-r, r),
                rand_range(-r, r),
                rand_range(-r, r));
            for (vertex_type *cv = v->coarser; cv != nullptr; cv = cv->coarser) {
                cv->x = v->x;
            }
            for (auto e : v->es) {
                auto e_styled = dynamic_cast<edge_styled<_coord_type> *>(e);
                if (e_styled and e_styled->spline and e_styled->vspline) {
                    e_styled->vspline->x = vector3d_type(
                        rand_range(-r, r),
                        rand_range(-r, r),
                        rand_range(-r, r));
                }
            }
        }
    }

    // 
    // Figure out the size of the bounding box of the graph
    // 
    void bounding_box(
        float_type &x_min, float_type &x_max,
        float_type &y_min, float_type &y_max,
        float_type &z_min, float_type &z_max)
    {
        write_lock_guard l(lock);
        _coord_type xmin = -10, xmax = 10;
        _coord_type ymin = -10, ymax = 10;
        _coord_type zmin = -10, zmax = 10;

        for (auto v : g->vs) {
            _coord_type x, y, z;
            if (static_cast<vertex_styled<_coord_type> *>(v)->visible) {
                v->x.coord(x, y, z);
                xmin = std::min(xmin, x);
                xmax = std::max(xmax, x);
                ymin = std::min(ymin, y);
                ymax = std::max(ymax, y);
                zmin = std::min(zmin, z);
                zmax = std::max(zmax, z);
            }
        }

        x_min = xmin; x_max = xmax;
        y_min = ymin; y_max = ymax;
        z_min = zmin; z_max = zmax;
    }

    virtual void bounding_box_gl(
        GLfloat &x_min, GLfloat &x_max,
        GLfloat &y_min, GLfloat &y_max,
        GLfloat &z_min, GLfloat &z_max)
    {
        _coord_type xmin = -10, xmax = 10;
        _coord_type ymin = -10, ymax = 10;
        _coord_type zmin = -10, zmax = 10;
        bounding_box(xmin, xmax, ymin, ymax, zmin, zmax);
        x_min = xmin; x_max = xmax;
        y_min = ymin; y_max = ymax;
        z_min = zmin; z_max = zmax;
    }


    rw_lock lock;
    std::vector<layer_type *> layers;
    layer_type *g = nullptr;
};


#endif /* _GRAPH_H_ */
