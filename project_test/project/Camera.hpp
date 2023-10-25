#ifndef __WIND_CAMERA
#define __WIND_CAMERA

#include "proj.h"

#define X_ROTATE_SENSITIVITY 0.001f
#define Y_ROTATE_SENSITIVITY 0.001f

#define X_MOVE_SENSITIVITY 0.01f
#define Y_MOVE_SENSITIVITY 0.01f
#define Z_MOVE_SENSITIVITY 0.01f

class Camera {
private:
	vec3 position;
	// ����ȷ��һ���ڲ���������ϵ
	vec3 front;
	vec3 up;
	vec3 right;
	// ����Camera����̬(������)
	float yaw = 0.0f;	// �涨0Ϊx��ǰ������ΧΪ[-pi,pi]
	float pitch = 0.0f;	// �涨0Ϊx��ǰ������ΧΪ[-pi/2,pi/2]
	float roll = 0.0f;	// ��������ʱ����
public:
	Camera(vec3 position, vec3 target) :position(position) {
		front = normalize(target - position);	// frontֻ��һ��ʼͨ��position��targetȷ��һ��

		updateAttitude();
		updateLocalCoordiante();
	}
	vec3 getPosition() {
		return position;
	}
	void updateAttitude() {			// ���������̬(������pitch�������yaw)
		// ���㸩����
		float tmp = acosf(dot(front, _up));
		if (tmp > 0)
			pitch = PI / 2 - tmp;
		else if (tmp < 0)
			pitch = PI / 2 + tmp;
		// ���㺽���
		vec3 frontDown = normalize(front - dot(front, _up) * _up); //ӳ�䵽xzƽ���ϵ�front����
		float x = dot(frontDown, _right);
		float z = dot(frontDown, _front);

		yaw = atan2f(z, x);
	}
	void updateLocalCoordiante() {	// ���¾ֲ�����ϵ(����front��������right������up����)
		right = normalize(cross(front, _up));
		up = normalize(cross(right, front));
		// ������ʱ�ò����ֲ�����ϵ
	}
	void lookAt(vec3 dst) {
		front = normalize(dst - position);
		updateAttitude();
		updateLocalCoordiante();
	}
	void rotate(float dx, float dy) {	// ���������ת(����front����)
		yaw -= dx * X_ROTATE_SENSITIVITY;
		pitch -= dy * Y_ROTATE_SENSITIVITY;
		if (pitch > (PI / 2 - 1e-5f))
			pitch = PI / 2 - 1e-5f;
		else if (pitch < (-PI / 2 + 1e-5f))
			pitch = -PI / 2 + 1e-5f;
		front.x = cosf(yaw) * cosf(pitch);
		front.y = sinf(pitch);
		front.z = -sinf(yaw) * cosf(pitch);

		front = normalize(front);
		updateLocalCoordiante();
	}
	void rotateByAxis(float dx, float dy, vec3 axis = _up) {
		// Ĭ������������y��Ϊ��ת��
		mat3 rotMat = mat3(glm::rotate(glm::rotate(mat4(1.0f), -dx * X_ROTATE_SENSITIVITY, axis), -dy * Y_ROTATE_SENSITIVITY, right));
		position = rotMat * position;
		front = rotMat * front;
		updateAttitude();
		updateLocalCoordiante();
	}
	void move(float dx, float dy, float dz = 0.0f) {	// // ���������ת(�������λ��) 
		position += dx * X_MOVE_SENSITIVITY * right + dy * Y_MOVE_SENSITIVITY * up + dz * Z_MOVE_SENSITIVITY * front;
	}
	void moveTo(float x, float y, float z) {	// // ���������ת(�������λ��) 
		position = vec3(x, y, z);
	}

	// ����View�����Projection����
	mat4 getViewMatrix() {
		return glm::lookAt(position, position + front, up);
	}
	mat4 getProjectionMatrix() {
		return glm::perspective(45.0f, (float)WIDTH / HEIGHT, 0.1f, 50.0f);
		//return ortho(-5.0, 8.0, -3.0, 7.0, 0.1, 50.0);
	}
	vec3 getShootPos(double x, double y) {
		// x��yΪ���ڵ�δ��һ������
		x = (x - WIDTH / 2) / (WIDTH / 2);
		y = (HEIGHT / 2 - y) / (HEIGHT / 2);
		// ӳ�䵽��������ϵ
		vec4 worldPos = inverse(getProjectionMatrix() * getViewMatrix()) * vec4(x, y, 0.1, 1.0);
		return vec3(worldPos.x, worldPos.y, worldPos.z);
	}
	vec3 getFront() {
		return front;
	}
	vec3 getRight() {
		return right;
	}
	vec3 getUp() {
		return up;
	}
};

#endif