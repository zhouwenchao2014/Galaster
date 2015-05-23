#include "galaster.hh"
#include <unistd.h>
#include <thread>


typedef float _float_type;
typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;



graph_type *generate_random_graph(int n_layers, int n_vertex, int n_edge)
{
    graph_type *graph = new graph_type(n_layers, 
        250,                    // f0
        0.02,                   // K
        0.001,                  // eps
        0.6,                    // damping
        1.2);                   // dilation

    _float_type r = 5;
    for (int k = 0; k < n_vertex; k++) {
        graph->add_vertex(new vertex_styled<_float_type>(
                rand_range(-r, r),
                rand_range(-r, r),
                rand_range(-r, r)));
    }

    for (int k = 0; k < n_vertex - 1; k++) {
        int ne = rand_range(1, n_edge);
        for (int n = 0; n < ne; n++) {
            int x2 = rand_range(k + 1, n_vertex - 1);
            graph->add_edge(
                new edge_styled<_float_type>(
                    graph->g->vs[k], graph->g->vs[x2]));
        }
    }

    return graph;
}


int main(void)
{
    graph_type *graph = generate_random_graph(6, 300, 3);

    GLFWwindow *window = galaster_init();
    if (window) galaster_run(window, graph);
    delete graph;
    return 0;
}
