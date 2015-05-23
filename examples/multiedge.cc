#include "galaster.hh"
#include <unistd.h>
#include <thread>


typedef float _float_type;
typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;


graph_type *the_graph;
vertex_styled<_float_type> *v0, *v1;

graph_type *generate_multiedge_graph(int n_layers)
{
    graph_type *graph = new graph_type(n_layers, 
        250,                    // f0
        0.02,                   // K
        0.001,                  // eps
        0.6,                    // damping
        1.2);                   // dilation

    _float_type r = 5;
    v0 = new vertex_styled<_float_type>(
        rand_range(-r, r),
        rand_range(-r, r),
        rand_range(-r, r));
    v0->shape = shape_type::sphere;
    v0->color = color_type::red;
    graph->add_vertex(v0);

    v1 = new vertex_styled<_float_type>(
        rand_range(-r, r),
        rand_range(-r, r),
        rand_range(-r, r));
    v1->shape = shape_type::sphere;
    v1->color = color_type(20,20,20);
    graph->add_vertex(v1);

    auto e = new edge_styled<_float_type>(v0, v1);
    graph->add_edge(e);

    return graph;
}



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (!g_graph) return;
    if (key == 'A' and action == GLFW_RELEASE) {
        auto e = new edge_styled<_float_type>(v0, v1);
        e->set_spline();
        the_graph->add_edge(e);
    }
    else if (key == 'D' and action == GLFW_RELEASE) {

    }
    else {
        galaster_key_callback(window, key, scancode, action, mods);
    }
}


int main(void)
{
    GLFWwindow *window = galaster_init();
    if (window) {
        the_graph = generate_multiedge_graph(6);
        glfwSetKeyCallback(window, key_callback);
        galaster_run(window, the_graph);
    }
    delete the_graph;
    return 0;
}
