#include "graph.hh"
#include "verify.hh"
#include <unistd.h>

typedef float _float_type;

typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;


int randint(int from, int to)
{
    int n = to - from + 1;
    return rand() % n + from;
}


void random_test(int n_layers, int n_vertex, int epochs)
{
    graph_type *graph = new graph_type(n_layers, 0,0,0,0,0);
    
    for (int k = 0; k < n_vertex; k++) {
        graph->g->add_vertex(new vertex_styled<_float_type>(0,0,0));
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
            // edge_type *e = graph->g->vs[x1]->shared_edge(graph->g->vs[x2]);
            if (graph->g->vs[x1]->es.empty()) continue;
            edge_type *e = graph->g->vs[x1]->es[0];
            if (e) {
                printf("[REMOVE EDGE (%d)]: %d -> %d\n", k, e->a->id, e->b->id);
                graph->g->remove_edge(e);
            }
        }

        if (!verify_integrity(graph)) {
            printf("!!! INTEGRITY CHECK FAILED !!!\n");
            exit(-1);
        }
        if (!verify_redundancy(graph)) {
            printf("!!! REDUNDANCY CHECK FAILED !!!\n");
            exit(-1);
        }
    }

    int i_layer = 0;
    for (auto layer : graph->layers) {
        char filename[4096];
        sprintf(filename, "layer_%d.dot", i_layer);
        dump_graphviz(layer, filename);
        i_layer += 1;
    }

    auto vs = graph->g->vs;
    for (auto v : vs) {
        auto es = v->es;
        for (auto e : es) {
            // printf("[REMOVE EDGE (%d -> %d)\n", e->a->id, e->b->id);
            graph->g->remove_edge(e);
            if (!verify_integrity(graph)) {
                printf("!!! INTEGRITY CHECK FAILED !!!\n");
                exit(-1);
            }
            if (!verify_redundancy(graph)) {
                printf("!!! REDUNDANCY CHECK FAILED !!!\n");
                exit(-1);
            }
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
    int n_layer = 8;
    int n_vertex = 100;
    int n_edges = 3;
    random_test(n_layer, n_vertex, n_vertex * n_edges * 2);

    return 0;
}
