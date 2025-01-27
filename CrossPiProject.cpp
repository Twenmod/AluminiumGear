// CrossPiProject.cpp : This file contains the PC 'main' function. Program execution begins and ends there.
//
#include "precomp.h"
#include "Input.h"
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include "Game.h"
#include "WindowsGraphics.h"
#include "WindowsInput.h"
#

//
WindowsGraphics* WinGraphics;
real::InputManager* input;
Game* game;

void window_size_callback(GLFWwindow*, int width, int height)
{
	game->ResizeWindow(glm::ivec2(width, height));
}


// Mains only purpose is to set up OGL and then jump to general game code



int main()
{
	WinGraphics = new WindowsGraphics();

	glfwSwapInterval(0);


	input = new real::InputManager(new WindowsKeyboard(WinGraphics->Window()), new WindowsMouse(WinGraphics->Window()));
	printf("This cross project was partly inspired by BUas Student Ferri de Lange\n"); 
	printf("This GPU supplied by  :%s\n", glGetString(GL_VENDOR));
	printf("This GPU supports GL  :%s\n", glGetString(GL_VERSION));
	printf("This GPU Renders with :%s\n", glGetString(GL_RENDERER));
	printf("This GPU Shaders are  :%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	game = new Game(input, WinGraphics);

	//GLFW window resize callback
	glfwSetWindowSizeCallback(&WinGraphics->Window(), window_size_callback);

	game->Start();

	delete game;
	delete input;
	delete WinGraphics;

	return 0;
}

#ifdef RELEASE
int WinMain()
{
	return main();
}
#endif

void framebufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
	glViewport(0, 0, width, height);
}
