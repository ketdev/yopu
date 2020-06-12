#pragma once

#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES 1
#include <SDL_opengles2.h>
#elif defined _WIN32
#define GLEW_STATIC
#include <gl/glew.h>
#include <gl/wglew.h>
#include <SDL2/SDL_opengl.h>
#include <gl/glu.h>
#elif defined __ANDROID__
#include <EGL/egl.h>
//#include <GLES/gl.h>
#include <GLES3/gl31.h>
#include <GLES3/gl3ext.h>
#elif defined __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#define GL_GLEXT_PROTOTYPES 1
#include <SDL2/SDL_opengles2.h>
#endif

#if defined _MSC_VER
#define ALIGN(x) __declspec(align(x))
#elif defined __GNUC__
#define ALIGN(x) __attribute__((aligned(x)))
#endif
