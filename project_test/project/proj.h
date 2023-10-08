#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <chrono>
#include <vector>
//#include <memory>

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

#define PI 3.1415926535f

#define DEFAULT_LINE_WIDTH 0.02f

#define EPS 1e-7

extern glm::vec3 _up;
extern glm::vec3 _right;
extern glm::vec3 _front;

typedef struct {
	float x, y, z;
}vec3;
typedef struct {
	float x, y, z, w;
}vec4;

typedef struct _StatusInfo {
	bool leftMouseButtonPressed = false;
	bool rightMouseButtonPressed = false;
	double mousePos[2];
	unsigned int lastKey = 0;
	bool startShoot = false;
	bool shiftPressed = false;
	double shootPos[2];
	glm::vec3 lightPos = glm::vec3(0.1f, 0.1f, 0.1f);
}StatusInfo;

typedef struct _uniformTable {	// 除了MVP矩阵以外的其他定制化uniform变量
	bool autoColor{ true };
	glm::vec4 color{ 0.0f,0.0f,0.0f,0.0f };
}uniformTable;

