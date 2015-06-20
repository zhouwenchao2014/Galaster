#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include "font.hh"
#include <stdio.h>


fontcache::fontcache(void)
{
    if (FT_Init_FreeType(&library) != FT_Err_Ok) {
        fprintf(stderr, "initializing freetype library failed\n");
    }
}


fontcache::glyph fontcache::glyphcache::get_glyph(int size) {
    auto it = c_glyph.find(size);
    FT_Glyph g;
    if (it == c_glyph.end()) {
        // cache miss, we have to load bitmap of this size
        printf("loading glyph: %c, %d\n", (char) ch, size);
        g = glyph_cache->load_glyph(ch, size);
        if (g != nullptr && 
            (FT_Glyph_To_Bitmap(&g, FT_RENDER_MODE_NORMAL, NULL, false) 
                == FT_Err_Ok)) {
            GLuint glyph_tex_id;
            glGenTextures(1, &glyph_tex_id);
            glBindTexture(GL_TEXTURE_2D, glyph_tex_id);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            FT_Bitmap *bitmap = &((FT_BitmapGlyph) g)->bitmap;
            if (!bitmap) return glyph();
            glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, bitmap->width, bitmap->rows,
                0, GL_LUMINANCE, GL_UNSIGNED_BYTE, bitmap->buffer);

            auto ret = glyph(g, glyph_tex_id);
            c_glyph[size] = ret;
            return ret;
        }
        else {
            return glyph();
        }
    }
    else {
        return it->second;
    }
}


fontcache::glyphcache *fontcache::facecache::get_char(wchar_t ch)
{
    auto it = c_face.find(ch);
    glyphcache *bc;
    if (it == c_face.end()) {
        bc = new glyphcache(this, ch);
        c_face[ch] = bc;
    }
    else {
        bc = it->second;
    }
    return bc;
}

FT_Glyph fontcache::facecache::load_glyph(wchar_t ch, size_t size)
{
    FT_Error err;
    if ((err = FT_Set_Pixel_Sizes(face, 0, size))) {
        return nullptr;
    }
    int glyph_index = FT_Get_Char_Index(face, ch);
    FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
    FT_Glyph ret = nullptr;
    FT_Get_Glyph(face->glyph, &ret);
    return ret;
}


fontcache::facecache *fontcache::get_face(const std::string &fontpath)
{
    auto it = c_font.find(fontpath);
    facecache *gc = nullptr;
    if (it == c_font.end()) {
        FT_Face face;
        FT_Error err = FT_New_Face(library, fontpath.c_str(), 0, &face);
        if (err == FT_Err_Unknown_File_Format) {
            fprintf(stderr, "unknown font file format: %s\n", fontpath.c_str());
            face = nullptr;
        }
        else if (err) {
            fprintf(stderr, "loading font %s failed\n", fontpath.c_str());
            face = nullptr;
        }
        if (face) {
            gc = new facecache(this, face);
            c_font[fontpath] = gc;
        }
    }
    else {
        gc = it->second;
    }
    return gc;
}


fontcache::glyph fontcache::glyph_of(const std::string &fontpath, wchar_t ch, size_t size)
{
    facecache *fc = get_face(fontpath);
    if (fc) {
        glyphcache *gc = fc->get_char(ch);
        if (gc) {
            return gc->get_glyph(size);
        }
    }
    return glyph();
}
