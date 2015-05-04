#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include "graph.hh"
#include "layout.hh"
#include "verify.hh"
#include "render.hh"
#include <unistd.h>


typedef double _float_type;
// typedef float _float_type;

typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;


graph_type *g_graph;

double zoom = 450;
double theta = 0;
double phi = 0;


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
        graph->add_vertex(new vertex_styled<_float_type>(
                randint(-r, r),
                randint(-r, r),
                randint(-r, r)));
    }

    for (int k = 0; k < n_vertex - 1; k++) {
        int ne = randint(1, n_edge);
        for (int n = 0; n < ne; n++) {
            int x2 = randint(k + 1, n_vertex - 1);
            graph->add_edge(new edge_type(
                    graph->g->vs[k], graph->g->vs[x2],
                    false, true));
        }
    }

    return graph;
}

graph_type *generate_cube(int n_layers, int m)
{
    graph_type *graph = new graph_type(n_layers, 
        30,                     // f0
        0.02,                   // K
        0.001,                  // eps
        0.6,                    // damping
        0.8);                   // dilation

    _float_type r = 10;
    for (int k = 0; k < m * m * m; k++) {
        // graph->add_vertex(new vertex_styled<_float_type>(
        //         randint(-r, r),
        //         randint(-r, r),
        //         randint(-r, r)));
        auto v = new vertex_styled<_float_type>(
                randint(-r, r),
                randint(-r, r),
                randint(-r, r));
        v->shape = shape_type::cube;
        v->size = 3;
        v->color = color_type(0,50,255);
        graph->add_vertex(v);
    }

#define idx(i,j,k) (i)*m*m + (j)*m + (k)
#define addedge(a, b) {                                                 \
        auto e = new edge_styled<_float_type>(graph->g->vs[a], graph->g->vs[b]); \
        e->color = color_type(50,50,50);                             \
        graph->add_edge(e);                                          \
    }

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

    _float_type r = 10;

    int n_vertex = rows * lines;
    for (int k = 0; k < n_vertex; k++) {
        auto v = new vertex_styled<_float_type>(
            randint(-r, r),
            randint(-r, r),
            randint(-r, r));
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
    int rows = membrane_rows, lines = membrane_lines;
    for (int k = 0; k < lines; k++) {
        for (int kk = 0; kk < rows - 1; kk++) {
            graph->add_edge(new edge_styled<_float_type>(
                    graph->g->vs[rows * k + kk],
                    graph->g->vs[rows * k + kk + 1]));
        }
    }
    membrane_mode = 1;
}

void membrane_2(graph_type *graph)
{
    int rows = membrane_rows, lines = membrane_lines;
    for (int kk = 0; kk < rows; kk++) {
        graph->add_edge(new edge_styled<_float_type>(
                graph->g->vs[kk],
                graph->g->vs[rows * (lines - 1) + kk]));
    }
    membrane_mode = 2;
}

void membrane_3(graph_type *graph)
{
    int rows = membrane_rows, lines = membrane_lines;
    for (int kk = 0; kk < lines; kk++) {
        graph->add_edge(new edge_styled<_float_type>(
                graph->g->vs[rows * kk],
                graph->g->vs[rows * kk + rows - 1]));
    }
    membrane_mode = 3;
}

void membrane_4(graph_type *graph)
{
    int rows = membrane_rows, lines = membrane_lines;
    for (int kk = 0; kk < lines; kk++) {
        graph->g->remove_edge(
            graph->g->vs[rows * kk]->shared_edge(
                graph->g->vs[rows * kk + rows - 1]));
    }
    membrane_mode = 4;
}

void membrane_5(graph_type *graph)
{
    int rows = membrane_rows, lines = membrane_lines;
    for (int kk = 0; kk < rows; kk++) {
        graph->g->remove_edge(
            graph->g->vs[kk]->shared_edge(
                graph->g->vs[rows * (lines - 1) + kk]));
    }
    membrane_mode = 5;
}

void membrane_6(graph_type *graph)
{
    int rows = membrane_rows, lines = membrane_lines;
    for (int k = 0; k < lines; k++) {
        for (int kk = 0; kk < rows - 1; kk++) {
            graph->g->remove_edge(
                graph->g->vs[rows * k + kk]->shared_edge(
                    graph->g->vs[rows * k + kk + 1]));
        }
    }
    membrane_mode = 0;
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
    else if (key == 'N' and action == 0) {
        if (membrane_mode == 0) membrane_1(g_graph);
        else if (membrane_mode == 1) membrane_2(g_graph);
        else if (membrane_mode == 2) membrane_3(g_graph);
        else if (membrane_mode == 3) membrane_4(g_graph);
        else if (membrane_mode == 4) membrane_5(g_graph);
        else if (membrane_mode == 5) membrane_6(g_graph);
    }
    else if (key == 'Q' and action == 0) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (key == 'W' and (action == 1 or action == 2)) {
        zoom -= 10;
    }
    else if (key == 'S' and (action == 1 or action == 2)) {
        zoom += 10;
    }
    else if (key == 262 and (action == 1 or action == 2)) {
        theta += 1;
    }
    else if (key == 263 and (action == 1 or action == 2)) {
        theta -= 1;
    }
    else if (key == 264 and (action == 1 or action == 2)) {
        phi += 1;
    }
    else if (key == 265 and (action == 1 or action == 2)) {
        phi -= 1;
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
    // gluPerspective(60.0, ratio, 1.0, 1024.0);
    gluPerspective(60.0, ratio, 1.0, 10240.0);
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

    // Light
    const GLfloat light_position0[4] = {0.0f, 8.0f, 8.0f, 1.0f};
    const GLfloat light_position1[4] = {0.0f, -8.0f, -8.0f, 1.0f};
    const GLfloat light_diffuse[4]  = {1.0f, 1.0f, 1.0f, 1.0f};
    const GLfloat light_specular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    const GLfloat light_ambient[4]  = {0.5f, 0.5f, 0.5f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

    // Enable OpenGL features
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_LINE_STIPPLE);
    glEnable(GL_DEPTH_TEST);
}


void draw_scene(GLFWwindow* , graph_type *graph)
{
    // Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // We don't want to modify the projection matrix
    glMatrixMode(GL_MODELVIEW);
    
    for (auto v : graph->g->vs) {
        glLoadIdentity();
        gluLookAt(
            0, 0, -zoom,
            0, 0, 0,
            0, 1, 0);
        glRotatef(theta, 0, 1, 0);
        glRotatef(phi, 1, 0, 0);
        static_cast<vertex_styled<_float_type> *>(v)->render();

        for (auto e : v->es) {
            if (e->a != e->b and e->a == v) {
                glLoadIdentity();
                gluLookAt(
                    0, 0, -zoom,
                    0, 0, 0,
                    0, 1, 0);
                glRotatef(theta, 0, 1, 0);
                glRotatef(phi, 1, 0, 0);
                static_cast<edge_styled<_float_type> *>(e)->render();
            }
        }
    }
}



#ifdef __APPLE__
void check_for_leaks(void)
{
    // launch leaks(1) to detect memory leaks
    char leakdet_cmd[128];
    sprintf(leakdet_cmd, "leaks %d", getpid());
    system(leakdet_cmd);
}
#endif

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;
#ifdef __APPLE__
    atexit(check_for_leaks);
#endif

    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwSetErrorCallback(error_callback);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Galaster", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to open GLFW window.\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    srand(time(NULL));
    int n_layer = 6;
    // int n_vertex = 300;
    // int n_edges = 3;
    // graph_type *graph = generate_random_graph(n_layer, n_vertex, n_edges);
    graph_type *graph = generate_cube(n_layer, 8);
    // graph_type *graph = generate_membrane(n_layer, 6, 20);
    g_graph = graph;

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);

    init_opengl();

    // Initialize timer
    double t, dt_total, t_old;
    t_old = glfwGetTime() - 0.01;

    while (!glfwWindowShouldClose(window))
    {
        // layout scene
        do {
            graph->layout(1.0);
            t = glfwGetTime();
            dt_total = t - t_old;
        } while (dt_total < 0.02);
        t_old = t;
        
        // render scene
        draw_scene(window, graph);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    delete graph;
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
