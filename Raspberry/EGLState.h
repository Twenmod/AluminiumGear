#pragma once
#include <EGL/egl.h>
#include <EGL/eglplatform.h>

struct EGLState
{
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	EGLConfig config;

	EGLNativeWindowType  nativeWindow;

};
