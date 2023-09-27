#include "proj.h"
#include "Camera.hpp"
#include "Geometry.hpp"

class Scene {
private:
	std::vector<Drawable*> objs;
	Camera* camera; // 当前主相机

	const GLuint matrixBindPoint = 0;

	GLuint uboBlock;
public:
	Scene():uboBlock(0),camera(0){
		// 初始化uniform缓冲区
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 128, NULL, GL_STATIC_DRAW);	// VIEW矩阵是会发生变化的，不清楚是否应该GL_DYNAMIC_DRAW
		// 将数据载入uniform缓冲区
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(glm::mat4(1.0)));	// 初始化PROJECTION矩阵
		glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, glm::value_ptr(glm::mat4(1.0)));	// 初始化VIEW矩阵
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	Scene(Camera* camera):uboBlock(0) {
		// 初始化uniform缓冲区
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 128, NULL, GL_STATIC_DRAW);	// VIEW矩阵是会发生变化的，不清楚是否应该GL_DYNAMIC_DRAW
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		setCamera(camera);
	}

	void addObj(Drawable* obj) {
		objs.push_back(obj);
	}
	void setCamera(Camera* camera) {
		this->camera = camera;
		// 更新uniform buffer中的Projection矩阵
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, 64, glm::value_ptr(camera->getProjectionMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void render() {
		// 更新uniform buffer中的view矩阵
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 64, 64, glm::value_ptr(camera->getViewMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		for (auto ptr = objs.begin(); ptr != objs.end(); ptr++) {
			(*ptr)->draw();
		}
	}
};
