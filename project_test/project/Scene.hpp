#ifndef __WIND_SCENE
#define __WIND_SCENE

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
		bindShader(DefaultShader::getDefaultShader());
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
		bindShader(DefaultShader::getDefaultShader());
	}
	float step() { // 渲染循环中每一轮调用一次，更新视图变换矩阵，更新计时，并返回时间步长
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera->getViewMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		lastTime = currentTime;
		currentTime = (float)glfwGetTime();
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

	void render(Drawable* obj) {
		// 暂时先不考虑obj可能具有子节点
		obj->draw();
	}

	void render(Arrow* obj) {
		render(obj->getArrow());
		render(obj->getBody());
	}

	//void render(Axis* obj) {
	//	render(obj->getAxis_x());
	//	render(obj->getAxis_y());
	//	render(obj->getAxis_z());
	//}

	void render(Bone* obj) {
		// 两种方案，仅渲染一个骨骼节点，或渲染该骨骼上所有子骨骼节点。这里暂时先选择第一种方案
		// Bone的渲染不能直接调用render(Geometry*)，因为Bone需要考虑所有父骨骼的变换
		// 另外，Bone并不继承自Geometry，Bone和Geometry仅仅是组合关系

		if (this->shader == nullptr) {
			bindShader(DefaultShader::getDefaultShader());	// 如果没有预先bindShader，则绑定默认的shader
		}
		shader->use();
		// 传入model矩阵、modelBuffer矩阵 （顶点着色器）
		(*shader)["model"] = obj->getTransMatrix() * obj->getObj()->transform.getMatrix();		// model使用的是Bone的transMatrix(所有父骨骼变换)和Geometry的model(自身只有旋转没有位移的变换)矩阵的乘积
		(*shader)["modelBuffer"] = obj->getObj()->getModelBufferMatrix();									// modelBuffer使用的是Bone的Geometry成员自身的
		// 传入材质信息 （片段着色器）
		shader->loadAttribute(obj->getObj()->getAttribute());

		// 绘图
		glBindVertexArray(obj->getObj()->getVAO());
		glDrawElements(GL_TRIANGLES, obj->getObj()->getVAOLength(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	void render(Skeleton* obj) {
		// 遍历，渲染所有骨骼节点
		std::deque<Bone*> buf{ obj->getRoot() };
		Bone* tmp = nullptr;
		while (!buf.empty()) {
			tmp = buf.front();
			buf.pop_front();
			render(tmp);
			for (auto& child : tmp->getChildren()) {
				buf.push_back(child);
			}
		}
	}

	void render(Leaf* obj) {
		if (obj->isChangedMesh())
			obj->updateMesh();
		render(dynamic_cast<Geometry*>(obj));
	}

	void render(Combination* obj) {
		std::vector<Geometry*> children = obj->getChildren();
		for (int i = 0; i < children.size(); i++) {
			// 此时，child是一个Geometry*  （暂时不考虑child可能是Combination*的情况）
			// child的modelBuffer需要被考虑，其model在加入Combination时被复位
			// Combination自身的model和modelBuffer需要被考虑，其childModel在加入Combination时被置为对应child的model
			shader->use();
			(*shader)["modelBuffer"] = obj->getModelBufferMatrix();
			(*shader)["model"] = (obj->transform.getMatrix()) * obj->getChildModel(i) * (children[i]->transform.getMatrix()) * (children[i]->getModelBufferMatrix());
			shader->loadAttribute(children[i]->getAttribute());
			glBindVertexArray(children[i]->getVAO());
			glDrawElements(GL_TRIANGLES, children[i]->getVAOLength(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}
};

#endif