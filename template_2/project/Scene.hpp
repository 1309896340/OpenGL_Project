#include "proj.h"
#include "Camera.hpp"
#include "Geometry.hpp"

class Scene {
private:
	std::vector<Drawable*> objs;
	std::vector<Shader*> shaders;
	Camera* camera; // 当前主相机
	GLuint uboBlock;

	float lastTime, currentTime, deltaTime;

	const GLuint matrixBindPoint = 0;

public:
	Scene() :uboBlock(0), camera(0), lastTime(0.0f), currentTime(glfwGetTime()), deltaTime(0.0f) {
		// 初始化uniform缓冲区
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);	// VIEW矩阵是会发生变化的，不清楚是否应该GL_DYNAMIC_DRAW
		// 数据载入uniform缓冲区
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(glm::mat4(1.0)));	// 初始化PROJECTION矩阵
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(glm::mat4(1.0)));	// 初始化VIEW矩阵
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	Scene(Camera* camera) :uboBlock(0), lastTime(0.0f), currentTime(glfwGetTime()), deltaTime(0.0f) {
		// 初始化uniform缓冲区
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		setCamera(camera);
	}
	float step() { // 更新计时，并返回时间步长
		lastTime = currentTime;
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		return deltaTime;
	}
	void addShader(Shader* shader) {
		// 绑定新的shader的uniform缓冲区
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
		// 更新uniform buffer中的Projection矩阵
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(camera->getProjectionMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void render() {
		// 更新uniform buffer中的view矩阵
		// 不同的shader需要传递不同的uniform变量，这部分工作应该由shader自己完成
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera->getViewMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		for (auto ptr = objs.begin(); ptr != objs.end(); ptr++) {
			(*ptr)->draw();
		}
	}
};
