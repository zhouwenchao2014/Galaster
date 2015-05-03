#ifndef _RENDER_H_
#define _RENDER_H_


#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#include "vertex_edge.hh"


static void render_cube(int size)
{
    glBegin(GL_QUADS);
    /* Front Face */
    glNormal3f(0,0,1);
    glVertex3f( -size, -size, size );
    glVertex3f(  size, -size, size );
    glVertex3f(  size,  size, size );
    glVertex3f( -size,  size, size );
    glEnd( );

    glBegin(GL_QUADS); 
    /* Back Face */
    glNormal3f(0,0,-1);
    glVertex3f( -size, -size, -size );
    glVertex3f( -size,  size, -size );
    glVertex3f(  size,  size, -size );
    glVertex3f(  size, -size, -size );
    glEnd( );
 
    glBegin(GL_QUADS);
    /* Top Face */
    glNormal3f(0,1,0);
    glVertex3f( -size,  size, -size );
    glVertex3f( -size,  size,  size );
    glVertex3f(  size,  size,  size );
    glVertex3f(  size,  size, -size );
    glEnd( );
 
    glBegin(GL_QUADS);
    /* Bottom Face */
    glNormal3f(0,-1,0);
    glVertex3f( -size, -size, -size );
    glVertex3f(  size, -size, -size );
    glVertex3f(  size, -size,  size );
    glVertex3f( -size, -size,  size );
    glEnd( );
 
    glBegin(GL_QUADS);
    /* Right face */
    glNormal3f(1,0,0);
    glVertex3f( size, -size, -size );
    glVertex3f( size,  size, -size );
    glVertex3f( size,  size,  size );
    glVertex3f( size, -size,  size );
    glEnd( );
 
    glBegin(GL_QUADS);
    /* Left Face */
    glNormal3f(-1,0,0);
    glVertex3f( -size, -size, -size );
    glVertex3f( -size, -size,  size );
    glVertex3f( -size,  size,  size );
    glVertex3f( -size,  size, -size );
    glEnd( );
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
        case shape_type::sphere: {
            GLUquadricObj* pQuadric = gluNewQuadric();
            gluSphere(pQuadric, size, shape_detail, shape_detail);
            break;
        }

        case shape_type::cube: {
            render_cube(size);
            break;
        }

            // TODO: handle other shape types

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
    _coord_type x0, y0, z0;
    _coord_type x1, y1, z1;
    this->a->x.coord(x0, y0, z0);
    this->b->x.coord(x1, y1, z1);

    // TODO: handle arrow, stroke and width

    glColor3d(color.redd(), color.greend(), color.blued());
    glBegin(GL_LINES);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y1, z1);
    glEnd();
}



#endif /* _RENDER_H_ */
