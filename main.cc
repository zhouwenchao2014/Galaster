#include "galaster.hh"
#include <unistd.h>
#include <thread>


typedef float _float_type;
typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;


#include "testgraph.hh"


#ifdef __APPLE__
void check_for_leaks(void)
{
    // launch leaks(1) to detect memory leaks
    char leakdet_cmd[128];
    sprintf(leakdet_cmd, "leaks %d", getpid());
    system(leakdet_cmd);
}
#endif

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    graph_type *graph = generate_splineedge_graph(6, 50, 2);

#ifdef __APPLE__
    atexit(check_for_leaks);
#endif

    GLFWwindow *window = galaster_init();
    if (window) galaster_run(window, graph);
    delete graph;
    return 0;
}
