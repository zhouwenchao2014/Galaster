
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
                randint(-r, r),
                randint(-r, r),
                randint(-r, r)));
    }

    for (int k = 0; k < n_vertex - 1; k++) {
        int ne = randint(1, n_edge);
        for (int n = 0; n < ne; n++) {
            int x2 = randint(k + 1, n_vertex - 1);
            graph->add_edge(new edge_type(
                    graph->g->vs[k], graph->g->vs[x2],
                    false, true));
        }
    }

    return graph;
}

graph_type *generate_cube(int n_layers, int m)
{
    graph_type *graph = new graph_type(n_layers, 
        30,                     // f0
        0.02,                   // K
        0.001,                  // eps
        0.6,                    // damping
        0.8);                   // dilation

    _float_type r = 5;
    for (int k = 0; k < m * m * m; k++) {
        // graph->add_vertex(new vertex_styled<_float_type>(
        //         randint(-r, r),
        //         randint(-r, r),
        //         randint(-r, r)));
        auto v = new vertex_styled<_float_type>(
                randint(-r, r),
                randint(-r, r),
                randint(-r, r));
        v->shape = shape_type::cube;
        v->size = 3;
        v->color = color_type(0,50,255);
        graph->add_vertex(v);
    }

#define idx(i,j,k) (i)*m*m + (j)*m + (k)
#define addedge(a, b) {                                                 \
        auto e = new edge_styled<_float_type>(graph->g->vs[a], graph->g->vs[b]); \
        e->color = color_type(100,100,100);                             \
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

static int membrane_mode;
static int membrane_rows, membrane_lines;

graph_type *generate_membrane(int n_layers, int rows, int lines)
{
    graph_type *graph = new graph_type(n_layers, 
        30,                     // f0
        0.02,                   // K
        0.001,                  // eps
        0.6,                    // damping
        0.8);                   // dilation

        // 250,                    // f0
        // 0.02,                   // K
        // 0.001,                  // eps
        // 0.6,                    // damping
        // 1.2);                   // dilation

    _float_type r = 5;

    int n_vertex = rows * lines;
    for (int k = 0; k < n_vertex; k++) {
        auto v = new vertex_styled<_float_type>(
            randint(-r, r),
            randint(-r, r),
            randint(-r, r));
        v->shape = shape_type::sphere;
        v->size = 2;
        graph->add_vertex(v);
    }

    for (int kk = 0; kk < rows - 1; kk++) {
        graph->add_edge(new edge_styled<_float_type>(
                graph->g->vs[kk],
                graph->g->vs[kk + 1]));
    }

    for (int kk = rows - 1; kk > 0; kk--) {
        graph->add_edge(new edge_styled<_float_type>(
                graph->g->vs[n_vertex - kk],
                graph->g->vs[n_vertex - kk - 1]));
    }

    for (int k = 0; k < lines - 1; k++) {
        for (int kk = 0; kk < rows; kk++) {
            graph->add_edge(new edge_styled<_float_type>(
                graph->g->vs[rows * k + kk],
                graph->g->vs[rows * (k + 1) + kk]));
        }
    }

    membrane_mode = 0;
    membrane_rows = rows;
    membrane_lines = lines;
    return graph;
}

void membrane_1(graph_type *graph)
{
    int rows = membrane_rows, lines = membrane_lines;
    for (int k = 0; k < lines; k++) {
        for (int kk = 0; kk < rows - 1; kk++) {
            graph->add_edge(new edge_styled<_float_type>(
                    graph->g->vs[rows * k + kk],
                    graph->g->vs[rows * k + kk + 1]));
        }
    }
    membrane_mode = 1;
}

void membrane_2(graph_type *graph)
{
    int rows = membrane_rows, lines = membrane_lines;
    for (int kk = 0; kk < rows; kk++) {
        graph->add_edge(new edge_styled<_float_type>(
                graph->g->vs[kk],
                graph->g->vs[rows * (lines - 1) + kk]));
    }
    membrane_mode = 2;
}

void membrane_3(graph_type *graph)
{
    int rows = membrane_rows, lines = membrane_lines;
    for (int kk = 0; kk < lines; kk++) {
        graph->add_edge(new edge_styled<_float_type>(
                graph->g->vs[rows * kk],
                graph->g->vs[rows * kk + rows - 1]));
    }
    membrane_mode = 3;
}

void membrane_4(graph_type *graph)
{
    int rows = membrane_rows, lines = membrane_lines;
    for (int kk = 0; kk < lines; kk++) {
        graph->g->remove_edge(
            graph->g->vs[rows * kk]->shared_edge(
                graph->g->vs[rows * kk + rows - 1]));
    }
    membrane_mode = 4;
}

void membrane_5(graph_type *graph)
{
    int rows = membrane_rows, lines = membrane_lines;
    for (int kk = 0; kk < rows; kk++) {
        graph->g->remove_edge(
            graph->g->vs[kk]->shared_edge(
                graph->g->vs[rows * (lines - 1) + kk]));
    }
    membrane_mode = 5;
}

void membrane_6(graph_type *graph)
{
    int rows = membrane_rows, lines = membrane_lines;
    for (int k = 0; k < lines; k++) {
        for (int kk = 0; kk < rows - 1; kk++) {
            graph->g->remove_edge(
                graph->g->vs[rows * k + kk]->shared_edge(
                    graph->g->vs[rows * k + kk + 1]));
        }
    }
    membrane_mode = 0;
}
