#include "proj.h"
#include "Camera.hpp"
#include "Geometry.hpp"

class Scene {
private:
	std::vector<Drawable*> objs;
	Camera* camera; // ��ǰ�����

	const GLuint matrixBindPoint = 0;

	GLuint uboBlock;
public:
	Scene():uboBlock(0),camera(0){
		// ��ʼ��uniform������
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 128, NULL, GL_STATIC_DRAW);	// VIEW�����ǻᷢ���仯�ģ�������Ƿ�Ӧ��GL_DYNAMIC_DRAW
		// ����������uniform������
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(glm::mat4(1.0)));	// ��ʼ��PROJECTION����
		glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, glm::value_ptr(glm::mat4(1.0)));	// ��ʼ��VIEW����
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	Scene(Camera* camera):uboBlock(0) {
		// ��ʼ��uniform������
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 128, NULL, GL_STATIC_DRAW);	// VIEW�����ǻᷢ���仯�ģ�������Ƿ�Ӧ��GL_DYNAMIC_DRAW
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		setCamera(camera);
	}

	void addObj(Drawable* obj) {
		objs.push_back(obj);
	}
	void setCamera(Camera* camera) {
		this->camera = camera;
		// ����uniform buffer�е�Projection����
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(camera->getProjectionMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void render() {
		// ����uniform buffer�е�view����
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, glm::value_ptr(camera->getViewMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		for (auto ptr = objs.begin(); ptr != objs.end(); ptr++) {
			(*ptr)->draw();
		}
	}
};
