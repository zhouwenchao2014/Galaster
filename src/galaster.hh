#ifndef _GALASTER_H_
#define _GALASTER_H_

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include "graph.hh"
#include "layout.hh"
#include "render.hh"


extern graph_base *g_graph;

GLFWwindow *galaster_init(void);
int galaster_run(GLFWwindow *window, graph_base *graph, double dt = 1.0);

void galaster_key_callback(
    GLFWwindow* window, 
    int key, int scancode, int action, int mods);


#endif /* _GALASTER_H_ */
