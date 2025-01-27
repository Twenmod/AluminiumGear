#include "GeneralOBJ.h"


GeneralOBJ::GeneralOBJ()
{
// a default system
	mTranslationMatrix	= glm::mat4(1.0f);
	mRotationMatrix		= glm::mat4(1.0f);
	mScaleMatrix		= glm::mat4(1.0f);
	Model	= glm::mat4(1.0f);  // give the model I as an initialisation
	glm::vec3 Pos = glm::vec3(11.0f, 0.0f, 0.4f);
	this->SetPosition(Pos); 
	
	Scales = glm::vec3(1.0f, 1.0f, 1.0f);
	SetModelMatrix();
	DegreeRotations = glm::vec3(0, 0, 0);
	Colour = glm::vec4(Rand(1.0f), Rand(1.0f), Rand(1.0f), 1.0f); 
	

}

// this is a handler to cope with OBJ files
GeneralOBJ::GeneralOBJ(char* FileName, MyFiles* FH) :GeneralOBJ()
{ // because we decided to have an OBJ LoadandConvert method in the base ObjectModel class, we can use that base class method to do our work for us
 // keeping this derived class simple
	bool ret = ObjectModel::LoadandConvert(&gdrawObjects, materials, textures, FileName, FH);
	 
	if (ret == false) 
	{  
		printf("This load for %s failed, check file name and try again!\n", FileName); 
		 
	}  
	
}


GeneralOBJ::~GeneralOBJ()
{ // if we created a memory location for things, we need to make sure we remove it here when it is deleted
  // this alos applies to specific single instances of GPU space like VBO's, for now, unwisely, we will not worry about this!
}


bool GeneralOBJ::Update()
{
	//set up model view
		glUseProgram(this->programObject);
		DegreeRotations.x += 1.1f;
		Rotations.x = DEG2RAD(DegreeRotations.x);
		DegreeRotations.y -= 1.1f;
		Rotations.y = DEG2RAD(DegreeRotations.y);
	
	//	DegreeRotations.z += 1.1f;
		Rotations.z = DEG2RAD(DegreeRotations.z);
		mScaleMatrix = glm::mat4(1.0f);
	
	
	mScaleMatrix = glm::scale(mScaleMatrix, Scales);
	MakeRotationMatrix();
	MakeTranslationMatrix();
	MakeModelMatrix(); 

	// get these from the camera normally but we'll hard code them now
	
	// Projection matrix : 45° Field of View, 4:3 ratio, display range : near 0.1 unit <->  far 200 units
	glm::mat4 Projection = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 200.0f);
	
	glm::mat4 View  = glm::lookAt(
		glm::vec3(0, 20, 10),
		 // Camera is at (0,20,10), in World Space again this would mostly come from a camera
		glm::vec3(0, 0, 0),
		 // look at the origin
		glm::vec3(0, 1, 0)  // pointing up( 0,-1,0 will be upside-down)););}
		);

	glUseProgram(this->programObject);
	glEnable(GL_DEPTH_TEST);

	// make the MVP 
	glm::mat4 MVP        = Projection * View * Model;  // Remember order seems backwards


	float Amb[] = {1.0f, 1.0f, 1.0f, 1.0f};  // you can change these colour values to give your model a tint this is more likely to be a common value
	glUniform4fv(AmbID, 1, &Amb[0]);
	
	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);


	
	return true;
}
bool GeneralOBJ::Draw()
{
	for (int frames = 0; frames < gdrawObjects.size(); frames++)
	
	{
		DrawObject o = gdrawObjects[frames];  // get the DrawObject which has the vb
		glBindBuffer(GL_ARRAY_BUFFER, o.vb);  // this is now the buffer we work with

		GLsizei stride = (3 + 3 + 3 + 2)*sizeof(GLfloat);  // we have 3x vert, 3x normal,3xcolour,2x texcoord. We don't use the colour
			//load the vertex data info
		glVertexAttribPointer(this->positionLoc, 
			3,		// there are 3 values xyz
			GL_FLOAT, // they a float
			GL_FALSE,	 // don't need to be normalised
			stride,	  // how may floats to the next one
			(const void*)0  //this->Vertices // where do they start as an index); // use 3 values, but add 5 each time to get to the next
			);
	
		// Load the texture coordinate
		glVertexAttribPointer(this->texCoordLoc,
			2,	 // there are 2 values
			GL_FLOAT,	 //they are floats 
			GL_FALSE,	 // we don't need them normalise
			stride,		  // wats the stride to the next ones
			(const void*)(sizeof(float) * 9)  //(GLfloat*)this->Vertices + 9);
			);
		glEnableVertexAttribArray(this->positionLoc);
		glEnableVertexAttribArray(this->texCoordLoc);
		glActiveTexture(GL_TEXTURE0); 
		// bind the correct texture for this shape	
		if((o.material_id < materials.size())) {
			std::string diffuse_texname = materials[o.material_id].diffuse_texname;
			if (textures.find(diffuse_texname) != textures.end()) {
			glBindTexture(GL_TEXTURE_2D, textures[diffuse_texname]);

			}
		}
		glUniform1i(this->samplerLoc, 0);
		glDrawArrays(GL_TRIANGLES, 0, o.numTriangles);
	
		// take these off the bind		
		glDisableVertexAttribArray(positionLoc);
		glDisableVertexAttribArray(texCoordLoc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	
		
	}	
	// always good to unbind	
		glBindTexture(GL_TEXTURE_2D, 0);
	return true;	
}



