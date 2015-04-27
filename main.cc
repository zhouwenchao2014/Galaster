#include "multilevel.hh"


typedef edge<float> edge_type;
typedef vertex<float> vertex_type;

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    vertex_type *a = new vertex_type(1,2,3);
    vertex_type *b = new vertex_type(4,5,6);
    
    edge_type *e0 = new edge_type(a, b, false);
    edge_type *e1 = new edge_type(a, b, false);
    e0 = e0->connect();
    e1 = e1->connect();
    e0->disconnect();
    e1->disconnect();

    // delete e0;
    // delete e1;
    delete a;
    delete b;
    
    return 0;
}
