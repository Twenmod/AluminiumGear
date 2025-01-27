#include "MyFiles.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


MyFiles::MyFiles()
{
}

MyFiles::~MyFiles()
{
}


char* MyFiles::Load(char const *filename, int* width, int* height)
{ 

	unsigned char *data = stbi_load(filename, width, height, &comp, 4); // ask it to load 4 componants since its rgba
	return (char*) data;
}


// will load a standard style text file
const char* MyFiles::LoadText(char const *filename)
{
	static string line; //<<note the fix for losing chars?
	
	ifstream myfile(filename);
	
	if (myfile.is_open())
	{
		getline(myfile, line, (char) myfile.eof()); // *this.eof isn't ideal but it works
		myfile.close();
	}
	
	return line.c_str();
	
}