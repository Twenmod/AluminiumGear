#include "precomp.h"
#include "RaspGraphics.h"
#include <EGL/egl.h>
#include "EGLState.h"
#include "RaspMouse.h"
#include "XWindow.h"

#include "Game.h"

RaspGraphics::RaspGraphics() : window(new XWindow())
{
	window->CreateWindow();
}

void RaspGraphics::Quit()
{
	XDestroyWindow(&window->GetDisplay(), window->GetWindow());
}

void RaspGraphics::SwapBuffer()
{
	//Check for resize event
	while (XPending(&window->GetDisplay()) > 0)
	{
		XEvent event;
		XNextEvent(&window->GetDisplay(), &event);

		if (event.type == ConfigureNotify)
		{
			XConfigureEvent xce = event.xconfigure;

			// The window was resized
			int newWidth = xce.width;
			int newHeight = xce.height;

			// Call your resize handling function
			game->ResizeWindow(newWidth, newHeight);
		}
	}

	EGLState state = window->GetState();
	eglSwapBuffers(state.display, state.surface);

}

XWindow& RaspGraphics::Window() const
{
	return *window;
}

void RaspGraphics::SetFullScreen(bool fullScreen)
{
}
