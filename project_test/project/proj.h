#pragma once
#include "GLAD/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <ctime>
#include <chrono>

#include <windows.h>


#define WIDTH 1600
#define HEIGHT 1200

#define PI 3.14159265358979323846

extern float deltaTime;
extern float lastFrame;


extern glm::vec3 _up;
extern glm::vec3 _right;
extern glm::vec3 _front;
