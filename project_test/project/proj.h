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

using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::mat3;
using glm::quat;
using glm::normalize;
using glm::length;
using glm::cross;
using glm::dot;
using glm::radians;
using glm::identity;

using std::vector;
using std::deque;
using std::map;
using std::cout;
using std::endl;

extern vec3 _up;
extern vec3 _right;
extern vec3 _front;
extern vec3 _origin;

// 为了确保这个结构的内存是连续的，要做一些检查
// static_assert(sizeof(glm::vec3) == sizeof(GLfloat) * 3, "Platform doesn't support this directly.");
typedef struct _Vertex{
	vec3 position;
	vec3 normal;
	vec4 color;
	// 后续可以增加其他属性
}Vertex;

typedef struct _LeafInfo {
	float length = 1.0f;				// 叶长
	float theta = 0.2f;				// 叶宽因子
	float k = 5.0f;						// 叶片弯曲系数
	float SLAngle = 30.0f;			// 茎叶夹角
	float MVAngle = 0.0f;			// 主叶脉扭曲角度
} LeafInfo;

typedef struct _StatusInfo {
	bool leftMouseButtonPressed = false;
	bool rightMouseButtonPressed = false;
	double mousePos[2];
	bool shiftPressed = false;
	unsigned int lastKey = 0;
	vec3 lightPos = vec3(0.1f, 0.1f, 0.1f);
	//bool startShoot = false;
	//double shootPos[2];
	LeafInfo leafstatus;
}StatusInfo;


typedef struct _uniformTable {	// 除了MVP矩阵以外的其他定制化uniform变量
	bool autoColor{ true };
	vec4 color{ 0.0f,0.0f,0.0f,0.0f };
}uniformTable;

#endif