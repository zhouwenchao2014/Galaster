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
    centroid->color = color_type(20,20,20);
    centroid->shape_detail = 50;
    centroid->size = 30;
    graph->add_vertex(centroid);

    std::vector<vertex_styled<_float_type> *> vs;
    _float_type r = 5;

    for (int k = 0; k < n_vertex; k++) {
        auto v = new vertex_styled<_float_type>(
            rand_range(-r, r),
            rand_range(-r, r),
            rand_range(-r, r));
        v->shape = shape_type::sphere;
        v->color = color_type(
            rand_range(0,0.5), rand_range(0,0.5), rand_range(0,0.5));
        v->size = 7;
        v->shape_detail = 30;
        graph->add_vertex(v);
        vs.push_back(v);

        auto e = new edge_styled<_float_type>(centroid, v);
        e->set_spline();
        e->color = color_type(0.2, 0.2, 0.2);
        graph->add_edge(e);
    }

    for (int i = 0; i < n_vertex; i++) {
        for (int j = i + 1; j < n_vertex; j++) {
            if (rand_range(0,1) < 2.0 / n_vertex) {
                auto e = new edge_styled<_float_type>(
                    vs[i],
                    vs[j]);
                e->stroke = stroke_type::dashed;
                e->set_spline();
                graph->add_edge(e);
                e->color = color_type(
                    rand_range(0.2,0.3), rand_range(0.2,0.3), rand_range(0.2,0.3));
            }
        }
    }

    return graph;
}


int main(void)
{
    GLFWwindow *window = galaster_init();
    if (window) {
        the_graph = generate_splineorama_graph(1);
        galaster_run(window, the_graph, 0.2);
    }
    delete the_graph;
    return 0;
}
