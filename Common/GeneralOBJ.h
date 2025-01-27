
#pragma once

#include "ObjectModel.h"



class GeneralOBJ : public ObjectModel
{
public:
	GeneralOBJ();
	GeneralOBJ(char* FileName, MyFiles*);

	~GeneralOBJ();
		
	bool Update();  // we supply an update
	bool Draw();  // and a draw 
	

	
	

};

