#pragma once
#undef countof
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#undef countof
#include "MyFiles.h"

#ifdef WINDOWS_BUILD
#include <glad.h>

#else
#include <GLES2/gl2.h>
#endif

#include  "tiny_obj_loader.h"


/*Parts of this file, specifically the routines, LoadandConvert and CalcNormal as well as the data structures used by them
 have been converted and modifed from Soyo Fujita's viewer example code in TinyOBJLoader which was distributed under the MIT
 license. In accordance with that license I acknowledge the original copyright and ownership of those routines and reprint
 the copyright notice here:

The MIT License (MIT)

Copyright (c) 2012-2017 Syoyo Fujita and many contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/


#define PI				3.14159265358979323846264338327950288419716939937510582097494459072381640628620899862803482534211706798f
#define DEG2RAD(x) (x*PI)/180
#define RAD2DEG(x) x*(180/PI)

class Graphics;

inline float Rand(float a_Range) { return ((float)rand() / RAND_MAX) * a_Range; }

using namespace glm;



class ObjectModel
{
public:

	/*********** This is important new content, we are adding the ability to work with OBJ's which consist of multiple shapes called DrawObjects
	 *********** These will mostly be used by OBJ's but we may use them for other types so putting them in the base class might be sensible
	 *********** however if only OBJ's use them, we should move them to the class that loads, but its probable we will create multiple types
	 *********** of derived ObjectModel derived classes that load OBJ's so this is just as availabe here, though a ModelManager/Loader class will later
	 *********** be disussed and used
	 *********** */

	typedef struct {
		// this struct defines our drawobjects as having a VB(o), a number of triangles, and a matriel ID, which is an index to a vector that will
		// grow as we load more and more objects with new materials

		GLuint vb;     // vertex buffer
		int numTriangles;
		size_t material_id;
	} DrawObject;


	ObjectModel();
	ObjectModel(char* FileName, MyFiles*); /* this is a new constructor which can load an obj file, though this is probably temporary
											since there are many other kinds of model loader we will eventually depricate this
											and move the functionality to the classes that specifically need to load obj's
											in this project example we have a type of model called a ShipModel which is going to excusively use OBJ's
											*/

	bool  LoadandConvert(  // a complex routine which actually simplifies the loading of data using Tinyobj
		std::vector<DrawObject>* drawObjects,
		std::vector<tinyobj::material_t>& materials,
		std::map<std::string,
		GLuint>& textures,
		const char* filename,
		MyFiles* FH);


	~ObjectModel();

	// by making these pure virtual (= 0) we MUST ensure that the derived class provides these funcitons
	virtual bool Update() = 0;  // we need to have an Update to move and create the ModelView
	virtual bool Draw() = 0;  // and we need to draw from the vertices





	// a function used sometimes in model loading, so left at the base class for others to use, but mainly OBJ

	static void CalcNormal(float N[3], float v0[3], float v1[3], float v2[3]) {
		float v10[3];
		v10[0] = v1[0] - v0[0];
		v10[1] = v1[1] - v0[1];
		v10[2] = v1[2] - v0[2];

		float v20[3];
		v20[0] = v2[0] - v0[0];
		v20[1] = v2[1] - v0[1];
		v20[2] = v2[2] - v0[2];

		N[0] = v20[1] * v10[2] - v20[2] * v10[1];
		N[1] = v20[2] * v10[0] - v20[0] * v10[2];
		N[2] = v20[0] * v10[1] - v20[1] * v10[0];

		float len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
		if (len2 > 0.0f) {
			float len = sqrtf(len2);

			N[0] /= len;
			N[1] /= len;
		}
	}




	// passed and maybe removed once used
	std::vector<tinyobj::material_t> materials;
	std::vector<DrawObject> gdrawObjects;

	//temp these are to be relocated into the Model Manager	
	static std::map<std::string, GLuint> textures;

	/******************** Back to normal standard objects content*/

	// a few basic utility functions	
	glm::vec3	GetPositon();
	void		SetPosition(glm::vec3*);
	void		SetPosition(glm::vec3);

	glm::vec3	GetRotations();
	void		SetRotations(glm::vec3*);
	void		SetRotations(glm::vec3);

	void		SetXRotation(float);  // if you need to set indivual rotations
	void		SetYRotation(float);
	void		SetZRotation(float);

	void		SetModelMatrix();  // initialises and resets
	void	 	MakeModelMatrix();  // makes and returns the model matrix
	void		MakeRotations();
	void		MakeTranslationMatrix();
	void		StoreGraphicClass(Graphics* Graphics);


	GLuint texture1;  // a handle
	bool		LoadModel(GLvoid* vertices);
	glm::vec4	Colour;

	glm::vec3	WorldPosition;   // where am I in the world?

	glm::vec3	Rotations; 		// what are my rotations? Stored as Radians!
	glm::vec3	DegreeRotations;
	glm::vec3	Scales;

	glm::mat4 mTranslationMatrix;
	glm::mat4 mRotationMatrix;
	glm::mat4 mScaleMatrix;

	glm::mat4 RotationMatrixX;
	glm::mat4 RotationMatrixY;
	glm::mat4 RotationMatrixZ;


	void MakeRotationMatrix();  // since these get altered alot
	GLuint programObject, vertexShader, fragmentShader;  // programObject used for draw. vertex and fragment used to create programObject
	glm::mat4 Model;      // the model matrix will effectivly hold all the rotation and positional data for the object
	GLint	positionLoc;  // index handles to important concepts in texture 
	GLint	texCoordLoc;
	GLint	samplerLoc;
	GLint	MatrixID;
	GLint AmbID;
	Graphics* TheGraphics;  // anything that uses a shader will need access to the graphic class set up in the game app. Make sure this is supplied

	GLvoid* Vertices;  // a pointer to our attribute array, which should contain vertices and texture coords
	GLuint	vbo; 	// handle to our vbo value which should contain the GPU based verison of Vertices
};
