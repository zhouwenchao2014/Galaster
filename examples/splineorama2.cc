#include "galaster.hh"
#include <unistd.h>
#include <thread>


typedef float _float_type;
typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;


graph_type *the_graph;
vertex_styled<_float_type> *centroid;
int n_vertex = 40;


graph_type *generate_splineorama_graph(int n_layers)
{
    graph_type *graph = new graph_type(n_layers, 
        250,                    // f0
        0.02,                   // K
        0.001,                  // eps
        0.6,                    // damping
        1.2);                   // dilation

    centroid = new vertex_styled<_float_type>(0,0,0);
    centroid->shape = shape_type::sphere;
    centroid->color = color_type(10,10,10);
    centroid->shape_detail = 50;
    centroid->size = 30;
    graph->add_vertex(centroid);
    
    return graph;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (!g_graph) return;
    if (key == 'A' and action == GLFW_RELEASE) {
        new std::thread([&]() {
                std::vector<vertex_styled<_float_type> *> vs;
                graph_type *graph = the_graph;
                _float_type r = 5;

                for (int k = 0; k < n_vertex; k++) {
                    auto v = new vertex_styled<_float_type>(
                        rand_range(-r, r),
                        rand_range(-r, r),
                        rand_range(-r, r));
                    v->shape = shape_type::sphere;
                    v->color = color_type(
                        rand_range(0,1), rand_range(0,1), rand_range(0,1));
                    v->size = 7;
                    v->shape_detail = 30;
                    graph->add_vertex(v);
                    vs.push_back(v);

                    auto e = new edge_styled<_float_type>(centroid, v);
                    // e->set_spline();
                    e->strength = 0.1;
                    e->color = color_type(0.15, 0.15, 0.15);
                    graph->add_edge(e);
                    usleep(10000);
                }

                for (int i = 0; i < n_vertex; i++) {
                    for (int j = i + 1; j < n_vertex; j++) {
                        if (rand_range(0,1) < 4.0 / n_vertex) {
                            auto e = new edge_styled<_float_type>(
                                vs[i],
                                vs[j]);
                            e->stroke = stroke_type::dashed;
                            // e->set_spline();
                            e->spline = true;
                            graph->add_edge(e);
                            e->color = color_type(
                                rand_range(0.3,0.5), rand_range(0.3,0.5), rand_range(0.3,0.5));
                            usleep(10000);
                        }
                    }
                }
            });
    }
    if (key == 'D' and action == GLFW_RELEASE) {
        graph_type *graph = the_graph;
        auto vs = graph->g->vs;
        for (auto it = vs.rbegin(); it != vs.rend(); it++) {
            if (*it != centroid) {
                graph->remove_vertex(*it);
            }
        }
    }
    else {
        galaster_key_callback(window, key, scancode, action, mods);
    }
}


int main(void)
{
    GLFWwindow *window = galaster_init();
    if (window) {
        the_graph = generate_splineorama_graph(6);
        glfwSetKeyCallback(window, key_callback);
        galaster_run(window, the_graph, 1.0);
    }
    delete the_graph;
    return 0;
}
