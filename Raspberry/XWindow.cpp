#include "XWindow.h"

#include <cstring>
#include <EGL/eglplatform.h>
#include <EGL/egl.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <cstring>
#include <GLES3/gl31.h>

#include "Game.h"

XWindow::XWindow()
{
}

void XWindow::CreateWindow()
{
	display = XOpenDisplay(nullptr);
	root = RootWindowOfScreen(DefaultScreenOfDisplay(display));

	XSetWindowAttributes swa;
	XSetWindowAttributes  xattr;
	Atom wm_state;
	XWMHints hints;

	EGLint numConfigs;
	EGLint majorVersion;
	EGLint minorVersion;

	swa.event_mask = ExposureMask | PointerMotionMask | KeyPressMask | KeyReleaseMask;
	swa.background_pixmap = None;
	swa.background_pixel = 0;
	swa.border_pixel = 0;
	swa.override_redirect = true;

	window = XCreateWindow(
		display,
		root,
		0,		// puts it at the top left of the screen?
		0,
		WINDOW_WIDTH,	//set size  
		WINDOW_HEIGHT,
		0,
		CopyFromParent,
		InputOutput,
		CopyFromParent,
		CWEventMask,
		&swa);

	XSelectInput(display, window, KeyPressMask | KeyReleaseMask | StructureNotifyMask);
	xattr.override_redirect = false;
	XChangeWindowAttributes(display, window, CWOverrideRedirect, &xattr);

	hints.input = true;
	hints.flags = InputHint;
	XSetWMHints(display, window, &hints);
	
	// make the window visible on the screen
	XMapWindow(display, window);
	XStoreName(display, window, "Pi Project 3D");
	//XSetInputFocus(display, window, RevertToNone, CurrentTime);

	XClearWindow(display, window);
	XMapRaised(display, window);

	// get identifiers for the provided atom name strings
	wm_state = XInternAtom(display, "_NET_WM_STATE", false);
	Atom wm_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", false);
	XChangeProperty(display, window, wm_state, XA_ATOM, 32, PropModeReplace, (unsigned char*)&wm_fullscreen, 1);

	/*XEvent xev;
	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = window;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 1;
	xev.xclient.data.l[1] = false;
	XSendEvent(
		display,
		DefaultRootWindow(display),
		false,
		SubstructureNotifyMask,
		&xev);*/
	
	state.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(state.display, &majorVersion, &minorVersion);
	eglGetConfigs(state.display, nullptr, 0, &numConfigs);
	eglChooseConfig(state.display, attribute_list, &state.config, 1, &numConfigs);

	// OpenGL3 setup.
	state.context = eglCreateContext(state.display, state.config, nullptr, context_attributes);

	state.surface = eglCreateWindowSurface(state.display, state.config, window, nullptr);
	eglMakeCurrent(state.display, state.surface, state.surface, state.context);

	eglSurfaceAttrib(display, state.surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_PRESERVED);
	//eglSwapInterval(state.display, 1);
	eglSwapInterval(state.display, 0);

}

const EGLState& XWindow::GetState() const
{
	return state;
}

Display& XWindow::GetDisplay()
{
	return *display;
}

Window& XWindow::GetWindow()
{
	return window;
}
