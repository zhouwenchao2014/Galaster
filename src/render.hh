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


struct _vertexarrayelement {
    GLfloat s, t;
    GLuint  rgba;
    GLfloat x, y, z;
};

#define P_TEX_WIDTH  8    // Particle texture dimensions
#define P_TEX_HEIGHT 8

// Texture object IDs
static GLuint particle_tex_id;

// Particle texture (a simple spot)
const unsigned char particle_texture[ P_TEX_WIDTH * P_TEX_HEIGHT ] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x11, 0x22, 0x22, 0x11, 0x00, 0x00,
    0x00, 0x11, 0x33, 0x88, 0x77, 0x33, 0x11, 0x00,
    0x00, 0x22, 0x88, 0xff, 0xee, 0x77, 0x22, 0x00,
    0x00, 0x22, 0x77, 0xee, 0xff, 0x88, 0x22, 0x00,
    0x00, 0x11, 0x33, 0x77, 0x88, 0x33, 0x11, 0x00,
    0x00, 0x00, 0x11, 0x33, 0x22, 0x11, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Define tokens for GL_EXT_separate_specular_color if not already defined
#ifndef GL_EXT_separate_specular_color
#define GL_LIGHT_MODEL_COLOR_CONTROL_EXT  0x81F8
#define GL_SINGLE_COLOR_EXT               0x81F9
#define GL_SEPARATE_SPECULAR_COLOR_EXT    0x81FA
#endif // GL_EXT_separate_specular_color

inline void init_particle_system(void)
{
    glGenTextures(1, &particle_tex_id);
    glBindTexture(GL_TEXTURE_2D, particle_tex_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, P_TEX_WIDTH, P_TEX_HEIGHT,
        0, GL_LUMINANCE, GL_UNSIGNED_BYTE, particle_texture);
    if (glfwExtensionSupported("GL_EXT_separate_specular_color"))
    {
        glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL_EXT,
            GL_SEPARATE_SPECULAR_COLOR_EXT);
    }
    // Set filled polygon mode as default (not wireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

// 
// Render this graph using particle system, this renderer is much faster than solid
// rendering system and performs well when visualizing large graphs (N > 5000)
// 
template <typename _coord_type>
void graph<_coord_type>::render_particle(GLfloat *modelview)
{
    glDisable(GL_DEPTH_TEST);
    for (auto v : g->vs) {
        glLoadMatrixf(modelview);
        for (auto e : v->es) {
            if (e->a == v) {
                glLoadMatrixf(modelview);
                static_cast<edge_styled<_coord_type> *>(e)->render();
            }
        }
    }

#define PARTICLE_SIZE 10.0f
    static int vertarray_len = 0;
    static _vertexarrayelement *vertex_arr = nullptr;
    static bool particle_initialized = false;
    if (!particle_initialized) {
        particle_initialized = true;
        init_particle_system();
    }
    glLoadMatrixf(modelview);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, particle_tex_id);

    // precalculate for billboarding
    GLfloat qlx = (-PARTICLE_SIZE / 2) * (modelview[0] + modelview[1]);
    GLfloat qly = (-PARTICLE_SIZE / 2) * (modelview[4] + modelview[5]);
    GLfloat qlz = (-PARTICLE_SIZE / 2) * (modelview[8] + modelview[9]);
    GLfloat qrx = (PARTICLE_SIZE / 2) * (modelview[0] - modelview[1]);
    GLfloat qry = (PARTICLE_SIZE / 2) * (modelview[4] - modelview[5]);
    GLfloat qrz = (PARTICLE_SIZE / 2) * (modelview[8] - modelview[9]);

    // construct vertex array
    size_t n_vertices = g->vs.size();
    if (!vertex_arr or vertarray_len < n_vertices * 4) {
        vertarray_len = n_vertices * 4;
        delete vertex_arr;
        vertex_arr = new _vertexarrayelement[vertarray_len];
    }
    glInterleavedArrays(GL_T2F_C4UB_V3F, 0, vertex_arr);
    _vertexarrayelement *vptr = vertex_arr;
    for (size_t i = 0; i < n_vertices; i++) {
        GLfloat x, y, z;
        auto v = static_cast<vertex_styled<_coord_type> *>(g->vs[i]);
        v->x.coord(x, y, z);
            
        GLuint rgba = 0xAAAAAAFF;
        ((GLubyte *) &rgba)[0] = (GLubyte) (v->color.redd() * 255);
        ((GLubyte *) &rgba)[1] = (GLubyte) (v->color.greend() * 255);
        ((GLubyte *) &rgba)[2] = (GLubyte) (v->color.blued() * 255);
        GLfloat vqlx = qlx * v->size;
        GLfloat vqly = qly * v->size;
        GLfloat vqlz = qlz * v->size;
        GLfloat vqrx = qrx * v->size;
        GLfloat vqry = qry * v->size;
        GLfloat vqrz = qrz * v->size;
        
        // Lower left corner
        vptr->s    = 0.f;
        vptr->t    = 0.f;
        vptr->rgba = rgba;
        vptr->x    = x + vqlx;
        vptr->y    = y + vqly;
        vptr->z    = z + vqlz;
        vptr ++;

        // Lower right corner
        vptr->s    = 1.f;
        vptr->t    = 0.f;
        vptr->rgba = rgba;
        vptr->x    = x + vqrx;
        vptr->y    = y + vqry;
        vptr->z    = z + vqrz;
        vptr ++;

        // Upper right corner
        vptr->s    = 1.f;
        vptr->t    = 1.f;
        vptr->rgba = rgba;
        vptr->x    = x - vqlx;
        vptr->y    = y - vqly;
        vptr->z    = z - vqlz;
        vptr ++;

        // Upper left corner
        vptr->s    = 0.f;
        vptr->t    = 1.f;
        vptr->rgba = rgba;
        vptr->x    = x - vqrx;
        vptr->y    = y - vqry;
        vptr->z    = z - vqrz;
        vptr ++;
    }

    glDrawArrays(GL_QUADS, 0, 4 * n_vertices);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}


// 
// Render the graph using traditional solid body rendering system, the render effect
// is identical with ubigraph, but way slower than particle system
// 
template <typename _coord_type>
void graph<_coord_type>::render_solid(GLfloat *modelview)
{
    glEnable(GL_DEPTH_TEST);
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
}


// 
// Render graph using OpenGL by rendering each individual vertices and edges. Note
// that the modelview matrix will not be affected after calling this function
// 
template <typename _coord_type>
void graph<_coord_type>::render(renderer method)
{
    GLfloat modelview[4 * 4];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    lock.read_lock();
    switch (method) {
        case renderer::solid:
            render_solid(modelview);
            break;

        case renderer::particle:
            render_particle(modelview);
            break;

        default:
            break;
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
        // glColor4d(.0, 1.0, 0.6, 0.3);
        glVertex3f(x0, y0, z0);
        // glColor4d(color.redd(), color.greend(), color.blued(), 0.3);
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
    else if (vspline) {
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
