#include "galaster.hh"
#include <unistd.h>
#include <thread>


typedef float _float_type;
typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;


int main(void)
{
    graph_type *graph = generate_splineedge_graph(6, 50, 2);

    GLFWwindow *window = galaster_init();
    if (window) galaster_run(window, graph);
    delete graph;
    return 0;
}
