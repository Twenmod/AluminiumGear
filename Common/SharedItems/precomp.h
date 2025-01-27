//Not actually precompiled because of cross platform things, but still contains stuff that should be
#pragma once




#ifdef WINDOWS_BUILD
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad/glad.h"

//VLD
#ifdef DEBUG
//#include <vld.h>
#endif

#endif

#ifdef Raspberry_BUILD
#include <GLES3/gl31.h>
#include <GLES2/gl2.h>
#endif


#include <map>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <string>
#include <array>

#include "btBulletDynamicsCommon.h"

#include "stb_image.h"


#include "Common.h"
