#include "proj.h"

#pragma once

#define X_ROTATE_SENSITIVITY 0.001f
#define Y_ROTATE_SENSITIVITY 0.001f

#define X_MOVE_SENSITIVITY 0.01f
#define Y_MOVE_SENSITIVITY 0.01f
#define Z_MOVE_SENSITIVITY 0.01f

class Camera {
private:
	glm::vec3 position;
	// 用于确定一个内部正交坐标系
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	// 描述Camera的姿态(弧度制)
	float yaw = 0.0f;	// 规定0为x正前方，范围为[-pi,pi]
	float pitch = 0.0f;	// 规定0为x正前方，范围为[-pi/2,pi/2]
	float roll = 0.0f;	// 保留，暂时不用
public:
	Camera(glm::vec3 position, glm::vec3 target) :position(position) {
		front = glm::normalize(target - position);	// front只在一开始通过position和target确定一次

		updateAttitude();
		updateLocalCoordiante();
	}
	glm::vec3 getPosition() {
		return position;
	}
	void updateAttitude() {			// 更新相机姿态(俯仰角pitch、航向角yaw)
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
	void updateLocalCoordiante() {	// 更新局部坐标系(根据front向量计算right向量和up向量)
		right = glm::normalize(glm::cross(front, _up));
		up = glm::normalize(glm::cross(right, front));
		// 好像暂时用不到局部坐标系
	}
	void lookAt(glm::vec3 dst) {
		front = glm::normalize(dst - position);
		updateAttitude();
		updateLocalCoordiante();
	}
	void rotate(float dx, float dy) {	// 控制相机旋转(更新front向量)
		yaw -= dx * X_ROTATE_SENSITIVITY;
		pitch -= dy * Y_ROTATE_SENSITIVITY;
		if (pitch > (PI / 2 - 1e-5f))
			pitch = PI / 2 - 1e-5f;
		else if (pitch < (-PI / 2 + 1e-5f))
			pitch = -PI / 2 + 1e-5f;
		front.x = cosf(yaw) * cosf(pitch);
		front.y = sinf(pitch);
		front.z = -sinf(yaw) * cosf(pitch);

		front = glm::normalize(front);
		updateLocalCoordiante();
	}
	void move(float dx, float dy, float dz = 0.0f) {	// // 控制相机旋转(更新相机位置) 
		position += dx * X_MOVE_SENSITIVITY * right + dy * Y_MOVE_SENSITIVITY * up + dz * Z_MOVE_SENSITIVITY * front;
	}
	void moveTo(float x, float y, float z) {	// // 控制相机旋转(更新相机位置) 
		position = glm::vec3(x, y, z);
	}

	// 生成View矩阵和Projection矩阵
	glm::mat4 getViewMatrix() {
		return glm::lookAt(position, position + front, up);
	}
	glm::mat4 getProjectionMatrix() {
		return glm::perspective(45.0f, (float)WIDTH / HEIGHT, 0.1f, 50.0f);
		//return glm::ortho(-5.0, 8.0, -3.0, 7.0, 0.1, 50.0);
	}
	glm::vec3 getShootPos(double x, double y) {
		// x和y为窗口的未归一化坐标
		x = (x - WIDTH / 2) / (WIDTH / 2);
		y = (HEIGHT / 2 - y) / (HEIGHT / 2);
		// 映射到世界坐标系
		glm::vec4 worldPos = glm::inverse(getProjectionMatrix() * getViewMatrix()) * glm::vec4(x, y, 0.1, 1.0);
		return glm::vec3(worldPos.x, worldPos.y, worldPos.z);
	}
	glm::vec3 getFront() {
		return front;
	}
	glm::vec3 getRight() {
		return right;
	}
	glm::vec3 getUp() {
		return up;
	}
};
