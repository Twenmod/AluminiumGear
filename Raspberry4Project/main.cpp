//Set up the Raspberry/Linux systems
#include "precomp.h"
#include "Input.h"

#include <iostream>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include "XWindow.h"
#include "RaspKeyboard.h"
#include "RaspMouse.h"
#include "RaspGraphics.h"
#include "IInput.h"
#include "Game.h"

#include "signal.h"

float lastTime{};
float deltaTime{ 0.2f };

int currentFrame = 0;

RaspGraphics* graphics;
real::InputManager* input;
Game* game;
int main()
{

    graphics = new RaspGraphics();


    PFNGLDEBUGMESSAGECALLBACKKHRPROC peglDebugMessageControlKHR = reinterpret_cast<PFNGLDEBUGMESSAGECALLBACKKHRPROC>(eglGetProcAddress("glDebugMessageCallback"));
    if (!(peglDebugMessageControlKHR != 0)) {
        printf("failed to eglGetProcAddress eglDebugMessageControlKHR\n");
    }
    else {
        GLDEBUGPROCKHR DebugFn = +[](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
            {
                switch (severity)
                {
                case GL_DEBUG_SEVERITY_HIGH_KHR:
                case GL_DEBUG_SEVERITY_MEDIUM_KHR:
                    std::cout << message << std::endl;
					raise(SIGTRAP);
					break;
                case GL_DEBUG_SEVERITY_LOW_KHR:
                case GL_DEBUG_SEVERITY_NOTIFICATION_KHR:
                default:
                    break; //Ignore.
                }
            };
        peglDebugMessageControlKHR(DebugFn, nullptr);
		glEnable(GL_DEBUG_OUTPUT_KHR);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR);
    }
    printf("This cross project was partly inspired by BUas Student Ferri de Lange\n");
    printf("This GPU supplied by  :%s\n", glGetString(GL_VENDOR));
    printf("This GPU supports GL  :%s\n", glGetString(GL_VERSION));
    printf("This GPU Renders with :%s\n", glGetString(GL_RENDERER));
    printf("This GPU Shaders are  :%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    input = new real::InputManager(new RaspKeyboard(), new RaspMouse(graphics->Window().GetDisplay(), graphics->Window().GetWindow()));
   
 // now we fire up our game giving it access to the input systems and graphics which are different on each platform but abstracted away   
    game = new Game(input, graphics);
	graphics->game = game;                                                   
	game->Start();

	delete game;
	delete input;
	delete graphics;
    return 0;
}
