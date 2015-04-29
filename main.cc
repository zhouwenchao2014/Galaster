// #ifdef __APPLE__
// #define glGenVertexArrays glGenVertexArraysAPPLE
// #define glBindVertexArrays glBindVertexArraysAPPLE
// #define glDeleteVertexArrays glDeleteVertexArraysAPPLE
// #endif

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include "graph.hh"
#include "layout.hh"
#include "verify.hh"
#include <unistd.h>


typedef double _float_type;
// typedef float _float_type;

typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;


graph_type *g_graph;

int randint(int from, int to)
{
    int n = to - from + 1;
    return rand() % n + from;
}

graph_type *generate_random_graph(int n_layers, int n_vertex, int n_edge)
{
    graph_type *graph = new graph_type(n_layers, 
        250,                    // f0
        0.02,                   // K
        0.001,                  // eps
        0.6,                    // damping
        1.2);                   // dilation

    _float_type r = 10;
    for (int k = 0; k < n_vertex; k++) {
        graph->g->add_vertex(new vertex_styled<_float_type>(
                randint(-r, r),
                randint(-r, r),
                randint(-r, r)));
    }

    for (int k = 0; k < n_vertex - 1; k++) {
        int ne = randint(1, n_edge);
        for (int n = 0; n < ne; n++) {
            int x2 = randint(k + 1, n_vertex - 1);
            graph->g->add_edge(new edge_type(graph->g->vs[k], graph->g->vs[x2]));
        }
    }

    return graph;
}

graph_type *generate_cube(int n_layers, int m)
{
    graph_type *graph = new graph_type(n_layers, 
        250,                    // f0
        0.02,                   // K
        0.001,                  // eps
        0.6,                    // damping
        1.2);                   // dilation

    _float_type r = 10;
    for (int k = 0; k < m * m * m; k++) {
        graph->g->add_vertex(new vertex_styled<_float_type>(
                randint(-r, r),
                randint(-r, r),
                randint(-r, r)));
    }

#define idx(i,j,k) (i)*m*m + (j)*m + (k)
#define addedge(a, b) graph->g->add_edge(                   \
        new edge_type(graph->g->vs[a], graph->g->vs[b]))

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


void error_callback(int error, const char *description)
{
    fprintf(stderr, "error encountered [%d]: ", error);
    fputs(description, stderr);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    printf("window: %p, key: %d, scancode: %d, action: %d, mods: %d\n",
        (void *) window, key, scancode, action, mods);
    if (key == 'R' and action == 0) {
        for (auto v : g_graph->g->vs) {
            _float_type r = 10;
            v->x = vector3d<_float_type>(
                randint(-r, r),
                randint(-r, r),
                randint(-r, r));
            for (vertex_type *cv = v->coarser; cv != nullptr; cv = cv->coarser) {
                cv->x = v->x;
            }
        }
    }
}

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    float ratio = 1.f;

    if (height > 0)
        ratio = (float) width / (float) height;

    // Setup viewport
    glViewport(0, 0, width, height);

    // Change to the projection matrix and set our viewing volume
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, ratio, 1.0, 1024.0);
}


void init_opengl(void)
{
    // Use Gouraud (smooth) shading
    glShadeModel(GL_SMOOTH);

    // Switch on the z-buffer
    glEnable(GL_DEPTH_TEST);

    glPointSize(2.0);

    // Background color is black
    glClearColor(0, 0, 0, 0);

    static GLfloat pos[4] = {5.f, 5.f, 10.f, 0.f};
    // static GLfloat red[4] = {0.8f, 0.1f, 0.f, 1.f};
    // static GLfloat green[4] = {0.f, 0.8f, 0.2f, 1.f};
    // static GLfloat blue[4] = {0.2f, 0.2f, 1.f, 1.f};

    glLightfv(GL_LIGHT0, GL_POSITION, pos);
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);

}

void draw_scene(GLFWwindow* , graph_type *graph)
{
    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // We don't want to modify the projection matrix
    glMatrixMode(GL_MODELVIEW);
    
    // Move back
    double zoom = 800;

    double radius = 4;
    int numSlices = 5;
    int numStacks = 5;

    for (auto v : graph->g->vs) {
        _float_type x, y, z;
        v->x.coord(x, y, z);
        glColor3f(0.0, 0.0, 1.0);
        glLoadIdentity();
        glTranslatef(0.0, 0.0, -zoom);
        glTranslatef(x, y, z);
        GLUquadricObj* pQuadric = gluNewQuadric();
        gluSphere(pQuadric,radius,numSlices,numStacks);

        glColor3f(1.0, 1.0, 1.0);
        for (auto e : v->es) {
            if (e->a != e->b and e->a == v) {
                _float_type x2, y2, z2;
                e->b->x.coord(x2, y2, z2);
                glLoadIdentity();
                glTranslatef(0.0, 0.0, -zoom);
                glBegin(GL_LINES);
                glVertex3f(x, y, z);
                glVertex3f(x2, y2, z2);
                glEnd();
            }
        }
    }
}


int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwSetErrorCallback(error_callback);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Galaster", NULL, NULL);
    if (!window) {
        fprintf( stderr, "Failed to open GLFW window. "
            "If you have an Intel GPU, they are not 3.3 compatible. "
            "Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    srand(time(NULL));
    int n_layer = 6;
    int n_vertex = 30;
    int n_edges = 3;
    // graph_type *graph = generate_random_graph(n_layer, n_vertex, n_edges);
    graph_type *graph = generate_cube(n_layer, 7);
    g_graph = graph;

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);

    init_opengl();

    while (!glfwWindowShouldClose(window))
    {
        for (int k = 0; k < 10; k++) {
            graph->layout(1.0);
        }
        draw_scene(window, graph);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete graph;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
