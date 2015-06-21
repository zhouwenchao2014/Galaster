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
int n_vertex = 10;


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
    centroid->font_family = "/Library/Fonts/华文细黑.ttf";
    centroid->font_size = 30;
    centroid->label = L"♂";
    centroid->size = 15;
    graph->add_vertex(centroid);

    std::vector<vertex_styled<_float_type> *> vs;
    _float_type r = 0;

    {
        auto v = new vertex_styled<_float_type>(
            rand_range(-r, r),
            rand_range(-r, r),
            rand_range(-r, r));
        v->shape = shape_type::sphere;
        v->color = color_type(
            rand_range(0,0.5), rand_range(0,0.5), rand_range(0,0.5));
        v->size = 7;
        v->shape_detail = 30;
        v->font_family = "/Library/Fonts/华文细黑.ttf";
        v->font_size = 20;
        v->label = L"奥";
        graph->add_vertex(v);
        vs.push_back(v);

        auto e = new edge_styled<_float_type>(centroid, v);
        e->set_spline();
        e->color = color_type(0.2, 0.2, 0.2);
        graph->add_edge(e);
    }

    {
        auto v = new vertex_styled<_float_type>(
            rand_range(-r, r),
            rand_range(-r, r),
            rand_range(-r, r));
        v->shape = shape_type::sphere;
        v->color = color_type(
            rand_range(0,0.5), rand_range(0,0.5), rand_range(0,0.5));
        v->size = 7;
        v->shape_detail = 30;
        v->font_family = "/Library/Fonts/华文细黑.ttf";
        v->font_size = 20;
        v->label = L"义";
        graph->add_vertex(v);
        vs.push_back(v);

        auto e = new edge_styled<_float_type>(centroid, v);
        e->set_spline();
        e->color = color_type(0.2, 0.2, 0.2);
        graph->add_edge(e);
    }


    {
        auto v = new vertex_styled<_float_type>(
            rand_range(-r, r),
            rand_range(-r, r),
            rand_range(-r, r));
        v->shape = shape_type::sphere;
        v->color = color_type(
            rand_range(0,0.5), rand_range(0,0.5), rand_range(0,0.5));
        v->size = 7;
        v->shape_detail = 30;
        v->font_family = "/Library/Fonts/华文细黑.ttf";
        v->font_size = 20;
        v->label = L"很";
        graph->add_vertex(v);
        vs.push_back(v);

        auto e = new edge_styled<_float_type>(centroid, v);
        e->set_spline();
        e->color = color_type(0.2, 0.2, 0.2);
        graph->add_edge(e);
    }


    {
        auto v = new vertex_styled<_float_type>(
            rand_range(-r, r),
            rand_range(-r, r),
            rand_range(-r, r));
        v->shape = shape_type::sphere;
        v->color = color_type(
            rand_range(0,0.5), rand_range(0,0.5), rand_range(0,0.5));
        v->size = 7;
        v->shape_detail = 30;
        v->font_family = "/Library/Fonts/华文细黑.ttf";
        v->font_size = 20;
        v->label = L"爽";
        graph->add_vertex(v);
        vs.push_back(v);

        auto e = new edge_styled<_float_type>(centroid, v);
        e->set_spline();
        e->color = color_type(0.2, 0.2, 0.2);
        graph->add_edge(e);
    }

    {
        auto v = new vertex_styled<_float_type>(
            rand_range(-r, r),
            rand_range(-r, r),
            rand_range(-r, r));
        v->shape = shape_type::sphere;
        v->color = color_type(
            rand_range(0,0.5), rand_range(0,0.5), rand_range(0,0.5));
        v->size = 7;
        v->shape_detail = 30;
        v->font_family = "/Library/Fonts/Baoli.ttc";
        v->font_size = 20;
        v->label = L"fuck♂you";
        graph->add_vertex(v);
        vs.push_back(v);

        auto e = new edge_styled<_float_type>(centroid, v);
        e->set_spline();
        e->color = color_type(0.2, 0.2, 0.2);
        graph->add_edge(e);
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
