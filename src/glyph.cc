#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include "font.hh"


struct _vertexarrayelement {
    GLfloat s, t;
    GLfloat x, y, z;
};

static inline void qvertex(
    struct _vertexarrayelement *v, 
    float s, float t, float x, float y) {
	v->s = s;
	v->t = t;
	v->x = x;
	v->y = y;
    v->z = 0;
}

void render_glyph_gl(
    const std::string &fontpath, const wchar_t *ch, size_t size, 
    GLfloat cx, GLfloat cy, GLfloat cz)
{
    static fontcache *fc = new fontcache;
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_TEXTURE_2D);

    // billboarding
    float modelview[16]; int i,j;
    glPushMatrix();
    glTranslatef(cx, cy, cz);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
    for( i=0; i<3; i++ ) 
        for( j=0; j<3; j++ ) 
            if (i == j) modelview[i*4+j] = 1.0; 
            else modelview[i*4+j] = 0.0;
    glLoadMatrixf(modelview); 
    
    GLfloat xpos = 0;
    for ( ; *ch != 0; ch++) {
        // load glyph as texture
        auto glyph = fc->glyph_of(fontpath, *ch, size);
        FT_Glyph gly = glyph.gly;
        FT_Bitmap *bitmap = &((FT_BitmapGlyph) gly)->bitmap;
        glBindTexture(GL_TEXTURE_2D, glyph.tex);

        // draw this glyph
        static _vertexarrayelement vertex_arr[6];
        GLfloat font_scale = 0.25;
        GLfloat w = bitmap->width * font_scale;
        GLfloat h = bitmap->rows * font_scale;
        glInterleavedArrays(GL_T2F_V3F, 0, vertex_arr);
        qvertex(&vertex_arr[0], 0, 1, xpos, 0);
        qvertex(&vertex_arr[1], 1, 1, xpos + w, 0);
        qvertex(&vertex_arr[2], 1, 0, xpos + w, h);
        qvertex(&vertex_arr[3], 0, 1, xpos, 0);
        qvertex(&vertex_arr[4], 1, 0, xpos + w, h);
        qvertex(&vertex_arr[5], 0, 0, xpos, h);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        xpos += (gly->advance.x >> 16) * font_scale;
    }

    glPopMatrix();

    // done
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glPopAttrib();
}
