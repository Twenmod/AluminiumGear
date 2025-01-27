#include "Graphics.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>
using namespace glm;


Graphics::Graphics()
{
	// don't really want to do anything here but could move init here
}
Graphics::~Graphics()
{

}

/*
Initialise an object so that it uses the correct shader to display
return int TRUE if all good.
*/

int Graphics::Init(ObjectModel* TheModel) // this gives every model a BASIC shader, not really meant to always be used
{

	glGetError(); // clear any old set up errors
	// hard code for now student can optimise :D

	GLbyte vShaderStr[] =
		"#version 100 \n"
		"precision mediump float;     \n"
		"attribute vec3 a_position;   \n"
		"attribute vec2 a_texCoord;   \n"
		"uniform mat4 MVP;            \n"
		"varying vec2 v_texCoord;     \n"
		"void main()                  \n"
		"{ 							  \n"
		" gl_Position =  MVP * vec4(a_position,1);\n"
		" v_texCoord = a_texCoord;  \n"
		"}                            \n";

	GLbyte fShaderStr[] =
		"#version 100 \n"
		"precision mediump float;                            \n"
		"varying vec2 v_texCoord;                            \n"
		"uniform sampler2D s_texture;                        \n"
		"uniform vec4  Ambient;\n"
		"void main()                                         \n"
		"{                                                   \n"
		"  gl_FragColor = texture2D( s_texture, v_texCoord )*Ambient;\n"
		"}                                                   \n";


	// Load and compile the vertex/fragment shaders
	TheModel->vertexShader = LoadShader(GL_VERTEX_SHADER, (const char*)vShaderStr);
	TheModel->fragmentShader = LoadShader(GL_FRAGMENT_SHADER, (const char*)fShaderStr);

	// Create the program object	
	TheModel->programObject = glCreateProgram();
	if (TheModel->programObject == 0) 	return 0;

	// now we have the V and F shaders  attach them to the progam object
	glAttachShader(TheModel->programObject, TheModel->vertexShader);
	glAttachShader(TheModel->programObject, TheModel->fragmentShader);

	// Link the program
	glLinkProgram(TheModel->programObject);
	// Check the link status
	GLint AreTheylinked;
	glGetProgramiv(TheModel->programObject, GL_LINK_STATUS, &AreTheylinked);
	if (!AreTheylinked)
	{
		GLint RetinfoLen = 0;
		// check and report any errors
		glGetProgramiv(TheModel->programObject, GL_INFO_LOG_LENGTH, &RetinfoLen);
		if (RetinfoLen > 1)
		{
			GLchar* infoLog = (GLchar*)malloc(sizeof(char) * RetinfoLen);
			glGetProgramInfoLog(TheModel->programObject, RetinfoLen, NULL, infoLog);
			fprintf(stderr, "Error linking program:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteProgram(TheModel->programObject);
		return FALSE;
	}

	// Get the attribute locations
	TheModel->positionLoc = glGetAttribLocation(TheModel->programObject, "a_position");
	TheModel->texCoordLoc = glGetAttribLocation(TheModel->programObject, "a_texCoord");

	// Get the sampler location
	TheModel->samplerLoc = glGetUniformLocation(TheModel->programObject, "s_texture");
	TheModel->MatrixID = glGetUniformLocation(TheModel->programObject, "MVP");  //LOOK!!!!
	TheModel->AmbID= glGetUniformLocation(TheModel->programObject, "Ambient");


	if (glGetError() == GL_NO_ERROR)	return TRUE;
	else
		printf("Oh bugger, Model graphic init failed\n");
	return FALSE;

}



///
// Create a simple width x height texture image with RGBA format
//
GLuint Graphics::CreateSimpleTexture2D(int width, int height, char* TheData)
{
	// Texture object handle
	GLuint textureId;

	// Use tightly packed data
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// Generate a texture object
	glGenTextures(1, &textureId);

	// Bind the texture object
	glBindTexture(GL_TEXTURE_2D, textureId);

	// Load the texture

	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA,
		width,
		height,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		TheData);

	if (glGetError() != GL_NO_ERROR) printf("Oh bugger");

	// Set the filtering mode
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	if (glGetError() == GL_NO_ERROR)	return textureId;
	printf("Oh bugger");

	return textureId;
}

/*
 Now we have be able to create a shader object, pass the shader source
 and them compile the shader.
*/
GLuint Graphics::LoadShader(GLenum type, const char* shaderSrc)
{
	// 1st create the shader object
	GLuint TheShader = glCreateShader(type);

	if (TheShader == 0) return FALSE; // can't allocate so stop.

	// pass the shader source then compile it
	glShaderSource(TheShader, 1, &shaderSrc, NULL);
	glCompileShader(TheShader);

	GLint  IsItCompiled;

	// After the compile we need to check the status and report any errors
	glGetShaderiv(TheShader, GL_COMPILE_STATUS, &IsItCompiled);
	if (!IsItCompiled)
	{
		GLint RetinfoLen = 0;
		glGetShaderiv(TheShader, GL_INFO_LOG_LENGTH, &RetinfoLen);
		if (RetinfoLen > 1)
		{
			// standard output for errors
			char* infoLog = (char*)malloc(sizeof(char) * RetinfoLen);
			glGetShaderInfoLog(TheShader, RetinfoLen, NULL, infoLog);
			fprintf(stderr, "Error compiling this shader:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteShader(TheShader);
		return FALSE;
	}
	return TheShader;
}

