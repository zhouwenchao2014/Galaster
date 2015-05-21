#ifndef _RENDER_H_
#define _RENDER_H_


#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include "vertex_edge.hh"


void glutSolidSphere(double radius, GLint slices, GLint stacks);
void glutSolidCube(double size);
void glutSolidTorus(double dInnerRadius, double dOuterRadius, GLint nSides, GLint nRings );
void glutSolidCone( double base, double height, GLint slices, GLint stacks);
void glutSolidOctahedron(void);
void glutSolidDodecahedron(void);
void glutSolidIcosahedron(void);
void glutSolidRhombicDodecahedron(void);


// Render this vertex using OpenGL
template <typename _coord_type>
void vertex_styled<_coord_type>::render(void) const
{
    if (!visible) return;
    _coord_type x, y, z;
    this->x.coord(x, y, z);
    glTranslatef(x, y, z);
    glColor3d(color.redd(), color.greend(), color.blued());
    switch (shape) {
        case shape_type::sphere:
            glutSolidSphere(size, shape_detail, shape_detail);
            break;

        case shape_type::cube:
            glutSolidCube(size);
            break;

        case shape_type::torus:
            glutSolidTorus(size * 0.5, size * 1.0, shape_detail, shape_detail);
            break;

        case shape_type::cone:
            glutSolidCone(size, size, shape_detail, shape_detail);
            break;

        case shape_type::octahedron:
            glScalef(size, size, size);
            glutSolidOctahedron();
            break;

        case shape_type::dodecahedron:
            glScalef(size, size, size);
            glutSolidDodecahedron();
            break;

        case shape_type::icosahedron:
            glScalef(size, size, size);
            glutSolidIcosahedron();
            break;

        case shape_type::rhombicdodecahedron:
            glScalef(size, size, size);
            glutSolidRhombicDodecahedron();
            break;

        default:
            fprintf(stderr, "unknown shape type specified: %d\n", (int) shape);
            break;
    }
}


// Render this edge using OpenGL
template <typename _coord_type>
void edge_styled<_coord_type>::render(void) const
{
    if (!visible) return;
    if (!spline and this->a == this->b) return;

    _coord_type x0, y0, z0;
    _coord_type x1, y1, z1;
    this->a->x.coord(x0, y0, z0);
    this->b->x.coord(x1, y1, z1);

    // TODO: handle arrow and spline
    switch (stroke) {
        case stroke_type::solid:  glLineStipple(1, 0xFFFF); break;
        case stroke_type::dotted: glLineStipple(2, 0xAAAA); break;
        case stroke_type::dashed: glLineStipple(1, 0x00FF); break;
        case stroke_type::none:   glLineStipple(1, 0x0000); break;
        default:
            fprintf(stderr, "unknown stroke type specified: %d\n", (int) stroke);
    }
    glLineWidth(width);
    glColor3d(color.redd(), color.greend(), color.blued());
    glBegin(GL_LINES);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y1, z1);
    glEnd();
}



#endif /* _RENDER_H_ */
