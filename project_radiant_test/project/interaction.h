#ifndef __WIND_INTERACTION
#define __WIND_INTERACTION

#include "proj.h"
#include "Camera.hpp"
#include "Geometry.hpp"
#include "Wheat.hpp"

void framebuff_size_callback(GLFWwindow* window, int width, int height);
void mouse_botton_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

#endif