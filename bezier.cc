#ifndef _BEZIER_H_
#define _BEZIER_H_

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>
#include <cmath>
#include <assert.h>


inline int factorial (int n)
{
    static int fact[] = {
        1,                      // 0!
        1,                      // 1!
        2,                      // 2!
        6,                      // 3!
        24,                     // 4!
        120,                    // 5!
        720,                    // 6!
        5040,                   // 7!
        40320,                  // 8!
        362880,                 // 9!
        3628800,                // 10!
        39916800,               // 11!
        479001600               // 12!
    };
    assert(n >= 0 and n <= (int) (sizeof(fact)/sizeof(int)));
    return fact[n];
}

inline int combination (int n, int p)
{
    if (p == 0)
        return 1;
    else
        return (factorial(n)/(factorial(p)*factorial(n-p)));
}

/*
 * B(i): The ith Bernstein Polynomial of degree n
 */
inline double B (int n, int i, GLfloat u)
{
    return (combination(n,i)*pow(u,i)*pow(1.0-u, n-i));
}

inline double p1(GLfloat u, int n, GLint ustride, const GLfloat *R, int
    offset)
{
    GLint i;
    GLfloat result = 0.0;
    for (i = 0;i <= n; i++) {
        result += B(n,i,u) * R[i*ustride + offset];
    }
    return result;
}

/*
 * Function: ownglEvalMesh1f()
 *
 * Parameters: The same (in order) that glEvalMesh1(), glMapGrid1f(), glMap1f()
 *
 * Notes: Doesn't generate normals.
 *        Only supports GL_MAP1_VERTEX_3.
 */
void ownglEvalMesh1f (
    /* glEvalMesh1() */ GLenum mode, GLint i1, GLint i2,
    /* glMapGrid1f() */ GLint nu, GLfloat u1_, GLfloat u2_,
    /* glMap1f */ GLenum target, GLfloat u1, GLfloat u2, 
    GLint ustride, GLint uorder, GLfloat *points)
{
    GLint i;
    GLfloat temp1;
    const GLfloat du = (u2_ - u1_)/(GLfloat)nu;
    const GLint n = uorder-1;

    (void) target;

    switch (mode)
    {
        case GL_LINE:
            mode=GL_LINE_STRIP;
            break;
        case GL_POINT:
            mode=GL_POINTS;
            break;
        default:
            return;
    }
    glBegin (mode);
    for (i = i1; i <= i2; i += 1)
    {
        temp1 = ((i*du+u1_)-u1)/(u2-u1);
        glVertex3f (
            p1( temp1, n, ustride, points, 0),
            p1( temp1, n, ustride, points, 1),
            p1( temp1, n, ustride, points, 2) );
    }
    glEnd( );
}


#endif /* _BEZIER_H_ */
