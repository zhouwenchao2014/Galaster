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
int n_vertex = 200;


graph_type *generate_rand_flower(int n_layers)
{
    graph_type *graph = new graph_type(n_layers, 
        250,                    // f0
        0.02,                   // K
        0.001,                  // eps
        0.6,                    // damping
        1.2);                   // dilation

    centroid = new vertex_styled<_float_type>(0,0,0);
    centroid->shape = shape_type::sphere;
    centroid->color = color_type(0,150,255);
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

                int i_center = (int) rand_range(0, graph->g->vs.size() - 1);
                int rand_nvertex = (int) rand_range(0, n_vertex);
                auto v_center = static_cast<vertex_styled<_float_type> * >(graph->g->vs[i_center]);
                
                // increase the weight of all edges pointing out from v_center
                v_center->color = color_type(0, 255, 150);
                for (auto e : v_center->es) {
                    e->strength = 50;
                }
                
                for (int k = 0; k < rand_nvertex; k++) {
                    _float_type cx, cy, cz;
                    v_center->x.coord(cx, cy, cz);

                    auto v = new vertex_styled<_float_type>(
                        rand_range(-r + cx, r + cx),
                        rand_range(-r + cy, r + cy),
                        rand_range(-r + cz, r + cz));
                    v->shape = shape_type::sphere;
                    v->color = color_type(0, 150, 255);
                    v->size = 2;
                    v->shape_detail = 30;
                    graph->add_vertex(v);
                    vs.push_back(v);

                    auto e = new edge_styled<_float_type>(v_center, v);
                    e->blendcolor = true;
                    graph->add_edge(e);
                    usleep(10000);
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
        the_graph = generate_rand_flower(1);
        glfwSetKeyCallback(window, key_callback);
        galaster_run(window, the_graph, 0.4);
    }
    delete the_graph;
    return 0;
}
