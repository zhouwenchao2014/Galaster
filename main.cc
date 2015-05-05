#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include "graph.hh"
#include "layout.hh"
#include "verify.hh"
#include "render.hh"
#include <unistd.h>
#include <thread>


// typedef double _float_type;
typedef float _float_type;

typedef graph<_float_type> graph_type;
typedef layer<_float_type> layer_type;
typedef edge<_float_type> edge_type;
typedef vertex<_float_type> vertex_type;


graph_type *g_graph;


struct camera_view_type
{
    double zoom = 450;
    double theta = 0;
    double phi = 0;
    double d_theta = 0;
    double d_phi = 0;
    double d_zoom = 0;
    double zoom_factor = 1;
    double d_zoom_factor = 0;
    double step = 0.1;

    void on_wayin(void) {
        zoom_factor -= 500;
    }

    void on_wayout(void) {
        zoom_factor += 500;
    }

    void on_forward(void) {
        if (d_zoom_factor > 0) d_zoom_factor = 0;
        else d_zoom_factor -= step;
    }

    void on_backward(void) {
        if (d_zoom_factor < 0) d_zoom_factor = 0;
        else d_zoom_factor += step;
    }

    void on_left(void) {
        if (d_theta > 0) d_theta = 0;
        else d_theta -= step;
    }

    void on_right(void) {
        if (d_theta < 0) d_theta = 0;
        else d_theta += step;
    }

    void on_up(void) {
        if (d_phi > 0) d_phi = 0;
        else d_phi -= step;
    }

    void on_down(void) {
        if (d_phi < 0) d_phi = 0;
        else d_phi += step;
    }

    void update_view(double new_zoom) {
        d_zoom = ((new_zoom + zoom_factor) - zoom) * 0.02;
        zoom += d_zoom;
        theta += d_theta;
        phi += d_phi;
        zoom_factor += d_zoom_factor;
    }
    
} camera_view;


double rand_range(double from, double to)
{
    double n = to - from + 1;
    return rand() * n / RAND_MAX + from;
}

#include "testgraph.hh"


void error_callback(int error, const char *description)
{
    fprintf(stderr, "error encountered [%d]: ", error);
    fputs(description, stderr);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    printf("window: %p, key: %d, scancode: %d, action: %d, mods: %d\n",
        (void *) window, key, scancode, action, mods);
    if (key == 'R' and action == GLFW_RELEASE) {
        for (auto v : g_graph->g->vs) {
            _float_type r = 5;
            v->x = vector3d<_float_type>(
                rand_range(-r, r),
                rand_range(-r, r),
                rand_range(-r, r));
            for (vertex_type *cv = v->coarser; cv != nullptr; cv = cv->coarser) {
                cv->x = v->x;
            }
        }
    }
    else if (key == 'N' and action == GLFW_RELEASE) {
        if (membrane_mode == 0) membrane_1(g_graph);
        else if (membrane_mode == 1) membrane_2(g_graph);
        else if (membrane_mode == 2) membrane_3(g_graph);
        else if (membrane_mode == 3) membrane_4(g_graph);
        else if (membrane_mode == 4) membrane_5(g_graph);
        else if (membrane_mode == 5) membrane_6(g_graph);
    }
    else if (key == 'A' and (action == GLFW_PRESS or action == GLFW_REPEAT)) {
        new std::thread([=]() {
                for  (int k = 0; k < 1000; k++) {
                    binary_tree_add_node();
                    usleep(5000);
                }
            });
    }
    else if (key == 'Q' and action == 0) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (key == 'F' and (action == GLFW_PRESS or action == GLFW_REPEAT)) {
        camera_view.on_wayin();
    }
    else if (key == 'B' and (action == GLFW_PRESS or action == GLFW_REPEAT)) {
        camera_view.on_wayout();
    }
    else if (key == 'W' and (action == GLFW_PRESS or action == GLFW_REPEAT)) {
        camera_view.on_forward();
    }
    else if (key == 'S' and (action == GLFW_PRESS or action == GLFW_REPEAT)) {
        camera_view.on_backward();
    }
    else if (key == 262 and (action == GLFW_PRESS or action == GLFW_REPEAT)) {
        camera_view.on_right();
    }
    else if (key == 263 and (action == GLFW_PRESS or action == GLFW_REPEAT)) {
        camera_view.on_left();
    }
    else if (key == 264 and (action == GLFW_PRESS or action == GLFW_REPEAT)) {
        camera_view.on_down();
    }
    else if (key == 265 and (action == GLFW_PRESS or action == GLFW_REPEAT)) {
        camera_view.on_up();
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
    const GLfloat light_specular[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    const GLfloat light_ambient[4]  = {0.2f, 0.2f, 0.2f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT0, GL_AMBIENT_AND_DIFFUSE, light_ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv(GL_LIGHT1, GL_AMBIENT_AND_DIFFUSE, light_ambient);
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

    _float_type x_min, x_max, y_min, y_max, z_min, z_max;
    graph->bounding_box(x_min, x_max, y_min, y_max, z_min, z_max);
    _float_type l = std::max(
        std::max(x_max - x_min, y_max - y_min), 
        z_max - z_min);
    camera_view.update_view(l);

    GLfloat modelview[4 * 4];
    glLoadIdentity();
    gluLookAt(
        0, 0, -camera_view.zoom,
        0, 0, 0,
        0, 1, 0);
    glRotatef(camera_view.theta, 0, 1, 0);
    glRotatef(camera_view.phi, 1, 0, 0);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

    graph->lock.lock();
    for (auto v : graph->g->vs) {
        glLoadMatrixf(modelview);
        static_cast<vertex_styled<_float_type> *>(v)->render();
        for (auto e : v->es) {
            if (e->a != e->b and e->a == v) {
                glLoadMatrixf(modelview);
                static_cast<edge_styled<_float_type> *>(e)->render();
            }
        }
    }
    graph->lock.unlock();
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
    // graph_type *graph = generate_membrane(n_layer, 8, 30);
    // graph_type *graph = generate_binary_tree(n_layer);
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
