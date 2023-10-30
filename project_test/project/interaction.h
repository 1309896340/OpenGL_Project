

#ifndef __WIND_INTERACTION
#define __WIND_INTERACTION
#include "proj.h"

#ifdef TEST_OPENGL
#include "Camera.hpp"
#include "Geometry.hpp"
#include "Wheat.hpp"

void framebuff_size_callback(GLFWwindow* window, int width, int height);
void mouse_botton_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
#endif

#ifdef TEST_SOFT_RASTERIZATION
#include "proj.h"
#include "Camera.hpp"
void opencv_mouseCallback(int event, int x, int y, int flags, void* userdata);

#endif

#endif

