#include "proj.h"
#include "utils.h"
#include "Geometry.hpp"

#define X_MOVE_SENSITIVITY 0.001f
#define Y_MOVE_SENSITIVITY 0.001f

class Camera {
private:
	glm::vec3 position;
	// 用于确定一个内部正交坐标系
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	// 描述Camera的姿态(弧度制)
	float yaw = 0.0f;	// 规定0为x正前方，范围为[-pi,pi]
	float pitch = 0.0f;	//	规定0为x正前方，范围为[-pi/2,pi/2]
	float roll = 0.0f;	// 保留，暂时不用
	// 上下文
	std::vector<GLuint> program;
public:
	Camera(glm::vec3 position, glm::vec3 target) :position(position) {
		front = glm::normalize(target - position);	// front只在一开始通过position和target确定一次

		updateAttitude();	//计算初始姿态
		updateLocalCoordiante();
	}
	void updateAttitude() {
		// 计算俯仰角
		float tmp = acosf(glm::dot(front, _up));
		if (tmp > 0)
			pitch = PI / 2 - tmp;
		else if (tmp < 0)
			pitch = PI / 2 + tmp;
		// 计算航向角
		glm::vec3 frontDown = glm::normalize(front - glm::dot(front, _up) * _up); //映射到xz平面上的front向量
		float x = glm::dot(frontDown, _right);
		float z = glm::dot(frontDown, _front);

		yaw = atan2f(z, x);
	}
	void updateLocalCoordiante() {
		right = glm::normalize(glm::cross(front, _up));
		up = glm::normalize(glm::cross(right, front));
		// 好像暂时用不到局部坐标系
	}

	void rotate(float dx, float dy) {	// dx,dy为鼠标移动的距离
		yaw -= dx * X_MOVE_SENSITIVITY;
		pitch -= dy * Y_MOVE_SENSITIVITY;
		//std::cout<< dx / 40000<<"   "<<dy/40000<<std::endl;
		if (pitch > (PI / 2 - 1e-5))
			pitch = PI / 2 - 1e-5;
		else if (pitch < (-PI / 2 + 1e-5))
			pitch = -PI / 2 + 1e-5;
		front.x = cosf(yaw) * cosf(pitch);
		front.y = sinf(pitch);
		front.z = -sinf(yaw) * cosf(pitch);

		//std::cout << "(" << front.x << "," << front.y << "," << front.z << ")  "
		//	<< "yaw:" << yaw << " pitch:" << pitch << std::endl;
		front = glm::normalize(front);
		updateLocalCoordiante();
	}
	void updateViewMatrix() {
		glm::mat4 view = glm::lookAt(position, position + front, up);
		for (auto ptr = program.begin(); ptr != program.end(); ptr++) {
			glUseProgram(*ptr);
			updateUniformMatrix4fv(*ptr, "view", view);
			glUseProgram(0);
		}
	}
	void updateProjectionMatrix() {
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 40.0f);
		for (auto ptr = program.begin(); ptr != program.end(); ptr++) {
			glUseProgram(*ptr);
			updateUniformMatrix4fv(*ptr, "projection", projection);
			glUseProgram(0);
		}
	}
	void addAixs(Axis *axis) {
		addProgramList(axis->getProgramList());
	}
	void addProgram(GLuint _program) {
		// 检查program是否合法
		if (_program == 0) {
			std::cout << "错误，用于初始化Camera的program为空！" << std::endl;
			exit(1);
		}
		GLint success;
		glGetProgramiv(_program, GL_LINK_STATUS, &success);
		if (success != GL_TRUE) {
			std::cout << "错误，用于初始化Camera的program不合法！" << std::endl;
			exit(1);
		}
		program.push_back(_program);

		updateProjectionMatrix();
		updateViewMatrix();
	}
	void addProgramList(std::vector<GLuint> programs) {
		for (auto ptr = programs.begin(); ptr != programs.end(); ptr++) {
			if ((*ptr) == 0) {
				std::cout << "错误，用于初始化Camera的program为空！" << std::endl;
				exit(1);
			}
			GLint success;
			glGetProgramiv(*ptr, GL_LINK_STATUS, &success);
			if (success != GL_TRUE) {
				std::cout << "错误，用于初始化Camera的program不合法！" << std::endl;
				exit(1);
			}
			program.push_back(*ptr);
		}
		updateProjectionMatrix();
		updateViewMatrix();
	
	}
	void clearProgram() {
		program.clear();
	}
};
