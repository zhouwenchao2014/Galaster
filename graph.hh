#ifndef _GRAPH_H_
#define _GRAPH_H_


#include "layer.hh"


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
        for (int k = 0; k < n_layers; k++)
            layers.push_back(new layer_type(f0, K, eps, damping, dilation));

        for (int k = 1; k < n_layers; k++)
            layers[k - 1]->coarser = layers[k];

        g = layers[0];
    }
    graph(const graph &) = delete;

    ~graph(void) {
        // remove all vertices (and edges) in this graph before disposing all layers
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

    void add_vertex(vertex_type *v) { g->add_vertex(v); }
    void remove_vertex(vertex_type *v) { g->remove_vertex(v); }
    edge_type *add_edge(edge_type *e) { return g->add_edge(e); }
    void remove_edge(edge_type *e) { g->remove_edge(e); }
    std::vector<vertex_type *> &vertex_list(void) { return g->vs; }

    void layout(float_type dt)
    {
        for (auto i = layers.rbegin(); i != layers.rend(); ++i) {
            (*i)->layout(dt);
        }
    }

    std::vector<layer_type *> layers;
    layer_type *g = nullptr;
};


#endif /* _GRAPH_H_ */
