#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "glad\glad.h"
#include "GLFW\glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define PI 3.141592653589f

using namespace std;


using glm::vec3;
using glm::mat4;
using glm::length;
using glm::cross;
using glm::dot;
using glm::normalize;

string readFile(const char* filePath) {
	ifstream file(filePath);
	stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}
GLuint compileComputeShader() {
	GLuint shader = glCreateProgram();
	GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
	string source = readFile("shader\\cshader.gcs");
	const char* csource = source.c_str();
	glShaderSource(computeShader, 1, &csource, NULL);
	glCompileShader(computeShader);
	GLint success;
	glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
	assert(success == GL_TRUE);
	glAttachShader(shader, computeShader);
	glLinkProgram(shader);
	glDeleteShader(computeShader);
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	assert(success == GL_TRUE);
	return shader;
}
GLuint compileShader() {
	GLuint shader = glCreateProgram();
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	GLint success;

	string source = readFile("shader\\vshader.gvs");
	const char* csource = source.c_str();
	glShaderSource(vertexShader, 1, &csource, NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	assert(success == GL_TRUE);

	source = readFile("shader\\fshader.gfs");
	csource = source.c_str();
	glShaderSource(fragmentShader, 1, &csource, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	assert(success == GL_TRUE);

	glAttachShader(shader, vertexShader);
	glAttachShader(shader, fragmentShader);
	glLinkProgram(shader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	// ��֤shader program�Ƿ���������
	glGetProgramiv(shader, GL_LINK_STATUS, &success);
	assert(success == GL_TRUE);
	return shader;
}


typedef struct _TriangleBuffer {
	vec3 vpos[3]{ glm::identity<vec3>() ,glm::identity<vec3>() ,glm::identity<vec3>() };
	float area{ 0.0f };
}TriangleBuffer;

int main(void) {
	// GLFW��ʼ��
	assert(glfwInit() == GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Hello World", NULL, NULL);
	assert(window != nullptr);
	glfwMakeContextCurrent(window);
	// GLAD��ʼ��
	assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));
	// ������������
	glEnable(GL_DEPTH);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// ����shader program
	GLuint shader = compileShader();
	// ������Ķ�������
	float radius = 1.0f;
	unsigned int wNum = 36, hNum = 20;
	vector<vec3> vPos((wNum + 1) * (hNum + 1), vec3(0.0f, 0.0f, 0.0f));
	vector<GLuint> vIndex(wNum * hNum * 6, 0);
	float dw = 2.0f * PI / wNum, dh = PI / hNum;
	for (unsigned int h = 0; h <= hNum; h++) {
		for (unsigned int w = 0; w <= wNum; w++) {
			float xx = radius * sinf(h * dh) * cosf(w * dw);
			float yy = radius * sinf(h * dh) * sinf(w * dw);
			float zz = radius * cos(h * dh);
			vPos[h * (wNum + 1) + w] = vec3(xx, zz, -yy);
			if (h != hNum && w != wNum) {
				vIndex[(h * wNum + w) * 6 + 0] = h * (wNum + 1) + w;
				vIndex[(h * wNum + w) * 6 + 1] = h * (wNum + 1) + w + 1;
				vIndex[(h * wNum + w) * 6 + 2] = (h + 1) * (wNum + 1) + w + 1;
				vIndex[(h * wNum + w) * 6 + 3] = h * (wNum + 1) + w;
				vIndex[(h * wNum + w) * 6 + 4] = (h + 1) * (wNum + 1) + w + 1;
				vIndex[(h * wNum + w) * 6 + 5] = (h + 1) * (wNum + 1) + w;
			}
		}
	}
	// ����VAO VBO EBO
	GLuint VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vPos.size() * sizeof(vec3), vPos.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, NULL, (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vIndex.size() * sizeof(GLuint), vIndex.data(), GL_STATIC_DRAW);
	// ����uniform
	static mat4 model = mat4(1.0f);
	static mat4 view = glm::lookAt(vec3(0.0f, 2.0f, 6.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
	static mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.f);
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// ���ü�����ɫ��
	GLuint cshader = compileComputeShader();
	// ʹ�ü�����ɫ����������Ԫ��Ŀǰ���ð뾶Ϊ1������������ӦΪ4PI������Ԫ�ָ���Ŀ������ƽ�
	//����һ����ɫ���洢�������
	GLuint SSBO;
	glGenBuffers(1, &SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, wNum * hNum * 2 * sizeof(TriangleBuffer), NULL, GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
	// ���ȼ�����ɫ��
	glUseProgram(cshader);


	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		model = glm::rotate(model, glm::radians(1.0f), vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));

		glDrawElements(GL_TRIANGLES, (GLsizei)vIndex.size(), GL_UNSIGNED_INT, 0);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	return 0;
}
