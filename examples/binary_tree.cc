#include "galaster.hh"
#include <unistd.h>
#include <thread>


typedef float _float_type;
typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;


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

                graph->add_edge(left_edge);

                left_edge->color = color_type::white;
                left->vertex->color = color_type::white;
                left_edge->visible = true;
                left->vertex->visible = true;
                left_edge->color = color_type::darkgray;
                left->vertex->color = color_type::blue;
            }
            else {
                left_edge->color = color_type::white;
                left->add(k);
                
                left->vertex->color = color_type::white;
                left_edge->color = color_type::darkgray;
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

                graph->add_edge(right_edge);

                right_edge->color = color_type::white;
                right->vertex->color = color_type::white;
                right_edge->visible = true;
                right->vertex->visible = true;
                right_edge->color = color_type::darkgray;
                right->vertex->color = color_type::blue;

            }
            else {
                right_edge->color = color_type::white;
                right->add(k);
                right_edge->color = color_type::darkgray;
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
    if (g_binary_tree) {
        int v = (int) rand_range(0, 2000);
        g_binary_tree->add(v);
    }
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


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (!g_graph) return;
    if (key == 'A' and (action == GLFW_PRESS or action == GLFW_REPEAT)) {
        static std::mutex uniq_thread;
        if (uniq_thread.try_lock()) {
            new std::thread([&]() {
                    for  (int k = 0; k < 1000; k++) {
                        binary_tree_add_node();
                        usleep(5000);
                    }
                    uniq_thread.unlock();
                });
        }
    }
    else {
        galaster_key_callback(window, key, scancode, action, mods);
    }
}

int main(void)
{
    graph_type *graph = generate_binary_tree(1);

    GLFWwindow *window = galaster_init();
    if (window) {
        glfwSetKeyCallback(window, key_callback);
        galaster_run(window, graph);
    }
    delete graph;
    return 0;
}
