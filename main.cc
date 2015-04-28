#include "graph.hh"


typedef graph<float> graph_type;
typedef layer<float> layer_type;
typedef edge<float> edge_type;
typedef vertex<float> vertex_type;


int randint(int from, int to)
{
    int n = to - from + 1;
    return rand() % n + from;
}

void dump_graphviz(const layer_type *layer, const std::string &filename)
{
    FILE *fp = fopen(filename.c_str(), "w+");
    fprintf(fp, "digraph G {\n");
    fprintf(fp, "node [ shape = \"circle\" ];\n");
    for (auto v : layer->vs) {
        fprintf(fp, "  v%d [ label = \"v%d\" ];\n", v->id, v->id);
        for (auto e : v->es) {
            if (e->a == v) {
                if (e->a != e->b and 
                    e->a->coarser == e->b->coarser and
                    e->a->coarser != nullptr) {
                    fprintf(fp, "  v%d -> v%d [ penwidth = 4 ];\n", 
                        e->a->id, e->b->id);
                }
                else {
                    fprintf(fp, "  v%d -> v%d [ penwidth = 1 ];\n", 
                        e->a->id, e->b->id);
                }
            }
        }
    }
    fprintf(fp, "}\n");
    fclose(fp);
}


void random_test(int n_layers, int n_vertex, int epochs)
{
    graph_type *graph = new graph_type(n_layers, 0,0,0,0,0);
    
    for (int k = 0; k < n_vertex; k++) {
        graph->g->add_vertex(new vertex_type(0,0,0));
    }

    for (int k = 0; k < epochs; k++)
    {
        int x1 = randint(0, n_vertex - 1);
        int x2 = randint(0, n_vertex - 1);
        int op = randint(0, 1);
        if (op == 1) {
            edge_type *e = new edge_type(graph->g->vs[x1], graph->g->vs[x2], false);
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

        if (!graph->verify_integrity()) {
            printf("!!! INTEGRITY CHECK FAILED !!!\n");
            exit(-1);
        }
        if (!graph->verify_redundancy()) {
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
            // printf("[REMOVE EDGE (%d -> %d)\n", e->a->id, e->b->id
            graph->g->remove_edge(e);
            if (!graph->verify_integrity()) {
                printf("!!! INTEGRITY CHECK FAILED !!!\n");
                exit(-1);
            }
            if (!graph->verify_redundancy()) {
                printf("!!! REDUNDANCY CHECK FAILED !!!\n");
                exit(-1);
            }
        }
        graph->g->remove_vertex(v);
        delete v;
    }

    delete graph;
}


int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

	// layer_type *layer = new layer_type(0,0,0,0,0);
	

    // vertex_type *a = new vertex_type(1,2,3);
    // vertex_type *b = new vertex_type(4,5,6);
    
    // edge_type *e0 = new edge_type(a, b, false);
    // edge_type *e1 = new edge_type(a, b, false);

	// layer->add_vertex(a);
	// layer->add_vertex(b);

    // // e0 = e0->connect();
    // // e1 = e1->connect();
    // // e0->disconnect();
    // // e1->disconnect();

    // e0 = layer->add_edge(e0);
    // e1 = layer->add_edge(e1);
    // layer->remove_edge(e0);
    // layer->remove_edge(e1);

	// layer->remove_vertex(a);
	// layer->remove_vertex(b);


    // // delete e0;
    // // delete e1;
    // delete a;
    // delete b;
	// delete layer;

    srand(time(NULL));
    int n_layer = 6;
    int n_vertex = 100;
    int n_edges = 3;
    random_test(n_layer, n_vertex, n_vertex * n_edges * 2);
    
    return 0;
}
