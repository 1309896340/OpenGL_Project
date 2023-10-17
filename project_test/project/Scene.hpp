#ifndef __WIND_SCENE
#define __WIND_SCENE

#include "proj.h"
#include "Camera.hpp"
#include "Geometry.hpp"

class Scene {
private:
	std::vector<Geometry*> objs;
	Camera* camera{ 0 }; // 当前主相机
	GLuint uboBlock{ 0 };

	float lastTime{ 0 }, currentTime{ 0 }, deltaTime{ 0 };

	const GLuint matrixBindPoint = 0;

public:
	Scene() :currentTime((float)glfwGetTime()) {
		// 初始化uniform缓冲区
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);	// View矩阵是会发生变化的，不清楚是否应该GL_DYNAMIC_DRAW
		// 数据载入uniform缓冲区
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(glm::mat4(1.0)));					// 默认初始化Projection矩阵
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(glm::mat4(1.0)));	// 默认初始化View矩阵
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// 绑定默认的着色器
		bindAllDefaultShader();
	}
	Scene(Camera* camera) : currentTime((float)glfwGetTime()) {
		// 初始化uniform缓冲区
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		// 根据Camera初始化Projection和View矩阵
		setCamera(camera);

		// 绑定默认的着色器
		bindAllDefaultShader();
	}
	void bindAllDefaultShader() {
		bindShader(DefaultShader::getShader());
		bindShader(NormalShader::getShader());
	}
	float step(float *t=nullptr) { // 渲染循环中每一轮调用一次，更新视图变换矩阵，更新计时，并返回时间步长
		static float t_accum = 0.0f;
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera->getViewMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		lastTime = currentTime;
		currentTime = (float)glfwGetTime();
		deltaTime = currentTime - lastTime;

		// ======= t_sec 仅仅用于每秒更新一次FPS显示，属于调试信息，可以删除 =======
		static float t_sec = 0.0f;
		t_sec += deltaTime;
		if (t_sec > 1.0f) {		
			t_sec = 0.0f;
			std::cout << "FPS: " << 1.0f / deltaTime << std::endl;
		}
		// =====================================================

		t_accum += deltaTime;
		if (t != nullptr) {
			*t = t_accum;	// 返回累计时间
		}
		return deltaTime;
	}
	void bindShader(Shader* shader) {			// 仅仅只是绑定shader
		// 绑定新的shader的uniform缓冲区（共享projection和view两个矩阵）
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glUniformBlockBinding(shader->getID(), glGetUniformBlockIndex(shader->getID(), "Matrices"), matrixBindPoint);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void setCamera(Camera* camera) {	// 即时更新
		this->camera = camera;
		// 更新uniform buffer中的Projection矩阵
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(camera->getProjectionMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void render(Drawable* obj) {
		// Drawable不考虑子节点
		obj->draw(DefaultShader::getShader());
	}
	void render(Geometry* obj) {
		// Geometry需要考虑子节点
		obj->drawAll();
	}
	void render(Leaf* obj, Shader *shader) {
		obj->draw(shader);
	}
};

#endif
