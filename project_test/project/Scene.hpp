#ifndef __WIND_SCENE
#define __WIND_SCENE

#include "proj.h"
#include "Camera.hpp"
#include "Geometry.hpp"

class Scene {
private:
	std::vector<Geometry*> objs;
	Camera* camera{ 0 }; // ��ǰ�����
	GLuint uboBlock{ 0 };

	float lastTime{ 0 }, currentTime{ 0 }, deltaTime{ 0 };

	const GLuint matrixBindPoint = 0;

public:
	Scene() :currentTime((float)glfwGetTime()) {
		// ��ʼ��uniform������
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);	// View�����ǻᷢ���仯�ģ�������Ƿ�Ӧ��GL_DYNAMIC_DRAW
		// ��������uniform������
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(glm::mat4(1.0)));					// Ĭ�ϳ�ʼ��Projection����
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(glm::mat4(1.0)));	// Ĭ�ϳ�ʼ��View����
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// ��Ĭ�ϵ���ɫ��
		bindAllDefaultShader();
	}
	Scene(Camera* camera) : currentTime((float)glfwGetTime()) {
		// ��ʼ��uniform������
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		// ����Camera��ʼ��Projection��View����
		setCamera(camera);

		// ��Ĭ�ϵ���ɫ��
		bindAllDefaultShader();
	}
	void bindAllDefaultShader() {
		bindShader(DefaultShader::getShader());
		bindShader(NormalShader::getShader());
	}
	float step(float *t=nullptr) { // ��Ⱦѭ����ÿһ�ֵ���һ�Σ�������ͼ�任���󣬸��¼�ʱ��������ʱ�䲽��
		static float t_accum = 0.0f;
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera->getViewMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		lastTime = currentTime;
		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;

		// ======= t_sec ��������ÿ�����һ��FPS��ʾ�����ڵ�����Ϣ������ɾ�� =======
		static float t_sec = 0.0f;
		t_sec += deltaTime;
		if (t_sec > 1.0f) {		
			t_sec = 0.0f;
			std::cout << "FPS: " << 1.0f / deltaTime << std::endl;
		}
		// =====================================================

		t_accum += deltaTime;
		if (t != nullptr) {
			*t = t_accum;	// �����ۼ�ʱ��
		}
		return deltaTime;
	}
	void bindShader(Shader* shader) {			// ����ֻ�ǰ�shader
		// ���µ�shader��uniform������������projection��view��������
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glUniformBlockBinding(shader->getID(), glGetUniformBlockIndex(shader->getID(), "Matrices"), matrixBindPoint);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void setCamera(Camera* camera) {	// ��ʱ����
		this->camera = camera;
		// ����uniform buffer�е�Projection����
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(camera->getProjectionMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void render(Drawable* obj) {
		// Drawable�������ӽڵ�
		obj->draw(DefaultShader::getShader());
	}
	void render(Geometry* obj) {
		// Geometry��Ҫ�����ӽڵ�
		obj->drawAll();
	}
	void render(Leaf* obj, Shader *shader) {
		obj->draw(shader);
	}
};

#endif
