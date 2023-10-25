#ifndef __WIND_SCENE
#define __WIND_SCENE

#include "proj.h"
#include "Camera.hpp"
#include "Geometry.hpp"
#include "Wheat.hpp"

class Scene {
	// Ŀǰ���� Scene����һ�����������������е�Shader��Geometry��Camera
	// �������ǵ��������ģ�Geometry��ҪScene����ȡһ��Ĭ��shader
	// �Ǿ͸ɴ����������shader�ó�������Ϊһ��ȫ�ֱ���������Geometry�Ͳ���Ҫ����Scene��
private:
	std::vector<Geometry*> objs;
	Camera* camera{ 0 }; // ��ǰ�����
	GLuint uboBlock{ 0 };

	float lastTime{ 0 }, currentTime{ (float)glfwGetTime() }, deltaTime{ 0 };

	const GLuint matrixBindPoint = 0;		// projection��view�ӿڿ�󶨵�

public:
	std::map<std::string, Shader*> shaders;

	Scene() : currentTime((float)glfwGetTime()) {		// ���Ϊ����ģʽ��ֻ��ͨ��getInstance()��ȡʵ��
		initUniformBuffer();
		initShaders();					//  ��ʼ������Shader�����롢����
		bindAllShaders();				//  ������Shader��uniform������
	}
	~Scene() {
		for (auto& shader : shaders) {
			delete shader.second;
		}
		glDeleteBuffers(1, &uboBlock);
	}

	void initShaders() {
		shaders["normal"] = new Shader("shader/normVisualize.gvs", "shader/normVisualize.ggs", "shader/normVisualize.gfs");	// ������Ԫ���߿��ӻ�
		shaders["normal_v"] = new Shader("shader/nshader.gvs", "shader/nshader.ggs", "shader/nshader.gfs");				// ���㷨�߿��ӻ�	
		
		shaders["line"] = new Shader("shader/line.gvs","shader/line.gfs");				// ���Ƽ�����
		shaders["plane"] = new Shader("shader/plane.gvs", "shader/plane.gfs");		// ���Ƽ�ƽ��

		shaders["leaf"] = new Shader("shader/leaf.gvs","shader/leaf.gfs");		// ��ȾС��ҶƬ����ɫ�������а�������
	}

	void initUniformBuffer() {
		// ��ʼ��uniform������
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(mat4), NULL, GL_DYNAMIC_DRAW);
		// ��������uniform������
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), value_ptr(mat4(1.0)));							// ��ջ�����
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), value_ptr(mat4(1.0)));	// ��ջ�����
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void bindAllShaders() {
		for (auto& shader : shaders) {
			bindShader(shader.second);
		}
	}
	void bindShader(Shader* shader) {			// ����ֻ�ǰ�shader
		// ���µ�shader��uniform������������projection��view��������
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glUniformBlockBinding(shader->getID(), glGetUniformBlockIndex(shader->getID(), "Matrices"), matrixBindPoint);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	float step(float* t = nullptr) { // ��Ⱦѭ����ÿһ�ֵ���һ�Σ�������ͼ�任���󣬸��¼�ʱ��������ʱ�䲽��
		static float t_accum = 0.0f;
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), value_ptr(camera->getViewMatrix()));
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

	void setCamera(Camera* camera) {	// ��camera����Ϊ��ǰ�����������һ�θ���ͶӰ����ͼ�任����
		this->camera = camera;
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), value_ptr(camera->getProjectionMatrix()));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), value_ptr(camera->getViewMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void render(Drawable* obj) {
		// Drawable�������ӽڵ�
		obj->draw(nullptr);
	}
	//void render(Geometry* obj) {
	//	// Geometry��Ҫ�����ӽڵ�
	//	obj->drawAll();
	//}
	//void render(Leaf* obj, Shader* shader) {
	//	obj->draw(shader);
	//}
};

#endif
