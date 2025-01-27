#pragma once
#include <X11/X.h>
#include <X11/Xlib.h>

#include <EGL/eglplatform.h>
#include "EGLState.h"
#include <EGL/eglext.h>

class XWindow
{
public:
	XWindow();

	void CreateWindow();
	const EGLState& GetState() const;

	Display& GetDisplay();
	Window& GetWindow();

private:
	Display* display;
	Window root;
	Window window;
	EGLState state;
};

static const EGLint context_attributes[] =
{
	EGL_CONTEXT_MAJOR_VERSION_KHR,
	3,
	EGL_CONTEXT_MINOR_VERSION_KHR,
	1,
	EGL_NONE,
	EGL_NONE
};

const EGLint attribute_list[] =
{
	EGL_RED_SIZE,
	8,
	EGL_GREEN_SIZE,
	8,
	EGL_BLUE_SIZE,
	8,
	EGL_ALPHA_SIZE,
	8,
	EGL_DEPTH_SIZE,
	16,	
//	EGL_SAMPLE_BUFFERS,1, // these tend to kill performance on a Pi
//	EGL_SAMPLES,4,
	EGL_SURFACE_TYPE,
	EGL_WINDOW_BIT,
#ifdef GLES3	
	EGL_CONFORMANT,
	EGL_OPENGL_ES3_BIT_KHR,
#endif	
	EGL_NONE
};