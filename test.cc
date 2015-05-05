#include "graph.hh"
#include "layout.hh"
#include "verify.hh"
#include <unistd.h>

typedef double _float_type;
// typedef float _float_type;

#define DBG_VERIFY_INTEGRITY

typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;


int randint(int from, int to)
{
    int n = to - from + 1;
    return rand() % n + from;
}


void layout_test(int n_layers, int n_vertex, int n_edge)
{
    graph_type *graph = new graph_type(n_layers, 
        250,                    // f0
        0.02,                   // K
        0.001,                  // eps
        0.6,                    // damping
        1.2);                   // dilation
    
    for (int k = 0; k < n_vertex; k++) {
        graph->g->add_vertex(new vertex_styled<_float_type>(
                randint(-100, 100),
                randint(-100, 100),
                randint(-100, 100)));
    }

    for (int k = 0; k < n_vertex - 1; k++) {
        int ne = randint(1, n_edge);
        for (int n = 0; n < ne; n++) {
            int x2 = randint(k + 1, n_vertex - 1);
            graph->g->add_edge(new edge_type(graph->g->vs[k], graph->g->vs[x2]));
        }
    }

    printf("\nLAYOUT BEGIN\n");
    clock_t start = clock();
    for (int k = 0; k < 10000; k++) {
        graph->g->layout(1.0);
        _float_type maxdelta = 0.0;
        for (auto v : graph->g->vs) {
            _float_type mod_delta = v->delta.mod();
            maxdelta = std::max(maxdelta, mod_delta);
        }
        printf("iter #%d, maxdelta: %f\n", k, maxdelta);
    }
    clock_t end = clock() - start;
    printf("LAYOUT END, time elapsed: %lu ms\n\n", (end - start) * 1000 / CLOCKS_PER_SEC);

    delete graph;
}

void random_test(int n_layers, int n_vertex, int epochs)
{
    graph_type *graph = new graph_type(n_layers, 
        250,                    // f0
        0.02,                   // K
        0.001,                  // eps
        0.6,                    // damping
        1.2);                   // dilation
    
    for (int k = 0; k < n_vertex; k++) {
        graph->g->add_vertex(new vertex_styled<_float_type>(
                randint(-100, 100),
                randint(-100, 100),
                randint(-100, 100)));
    }

    for (int k = 0; k < epochs; k++)
    {
        int x1 = randint(0, n_vertex - 1);
        int x2 = randint(0, n_vertex - 1);
        int op = randint(0, 1);
        if (op == 1) {
            edge_type *e = new edge_styled<_float_type>(graph->g->vs[x1], graph->g->vs[x2]);
            printf("[ADD EDGE (%d)]: %d -> %d\n", k, e->a->id, e->b->id);
            graph->g->add_edge(e);
        }
        else {
            edge_type *e = graph->g->vs[x1]->shared_edge(graph->g->vs[x2]);
            // if (graph->g->vs[x1]->es.empty()) continue;
            // edge_type *e = graph->g->vs[x1]->es[0];
            if (e) {
                printf("[REMOVE EDGE (%d)]: %d -> %d\n", k, e->a->id, e->b->id);
                graph->g->remove_edge(e);
            }
        }

#ifdef DBG_VERIFY_INTEGRITY
        if (!verify_integrity(graph)) {
            printf("!!! INTEGRITY CHECK FAILED !!!\n");
            exit(-1);
        }
        if (!verify_redundancy(graph)) {
            printf("!!! REDUNDANCY CHECK FAILED !!!\n");
            exit(-1);
        }
#endif
    }

    int i_layer = 0;
    for (auto layer : graph->layers) {
        char filename[4096];
        sprintf(filename, "layer_%d.dot", i_layer);
        dump_graphviz(layer, filename);
        i_layer += 1;
    }

    // dump number of vertices in each layer
    i_layer = 0;
    for (auto layer: graph->layers) {
        printf("# of vertices on layer %d: %lu\n",
            i_layer++, layer->vs.size());
    }

    auto vs = graph->g->vs;
    for (auto v : vs) {
        auto es = v->es;
        for (auto e : es) {
            // printf("[REMOVE EDGE (%d -> %d)\n", e->a->id, e->b->id);
            graph->g->remove_edge(e);

#ifdef DBG_VERIFY_INTEGRITY
            if (!verify_integrity(graph)) {
                printf("!!! INTEGRITY CHECK FAILED !!!\n");
                exit(-1);
            }
            if (!verify_redundancy(graph)) {
                printf("!!! REDUNDANCY CHECK FAILED !!!\n");
                exit(-1);
            }
#endif
        }
        graph->g->remove_vertex(v);
        delete v;
    }

    delete graph;
}


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

#ifdef __APPLE__
    atexit(check_for_leaks);
#endif

    srand(time(NULL));
    int n_layer = 6;
    int n_vertex = 1000;
    int n_edges = 3;
    random_test(n_layer, n_vertex, n_vertex * n_edges * 2);
    // layout_test(n_layer, n_vertex, n_edges);

    return 0;
}
