#ifndef _WIND_MAIN_HEADER
#define _WIND_MAIN_HEADER

#define TEST_SOFT_RASTERIZATION

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>
#include <functional>
#include <vector>
#include <deque>
#include <map>
#include <set>

#include <memory>
#include <cfloat>

#ifdef TEST_OPENGL
#include "GLAD/glad.h"
#include "GLFW/glfw3.h"
#endif

#ifdef TEST_SOFT_RASTERIZATION
#include "opencv2/opencv.hpp"
using cv::line;
using cv::Mat;
using cv::Point2i;
using cv::Point2f;
using cv::Scalar;
using cv::Vec3f;
#endif

#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "utils.h"

#define WIDTH 1600
#define HEIGHT 1200
#define PI 3.1415926535f
#define DEFAULT_LINE_WIDTH 0.02f
#define MEPS 1e-7

using glm::vec2;
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
using std::set;
using std::cout;
using std::endl;

extern vec3 _up;
extern vec3 _right;
extern vec3 _front;
extern vec3 _origin;


// Ϊ��ȷ������ṹ���ڴ��������ģ�Ҫ��һЩ���
// static_assert(sizeof(glm::vec3) == sizeof(GLfloat) * 3, "Platform doesn't support this directly.");
typedef struct _Vertex {
	vec3 position{ 0.0f,0.0f,0.0f };
	vec3 normal{ 0.0f,1.0f,0.0f };
	vec4 color{ 0.0f,0.0f,0.0f,0.0f };
	// ��������������������
}Vertex;

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
	vec3 lightPos = vec3(0.1f, 0.1f, 0.1f);
	//bool startShoot = false;
	//double shootPos[2];
	LeafInfo leafstatus;
}StatusInfo;


typedef struct _uniformTable {	// ����MVP����������������ƻ�uniform����
	bool autoColor{ true };
	vec4 color{ 0.0f,0.0f,0.0f,0.0f };
}uniformTable;

typedef struct _Triangle {
	Vertex vertex[3];
}Triangle;

extern Mat canvas;		// �����ڵ���������ȫ�ֱ���

//class Light;
//class Scene;


#endif