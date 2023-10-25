#ifndef __WIND_SCENE
#define __WIND_SCENE

#include "proj.h"
#include "Camera.hpp"
#include "Geometry.hpp"
#include "Wheat.hpp"

class Scene {
	// 目前来看 Scene类是一个管理器，管理所有的Shader、Geometry、Camera
	// 但它不是单向依赖的，Geometry需要Scene来获取一个默认shader
	// 那就干脆把这个特殊的shader拿出来声明为一个全局变量，这样Geometry就不需要依赖Scene了
private:
	std::vector<Geometry*> objs;
	Camera* camera{ 0 }; // 当前主相机
	GLuint uboBlock{ 0 };

	float lastTime{ 0 }, currentTime{ (float)glfwGetTime() }, deltaTime{ 0 };

	const GLuint matrixBindPoint = 0;		// projection和view接口快绑定点

public:
	std::map<std::string, Shader*> shaders;

	Scene() : currentTime((float)glfwGetTime()) {		// 设计为单例模式，只能通过getInstance()获取实例
		initUniformBuffer();
		initShaders();					//  初始化所有Shader，编译、链接
		bindAllShaders();				//  绑定所有Shader的uniform缓冲区
	}
	~Scene() {
		for (auto& shader : shaders) {
			delete shader.second;
		}
		glDeleteBuffers(1, &uboBlock);
	}

	void initShaders() {
		shaders["normal"] = new Shader("shader/normVisualize.gvs", "shader/normVisualize.ggs", "shader/normVisualize.gfs");	// 三角面元法线可视化
		shaders["normal_v"] = new Shader("shader/nshader.gvs", "shader/nshader.ggs", "shader/nshader.gfs");				// 顶点法线可视化	
		
		shaders["line"] = new Shader("shader/line.gvs","shader/line.gfs");				// 绘制简单线条
		shaders["plane"] = new Shader("shader/plane.gvs", "shader/plane.gfs");		// 绘制简单平面

		shaders["leaf"] = new Shader("shader/leaf.gvs","shader/leaf.gfs");		// 渲染小麦叶片的着色器，其中包含材质
	}

	void initUniformBuffer() {
		// 初始化uniform缓冲区
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(mat4), NULL, GL_DYNAMIC_DRAW);
		// 数据载入uniform缓冲区
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), value_ptr(mat4(1.0)));							// 清空缓冲区
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), value_ptr(mat4(1.0)));	// 清空缓冲区
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void bindAllShaders() {
		for (auto& shader : shaders) {
			bindShader(shader.second);
		}
	}
	void bindShader(Shader* shader) {			// 仅仅只是绑定shader
		// 绑定新的shader的uniform缓冲区（共享projection和view两个矩阵）
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glUniformBlockBinding(shader->getID(), glGetUniformBlockIndex(shader->getID(), "Matrices"), matrixBindPoint);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	float step(float* t = nullptr) { // 渲染循环中每一轮调用一次，更新视图变换矩阵，更新计时，并返回时间步长
		static float t_accum = 0.0f;
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), value_ptr(camera->getViewMatrix()));
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

	void setCamera(Camera* camera) {	// 将camera设置为当前主相机，并第一次更新投影、视图变换矩阵
		this->camera = camera;
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), value_ptr(camera->getProjectionMatrix()));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), value_ptr(camera->getViewMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void render(Drawable* obj) {
		// Drawable不考虑子节点
		obj->draw(nullptr);
	}
	//void render(Geometry* obj) {
	//	// Geometry需要考虑子节点
	//	obj->drawAll();
	//}
	//void render(Leaf* obj, Shader* shader) {
	//	obj->draw(shader);
	//}
};

#endif
