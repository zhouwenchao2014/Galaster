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
        // TODO: we might need to remove all edges and vertices in g
        for (auto layer : layers) delete layer;
    }

    bool verify_integrity(void)
    {
        for (auto layer : layers) {
            if (!layer->verify_integrity()) return false;
        }
        return true;
    }

    bool verify_redundancy(void)
    {
        for (auto layer : layers) {
            if (!layer->verify_redundancy()) return false;
        }
        return true;
    }

    std::vector<layer_type *> layers;
    layer_type *g = nullptr;
};


#endif /* _GRAPH_H_ */
