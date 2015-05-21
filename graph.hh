#ifndef _GRAPH_H_
#define _GRAPH_H_


#include "layer.hh"
#include <mutex>


template <typename _coord_type>
class graph
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
        std::lock_guard<std::mutex> l(lock);
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
        std::lock_guard<std::mutex> l(lock);
        g->add_vertex(v);
    }
    void remove_vertex(vertex_type *v) {
        std::lock_guard<std::mutex> l(lock);
        g->remove_vertex(v); 
    }
    edge_type *add_edge(edge_type *e) {
        std::lock_guard<std::mutex> l(lock);
        return g->add_edge(e);
    }
    void remove_edge(edge_type *e) {
        std::lock_guard<std::mutex> l(lock);
        g->remove_edge(e);
    }

    std::vector<vertex_type *> &vertex_list(void) { return g->vs; }

    void layout(float_type dt)
    {
        std::lock_guard<std::mutex> l(lock);
        for (auto i = layers.rbegin(); i != layers.rend(); ++i) {
            (*i)->layout(dt);
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
        std::lock_guard<std::mutex> l(lock);
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

    std::mutex lock;
    std::vector<layer_type *> layers;
    layer_type *g = nullptr;
};


#endif /* _GRAPH_H_ */
