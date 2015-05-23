#include "galaster.hh"
#include <unistd.h>
#include <thread>


typedef float _float_type;
typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;


graph_type *the_graph;
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


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (!g_graph) return;
    if (key == 'N' and action == GLFW_RELEASE) {
        if (membrane_mode == 0) membrane_1(the_graph);
        else if (membrane_mode == 1) membrane_2(the_graph);
        else if (membrane_mode == 2) membrane_3(the_graph);
        else if (membrane_mode == 3) membrane_4(the_graph);
        else if (membrane_mode == 4) membrane_5(the_graph);
        else if (membrane_mode == 5) membrane_6(the_graph);
    }
    else {
        galaster_key_callback(window, key, scancode, action, mods);
    }
}


int main(void)
{
    GLFWwindow *window = galaster_init();
    if (window) {
        the_graph = generate_membrane(6, 8, 30);
        glfwSetKeyCallback(window, key_callback);
        galaster_run(window, the_graph);
    }
    delete the_graph;
    return 0;
}
