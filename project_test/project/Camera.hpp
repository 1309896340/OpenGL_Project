#include "proj.h"
#include "utils.h"
#include "Geometry.hpp"

#define X_MOVE_SENSITIVITY 0.001f
#define Y_MOVE_SENSITIVITY 0.001f

class Camera {
private:
	glm::vec3 position;
	// ����ȷ��һ���ڲ���������ϵ
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	// ����Camera����̬(������)
	float yaw = 0.0f;	// �涨0Ϊx��ǰ������ΧΪ[-pi,pi]
	float pitch = 0.0f;	//	�涨0Ϊx��ǰ������ΧΪ[-pi/2,pi/2]
	float roll = 0.0f;	// ��������ʱ����
	// ������
	std::vector<GLuint> program;
public:
	Camera(glm::vec3 position, glm::vec3 target) :position(position) {
		front = glm::normalize(target - position);	// frontֻ��һ��ʼͨ��position��targetȷ��һ��

		updateAttitude();	//�����ʼ��̬
		updateLocalCoordiante();
	}
	void updateAttitude() {
		// ���㸩����
		float tmp = acosf(glm::dot(front, _up));
		if (tmp > 0)
			pitch = PI / 2 - tmp;
		else if (tmp < 0)
			pitch = PI / 2 + tmp;
		// ���㺽���
		glm::vec3 frontDown = glm::normalize(front - glm::dot(front, _up) * _up); //ӳ�䵽xzƽ���ϵ�front����
		float x = glm::dot(frontDown, _right);
		float z = glm::dot(frontDown, _front);

		yaw = atan2f(z, x);
	}
	void updateLocalCoordiante() {
		right = glm::normalize(glm::cross(front, _up));
		up = glm::normalize(glm::cross(right, front));
		// ������ʱ�ò����ֲ�����ϵ
	}

	void rotate(float dx, float dy) {	// dx,dyΪ����ƶ��ľ���
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
		// ���program�Ƿ�Ϸ�
		if (_program == 0) {
			std::cout << "�������ڳ�ʼ��Camera��programΪ�գ�" << std::endl;
			exit(1);
		}
		GLint success;
		glGetProgramiv(_program, GL_LINK_STATUS, &success);
		if (success != GL_TRUE) {
			std::cout << "�������ڳ�ʼ��Camera��program���Ϸ���" << std::endl;
			exit(1);
		}
		program.push_back(_program);

		updateProjectionMatrix();
		updateViewMatrix();
	}
	void addProgramList(std::vector<GLuint> programs) {
		for (auto ptr = programs.begin(); ptr != programs.end(); ptr++) {
			if ((*ptr) == 0) {
				std::cout << "�������ڳ�ʼ��Camera��programΪ�գ�" << std::endl;
				exit(1);
			}
			GLint success;
			glGetProgramiv(*ptr, GL_LINK_STATUS, &success);
			if (success != GL_TRUE) {
				std::cout << "�������ڳ�ʼ��Camera��program���Ϸ���" << std::endl;
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
