#include "proj.h"
#include "Camera.hpp"
#include "Geometry.hpp"

class Scene {
private:
	std::vector<Geometry*> objs;
	Camera* camera{ 0 }; // 当前主相机
	Shader* shader{ 0 }; // 当前使用的shader
	GLuint uboBlock{ 0 };

	float lastTime{ 0 }, currentTime{ 0 }, deltaTime{ 0 };

	const GLuint matrixBindPoint = 0;

public:
	Scene() :currentTime(glfwGetTime()) {
		// 初始化uniform缓冲区
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);	// View矩阵是会发生变化的，不清楚是否应该GL_DYNAMIC_DRAW
		// 数据载入uniform缓冲区
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(glm::mat4(1.0)));					// 默认初始化Projection矩阵
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(glm::mat4(1.0)));	// 默认初始化View矩阵
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	Scene(Camera* camera) : currentTime(glfwGetTime()) {
		// 初始化uniform缓冲区
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		// 根据Camera初始化Projection和View矩阵
		setCamera(camera);
	}
	float step() { // 渲染循环中每一轮调用一次，更新视图变换矩阵，更新计时，并返回时间步长
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera->getViewMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		lastTime = currentTime;
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		return deltaTime;
	}
	void bindShader(Shader* shader) {			// 仅仅只是绑定shader
		// 绑定新的shader的uniform缓冲区（共享projection和view两个矩阵）
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glUniformBlockBinding(shader->getID(), glGetUniformBlockIndex(shader->getID(), "Matrices"), matrixBindPoint);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		this->shader = shader;
	}

	void setCamera(Camera* camera) {	// 即时更新
		this->camera = camera;
		// 更新uniform buffer中的Projection矩阵
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(camera->getProjectionMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void render(Geometry* obj) {
		if (this->shader == nullptr) {
			bindShader(DefaultShader::getDefaultShader());	// 如果没有预先bindShader，则绑定默认的shader
		}
		shader->use();
		// 传入model矩阵、modelBuffer矩阵 （顶点着色器）
		(*shader)["model"] = obj->transform.getMatrix();
		(*shader)["modelBuffer"] = obj->getModelBufferMatrix();
		// 传入材质信息 （片段着色器）
		shader->loadUniform(obj->getAttribute());
	}

	void render(Arrow* obj) {
		if (this->shader == nullptr) {
			bindShader(DefaultShader::getDefaultShader());
		}
		render(obj->getArrow());
		render(obj->getBody());
	}

	void render(Axis* obj) {
		if (this->shader == nullptr) {
			bindShader(DefaultShader::getDefaultShader());
		}
		render(obj->getAxis_x());
		render(obj->getAxis_y());
		render(obj->getAxis_z());
	}

	void render(Bone* obj) {
		if (this->shader == nullptr) {
			bindShader(DefaultShader::getDefaultShader());
		}
		// 两种方案，仅渲染一个骨骼节点，或渲染该骨骼上所有子骨骼节点
		// 。。。
	}
};
