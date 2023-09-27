#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <chrono>
#include <vector>

#include "GLAD/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "tinynurbs.h"

//#include <windows.h>



#define WIDTH 1200
#define HEIGHT 900

#define PI 3.1415926535897932384626433832795

#define DEFAULT_LINE_WIDTH 0.02f

#define EPS 1e-7

extern float deltaTime;
extern float lastFrame;


extern glm::vec3 _up;
extern glm::vec3 _right;
extern glm::vec3 _front;

typedef struct {
	float x, y, z;
}vec3;
typedef struct {
	float x, y, z, w;
}vec4;
