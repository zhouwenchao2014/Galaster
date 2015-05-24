#ifndef _RENDER_H_
#define _RENDER_H_


#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include "vertex_edge.hh"
#include <iostream>


void ownglEvalMesh1f (
    /* glEvalMesh1() */ GLenum mode, GLint i1, GLint i2,
    /* glMapGrid1f() */ GLint nu, GLfloat u1_, GLfloat u2_,
    /* glMap1f */ GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint
    uorder, GLfloat *points);

void ownglEvalCoord1f(
    GLint ustride, GLint uorder, const GLfloat *points, GLfloat t, 
    GLfloat &x, GLfloat &y, GLfloat &z);


void glutSolidSphere(double radius, GLint slices, GLint stacks);
void glutSolidCube(double size);
void glutSolidTorus(double dInnerRadius, double dOuterRadius, GLint nSides, GLint nRings );
void glutSolidCone( double base, double height, GLint slices, GLint stacks);
void glutSolidOctahedron(void);
void glutSolidDodecahedron(void);
void glutSolidIcosahedron(void);
void glutSolidRhombicDodecahedron(void);



// 
// Render graph using OpenGL by rendering each individual vertices and edges. Note
// that the modelview matrix will not be affected after calling this function
// 
template <typename _coord_type>
void graph<_coord_type>::render(void)
{
    GLfloat modelview[4 * 4];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    lock.lock();
    for (auto v : g->vs) {
        glLoadMatrixf(modelview);
        static_cast<vertex_styled<_coord_type> *>(v)->render();
        for (auto e : v->es) {
            if (e->a == v) {
                glLoadMatrixf(modelview);
                static_cast<edge_styled<_coord_type> *>(e)->render();
            }
        }
    }
    lock.unlock();
    glLoadMatrixf(modelview);
}


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

    vector3d_type arrow_dir(vector3d_type::zero);
    _coord_type ax = 0, ay = 0, az = 0;

    if (!spline) {
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        glVertex3f(x0, y0, z0);
        glVertex3f(x1, y1, z1);
        glEnd();
        glEnable(GL_LIGHTING);

        // calculate arrow position and direction
        if (arrow) {
            vector3d_type dvertex = this->b->x - this->a->x;
            arrow_dir = dvertex.normalized();
            (this->a->x + dvertex * (_coord_type) arrow_position).coord(ax, ay, az);
        }
    }
    else {
        _coord_type x0,y0,z0, x1,y1,z1, x2,y2,z2;
        this->a->x.coord(x0, y0, z0);
        vspline->x.coord(x1, y1, z1);
        this->b->x.coord(x2, y2, z2);

        if (this->a != this->b) {
            GLfloat ctrl_pts[3][3] = {
                {(GLfloat) x0, (GLfloat) y0, (GLfloat) z0},
                {(GLfloat) x1, (GLfloat) y1, (GLfloat) z1},
                {(GLfloat) x2, (GLfloat) y2, (GLfloat) z2}
            };
            glDisable(GL_LIGHTING);
            glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 3, &ctrl_pts[0][0]);
            glMapGrid1f(10, 0.0, 1.0);
            glEvalMesh1(GL_LINE, 0, 10);
            glEnable(GL_LIGHTING);

            // calculate arrow position and direction
            if (arrow) {
                GLfloat axf, ayf, azf;
                GLfloat x1, y1, z1;
                ownglEvalCoord1f(3, 3, &ctrl_pts[0][0], arrow_position, axf, ayf, azf);
                ownglEvalCoord1f(3, 3, &ctrl_pts[0][0], arrow_position + 0.1, x1, y1, z1);
                ax = (_coord_type) axf, ay = (_coord_type) ayf, az = (_coord_type) azf;
                arrow_dir = vector3d_type(x1 - ax, y1 - ay, z1 - az).normalized();
            }
        }
        else {
            _coord_type x0,y0,z0, x1,y1,z1, dx, dy, dz;
            this->a->x.coord(x0, y0, z0);
            vspline->x.coord(x1, y1, z1);
            (vspline->x - this->a->x).coord(dx, dy, dz);
            _coord_type k = 50;
            _coord_type l = k / sqrt(dx * dx + dz * dz);
            dx *= l; dz *= l;
            
            GLfloat ctrl_pts[4][3] = {
                {(GLfloat) x0, (GLfloat) y0, (GLfloat) z0},
                {(GLfloat) (x0 + dx), (GLfloat) (y0 - k), (GLfloat) (z0 + dz)},
                {(GLfloat) (x0 + dx), (GLfloat) (y0 + k), (GLfloat) (z0 + dz)},
                {(GLfloat) x0, (GLfloat) y0, (GLfloat) z0},
            };
            glDisable(GL_LIGHTING);
            ownglEvalMesh1f (
                GL_LINE, 0, 50,
                50, 0.0, 1.0,
                GL_MAP1_VERTEX_4, 0.0, 1.0, 3, 4, &ctrl_pts[0][0]);
            glEnable(GL_LIGHTING);

            // calculate arrow position and direction
            if (arrow) {
                GLfloat axf, ayf, azf;
                GLfloat x1, y1, z1;
                ownglEvalCoord1f(3, 4, &ctrl_pts[0][0], arrow_position, axf, ayf, azf);
                ownglEvalCoord1f(3, 4, &ctrl_pts[0][0], arrow_position + 0.1, x1, y1, z1);
                ax = (_coord_type) axf, ay = (_coord_type) ayf, az = (_coord_type) azf;
                arrow_dir = vector3d_type(x1 - ax, y1 - ay, z1 - az).normalized();
            }
        }
    }

    render_arrow(arrow_dir, ax, ay, az);
}


template <typename _coord_type>
void edge_styled<_coord_type>::render_arrow(
    const vector3d_type &arrow_dir, GLfloat ax, GLfloat ay, GLfloat az) const
{
    if (arrow) {
        glTranslatef(ax, ay, az);
        auto zvec_dir = vector3d_type(0, 0, 1);
        _coord_type rot_angle = acos(arrow_dir.dot(zvec_dir));
        if (fabs(rot_angle) > 1e-6) {
            _coord_type rx, ry, rz;
            vector3d_type rot_axis = arrow_dir.cross(zvec_dir).normalized();
            rot_axis.coord(rx, ry, rz);
            glRotatef(-rot_angle * 180 / M_PI, rx, ry, rz);
        }

        if (arrow_reverse) glRotatef(180, 0, 1, 0);
        glutSolidCone(arrow_radius, arrow_length, 20, 20);
    }
}



#endif /* _RENDER_H_ */
