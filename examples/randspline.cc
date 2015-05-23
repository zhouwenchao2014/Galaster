#include "galaster.hh"
#include <unistd.h>
#include <thread>


typedef float _float_type;
typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;


graph_type *generate_splineedge_graph(int n_layers, int n_vertex, int n_edge)
{
    graph_type *graph = new graph_type(n_layers, 
        250,                    // f0
        0.02,                   // K
        0.001,                  // eps
        0.6,                    // damping
        1.2);                   // dilation

    _float_type r = 5;
    for (int k = 0; k < n_vertex; k++) {
        auto v = new vertex_styled<_float_type>(
            rand_range(-r, r),
            rand_range(-r, r),
            rand_range(-r, r));
        v->shape = shape_type::sphere;
        v->color = color_type(
            rand_range(0,1), rand_range(0,1), rand_range(0,1));
        v->size = 5;
        graph->add_vertex(v);
    }

    for (int k = 0; k < n_vertex; k++) {
        for (int n = 0; n < n_edge; n++) {
            int x2 = rand_range(0, n_vertex - 1);
            auto e = new edge_styled<_float_type>(
                graph->g->vs[k], graph->g->vs[x2]);
            e->set_spline();
            e->arrow = true;
            e->arrow_position = rand_range(0, 1);
            e->arrow_radius = 3;
            e->arrow_length = 6;
            e->arrow_reverse = (rand_range(0, 1) < 0.5);
            graph->add_edge(e);
        }
    }

    return graph;
}


int main(void)
{
    graph_type *graph = generate_splineedge_graph(6, 50, 2);

    GLFWwindow *window = galaster_init();
    if (window) galaster_run(window, graph);
    delete graph;
    return 0;
}
