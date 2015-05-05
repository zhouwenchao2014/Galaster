
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
        e->color = color_type(200,200,100);                             \
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
            rand_range(-r, r),
            rand_range(-r, r),
            rand_range(-r, r));
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
    new std::thread([=]() {
            int rows = membrane_rows, lines = membrane_lines;
            for (int k = 0; k < lines; k++) {
                for (int kk = 0; kk < rows - 1; kk++) {
                    graph->add_edge(new edge_styled<_float_type>(
                            graph->g->vs[rows * k + kk],
                            graph->g->vs[rows * k + kk + 1]));
                    usleep(5000);
                }
            }
            membrane_mode = 1;
        });
}

void membrane_2(graph_type *graph)
{
    new std::thread([=]() {
            int rows = membrane_rows, lines = membrane_lines;
            for (int kk = 0; kk < rows; kk++) {
                graph->add_edge(new edge_styled<_float_type>(
                        graph->g->vs[kk],
                        graph->g->vs[rows * (lines - 1) + kk]));
                usleep(100000);
            }
            membrane_mode = 2;
        });
}

void membrane_3(graph_type *graph)
{
    new std::thread([=]() {
            int rows = membrane_rows, lines = membrane_lines;
            for (int kk = 0; kk < lines; kk++) {
                graph->add_edge(new edge_styled<_float_type>(
                        graph->g->vs[rows * kk],
                        graph->g->vs[rows * kk + rows - 1]));
                usleep(50000);
            }
            membrane_mode = 3;
        });
}

void membrane_4(graph_type *graph)
{
    new std::thread([=]() {
            int rows = membrane_rows, lines = membrane_lines;
            for (int kk = 0; kk < lines; kk++) {
                graph->g->remove_edge(
                    graph->g->vs[rows * kk]->shared_edge(
                        graph->g->vs[rows * kk + rows - 1]));
                usleep(50000);
            }
            membrane_mode = 4;
        });
}

void membrane_5(graph_type *graph)
{
    new std::thread([=]() {
            int rows = membrane_rows, lines = membrane_lines;
            for (int kk = 0; kk < rows; kk++) {
                graph->g->remove_edge(
                    graph->g->vs[kk]->shared_edge(
                        graph->g->vs[rows * (lines - 1) + kk]));
                usleep(100000);
            }
            membrane_mode = 5;
        });
}

void membrane_6(graph_type *graph)
{
    new std::thread([=]() {
            int rows = membrane_rows, lines = membrane_lines;
            for (int k = 0; k < lines; k++) {
                for (int kk = 0; kk < rows - 1; kk++) {
                    graph->g->remove_edge(
                        graph->g->vs[rows * k + kk]->shared_edge(
                            graph->g->vs[rows * k + kk + 1]));
                    usleep(5000);
                }
            }
            membrane_mode = 0;
        });
}


struct binary_tree {
    binary_tree(graph_type *graph, int v, const vector3d<_float_type> &vecx)
        : graph(graph), val(v),
          vertex(nullptr), left(nullptr), right(nullptr),
          left_edge(nullptr), right_edge(nullptr)
    {
        _float_type x, y, z;
        vecx.coord(x, y, z);
        vertex = new vertex_styled<_float_type>(
            x + rand_range(-0.5, 0.5), 
            y + rand_range(0.5, 1.0), 
            z + rand_range(-0.5, 0.5));
        vertex->shape = shape_type::sphere;
        vertex->size = 3;
        vertex->visible = false;
        graph->add_vertex(vertex);
    }

    void add(int k) {
        if (val < k) {
            if (!left) {
                // add a new node here
                left = new binary_tree(graph, k, vertex->x);
                left_edge = new edge_styled<_float_type>(vertex, left->vertex);
                left_edge->oriented = true;
                left_edge->visible = false;

                // usleep(5000);
                graph->add_edge(left_edge);
                // usleep(10000);

                left_edge->color = color_type::white;
                left->vertex->color = color_type::white;
                left_edge->visible = true;
                left->vertex->visible = true;
                // usleep(10000);
                left_edge->color = color_type(100, 200, 100);
                left->vertex->color = color_type::blue;
            }
            else {
                left_edge->color = color_type::white;
                // usleep(5000);
                left->add(k);
                
                left->vertex->color = color_type::white;
                // usleep(5000);
                left_edge->color = color_type(100, 200, 100);
                left->vertex->color = color_type::blue;
            }
        }
        else {
            if (!right) {
                // add a new node here
                right = new binary_tree(graph, k, vertex->x);
                right_edge = new edge_styled<_float_type>(vertex, right->vertex);
                right_edge->oriented = true;
                right_edge->visible = false;

                // usleep(5000);
                graph->add_edge(right_edge);
                // usleep(10000);

                right_edge->color = color_type::white;
                right->vertex->color = color_type::white;
                right_edge->visible = true;
                right->vertex->visible = true;
                // usleep(10000);
                right_edge->color = color_type(100, 200, 100);
                right->vertex->color = color_type::blue;

            }
            else {
                right_edge->color = color_type::white;
                // usleep(5000);
                right->add(k);
                // usleep(5000);
                right_edge->color = color_type(100, 200, 100);
                right->vertex->color = color_type::blue;
            }
        }
    }

    graph_type *graph;
    int val;
    vertex_styled<_float_type> *vertex;
    struct binary_tree *left, *right;
    edge_styled<_float_type> *left_edge, *right_edge;

} *g_binary_tree;


void binary_tree_add_node(void)
{
    // new std::thread([=](){
            int v = (int) rand_range(0, 2000);
            g_binary_tree->add(v);
        // });
}

graph_type *generate_binary_tree(int n_layers)
{
    graph_type *graph = new graph_type(n_layers, 
        30,                     // f0
        0.02,                   // K
        0.001,                  // eps
        0.6,                    // damping
        0.8);                   // dilation

    g_binary_tree = new binary_tree(graph, 1000, vector3d<_float_type>(0, 20, 0));
    g_binary_tree->vertex->color = color_type::green;
    g_binary_tree->vertex->shape = shape_type::cube;
    g_binary_tree->vertex->size = 10;
    g_binary_tree->vertex->visible = true;

    return graph;
}
