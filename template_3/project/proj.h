#ifndef _WIND_MAIN_HEADER
#define _WIND_MAIN_HEADER

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <functional>
#include <vector>
#include <deque>
#include <map>

#include "GLAD/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"


#define WIDTH 1600
#define HEIGHT 1200
#define PI 3.1415926535f
#define DEFAULT_LINE_WIDTH 0.02f
#define EPS 1e-7

extern glm::vec3 _up;
extern glm::vec3 _right;
extern glm::vec3 _front;
extern glm::vec3 _origin;

typedef struct _vec2 {
	float x, y;
}vec2;

typedef struct _vec3 {
	float x, y, z;
}vec3;

typedef struct _vec4 {
	float x, y, z, w;
}vec4;

typedef struct _LeafInfo {
	float length = 1.0f;				// Ҷ��
	float theta = 0.2f;				// Ҷ������
	float k = 5.0f;						// ҶƬ����ϵ��
	float SLAngle = 30.0f;			// ��Ҷ�н�
	float MVAngle = 0.0f;			// ��Ҷ��Ť���Ƕ�
} LeafInfo;

typedef struct _StatusInfo {
	bool leftMouseButtonPressed = false;
	bool rightMouseButtonPressed = false;
	double mousePos[2];
	bool shiftPressed = false;
	unsigned int lastKey = 0;
	glm::vec3 lightPos = glm::vec3(0.1f, 0.1f, 0.1f);
	//bool startShoot = false;
	//double shootPos[2];
	LeafInfo leafstatus;
}StatusInfo;


typedef struct _uniformTable {	// ����MVP����������������ƻ�uniform����
	bool autoColor{ true };
	glm::vec4 color{ 0.0f,0.0f,0.0f,0.0f };
}uniformTable;

#endif