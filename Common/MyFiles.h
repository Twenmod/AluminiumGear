#pragma once

//Wrapper for graphic loading, do not use for binary or text files.

#include <iostream> // we're going to be loading files so need these
#include <fstream>
#include <string>
using namespace std;

class MyFiles
{
public:
	MyFiles();
	~MyFiles();
	
	int height;
	int width;
	int comp;
	

	char* Load(char const *filename, int*, int*); // specifically for graphics
	
	const char* LoadText(char const *filename);
	
};

