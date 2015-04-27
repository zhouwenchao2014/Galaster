#include "layer.hh"


typedef layer<float> layer_type;
typedef edge<float> edge_type;
typedef vertex<float> vertex_type;

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

	layer_type *layer = new layer_type(0,0,0,0,0);
	

    vertex_type *a = new vertex_type(1,2,3);
    vertex_type *b = new vertex_type(4,5,6);
    
    edge_type *e0 = new edge_type(a, b, false);
    edge_type *e1 = new edge_type(a, b, false);

	layer->add_vertex(a);
	layer->add_vertex(b);

    e0 = e0->connect();
    e1 = e1->connect();
    e0->disconnect();
    e1->disconnect();

	layer->remove_vertex(a);
	layer->remove_vertex(b);


    // delete e0;
    // delete e1;
    delete a;
    delete b;
	delete layer;
    
    return 0;
}
