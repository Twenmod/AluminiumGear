#pragma once



// Graphics class primary responsibility is to set up OpenGLES for render and to hold info on window
#include <stdio.h>
#include <assert.h>
#include <string>
#include <math.h>


#ifdef WINDOWS_BUILD
#include <glad.h>
#include <GLFW/glfw3.h>
#else
#include <GLES2/gl2.h>

#endif

// Include GLM
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ObjectModel.h"
#include "Input.h"



// these defines are used often (and Pi is a joke :D) 
#define PI	3.14159265358979323846264338327950288419716939937510582097494459072381640628620899862803482534211706798f
#define DEG2RAD(x) (x*PI)/180
#define RAD2DEG(x) x*(180/PI)




//most standard OGL demos use int versions of TRUE and FALSE (careful not to mix up with bool true and false)
#define TRUE 1
#define FALSE 0

////ObjectData is a small container struct that provides shader info for render of an object, its usable but constrained!!!
//typedef struct
//{
//	// save a Handle to a program object
//	GLuint programObject;
//	// Attribute locations in that shader
//	GLint	positionLoc;  // these are common to almost all shaders
//	GLint	texCoordLoc;
//	GLint	samplerLoc;  	// Sampler location
//	// Texture handle
//	GLuint	textureId;  // assuming the texture is in gpu ram here's its ID
//
//} ObjectData;


class Graphics
{
public:
	Graphics();
	~Graphics();

	// this is really for the shader (edit later)
	int Init(ObjectModel*);
	// we provide a simple Texture loader/store syste	
	GLuint CreateSimpleTexture2D(int width, int height, char* TheData); // oh you lucky people

GLuint LoadShader(GLenum type, const char* shaderSrc);
};



