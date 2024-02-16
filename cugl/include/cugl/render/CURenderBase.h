//
//  CUBase.h
//  Cornell University Game Library (CUGL)
//
//  This header includes the necessary includes to use OpenGL in CUGL.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 12/29/22

#ifndef __CU_RENDER_BASE_H__
#define __CU_RENDER_BASE_H__
#include <cugl/base/CUBase.h>

// OpenGL support
/** Support for standard OpenGL   */
#define CU_GL_OPENGL   0
/** Support for standard OpenGLES */
#define CU_GL_OPENGLES 1

// Load the libraries and define the platform
#if defined (__IPHONEOS__)
    #include <OpenGLES/ES3/gl.h>
    #include <OpenGLES/ES3/glext.h>
    /** The current OpenGL platform */
    #define CU_GL_PLATFORM   CU_GL_OPENGLES
#elif defined (__ANDROID__)
    #include <GLES3/gl3platform.h>
    #include <GLES3/gl3.h>
    #include <GLES3/gl3ext.h>
    /** The current OpenGL platform */
    #define CU_GL_PLATFORM   CU_GL_OPENGLES
#elif defined (__MACOSX__)
    #include <OpenGL/OpenGL.h>
    #include <OpenGL/gl3.h>
    #include <OpenGL/gl3ext.h>
    /** The current OpenGL platform */
    #define CU_GL_PLATFORM   CU_GL_OPENGL
#elif defined (__WINDOWS__)
    #define NOMINMAX
    #include <windows.h>
    #include <GL/glew.h>
    #include <SDL_opengl.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
    /** The current OpenGL platform */
    #define CU_GL_PLATFORM   CU_GL_OPENGL
#elif defined (__LINUX__)
    #include <GL/glew.h>
    #include <SDL_opengl.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glut.h>
    /** The current OpenGL platform */
    #define CU_GL_PLATFORM   CU_GL_OPENGL
#endif

namespace cugl {
/**
 * Returns a string description of an OpenGL error type
 *
 * @param error The OpenGL error type
 *
 * @return a string description of an OpenGL error type
 */
std::string gl_error_name(GLenum error);

/**
 * Returns a string description of an OpenGL data type
 *
 * @param error The OpenGL error type
 *
 * @return a string description of an OpenGL data type
 */
std::string gl_type_name(GLenum error);
}
#endif /* __CU_RENDER_BASE_H__ */
