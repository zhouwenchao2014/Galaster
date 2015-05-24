#include "galaster.hh"
#include <unistd.h>
#include <thread>


typedef float _float_type;
typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;



graph_type *generate_cube(int n_layers, int m, bool spline)
{
    graph_type *graph = new graph_type(n_layers, 
        30,                     // f0
        0.02,                   // K
        0.001,                  // eps
        0.6,                    // damping
        0.8);                   // dilation

    _float_type r = 5;
    for (int k = 0; k < m * m * m; k++) {
        auto v = new vertex_styled<_float_type>(
                rand_range(-r, r),
                rand_range(-r, r),
                rand_range(-r, r));
        v->shape = shape_type::cube;
        v->size = 3;
        v->color = color_type(0,50,255);
        graph->add_vertex(v);
    }

#define idx(i,j,k) (i)*m*m + (j)*m + (k)

#define addedge(a, b) {                                                 \
        auto e = new edge_styled<_float_type>(graph->g->vs[a], graph->g->vs[b]); \
        e->set_spline(spline);                                          \
        graph->add_edge(e);                                             \
    }

    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            for (int k = 0; k < m; k++) {
                if (i > 0) addedge(idx(i,j,k), idx(i-1,j,k));
                if (j > 0) addedge(idx(i,j,k), idx(i,j-1,k));
                if (k > 0) addedge(idx(i,j,k), idx(i,j,k-1));
            }
        }
    }

#undef addedge
#undef idx

    return graph;
}


int main(void)
{
    graph_type *graph = generate_cube(6, 8, false);

    GLFWwindow *window = galaster_init();
    if (window) galaster_run(window, graph);
    delete graph;
    return 0;
}
