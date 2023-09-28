#include "proj.h"
#include "Camera.hpp"
#include "Geometry.hpp"

class Scene {
private:
	std::vector<Drawable*> objs;
	std::vector<Shader*> shaders;
	Camera* camera; // ��ǰ�����
	GLuint uboBlock;

	float lastTime, currentTime, deltaTime;

	const GLuint matrixBindPoint = 0;

public:
	Scene() :uboBlock(0), camera(0), lastTime(0.0f), currentTime(glfwGetTime()), deltaTime(0.0f) {
		// ��ʼ��uniform������
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);	// VIEW�����ǻᷢ���仯�ģ�������Ƿ�Ӧ��GL_DYNAMIC_DRAW
		// ��������uniform������
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(glm::mat4(1.0)));	// ��ʼ��PROJECTION����
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(glm::mat4(1.0)));	// ��ʼ��VIEW����
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	Scene(Camera* camera) :uboBlock(0), lastTime(0.0f), currentTime(glfwGetTime()), deltaTime(0.0f) {
		// ��ʼ��uniform������
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		setCamera(camera);
	}
	float step() { // ���¼�ʱ��������ʱ�䲽��
		lastTime = currentTime;
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		return deltaTime;
	}
	void addShader(Shader* shader) {
		// ���µ�shader��uniform������
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glUniformBlockBinding(shader->getID(), glGetUniformBlockIndex(shader->getID(), "Matrices"), matrixBindPoint);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		shaders.push_back(shader);
	}
	void addObj(Drawable* obj) {
		objs.push_back(obj);
	}
	void setCamera(Camera* camera) {
		this->camera = camera;
		// ����uniform buffer�е�Projection����
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(camera->getProjectionMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void render() {
		// ����uniform buffer�е�view����
		// ��ͬ��shader��Ҫ���ݲ�ͬ��uniform�������ⲿ�ֹ���Ӧ����shader�Լ����
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera->getViewMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		for (auto ptr = objs.begin(); ptr != objs.end(); ptr++) {
			(*ptr)->draw();
		}
	}
};
