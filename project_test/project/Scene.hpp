#include "proj.h"
#include "Camera.hpp"
#include "Geometry.hpp"

class Scene {
private:
	std::vector<Geometry*> objs;
	Camera* camera{ 0 }; // ��ǰ�����
	Shader* shader{ 0 }; // ��ǰʹ�õ�shader
	GLuint uboBlock{ 0 };

	float lastTime{ 0 }, currentTime{ 0 }, deltaTime{ 0 };

	const GLuint matrixBindPoint = 0;

public:
	Scene() :currentTime(glfwGetTime()) {
		// ��ʼ��uniform������
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);	// View�����ǻᷢ���仯�ģ�������Ƿ�Ӧ��GL_DYNAMIC_DRAW
		// ��������uniform������
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(glm::mat4(1.0)));					// Ĭ�ϳ�ʼ��Projection����
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(glm::mat4(1.0)));	// Ĭ�ϳ�ʼ��View����
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	Scene(Camera* camera) : currentTime(glfwGetTime()) {
		// ��ʼ��uniform������
		glGenBuffers(1, &uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, uboBlock);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		// ����Camera��ʼ��Projection��View����
		setCamera(camera);
	}
	float step() { // ��Ⱦѭ����ÿһ�ֵ���һ�Σ�������ͼ�任���󣬸��¼�ʱ��������ʱ�䲽��
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(camera->getViewMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		lastTime = currentTime;
		currentTime = glfwGetTime();
		deltaTime = currentTime - lastTime;
		return deltaTime;
	}
	void bindShader(Shader* shader) {			// ����ֻ�ǰ�shader
		// ���µ�shader��uniform������������projection��view��������
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glUniformBlockBinding(shader->getID(), glGetUniformBlockIndex(shader->getID(), "Matrices"), matrixBindPoint);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		this->shader = shader;
	}

	void setCamera(Camera* camera) {	// ��ʱ����
		this->camera = camera;
		// ����uniform buffer�е�Projection����
		glBindBuffer(GL_UNIFORM_BUFFER, uboBlock);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(camera->getProjectionMatrix()));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void render(Geometry* obj) {
		if (this->shader == nullptr) {
			bindShader(DefaultShader::getDefaultShader());	// ���û��Ԥ��bindShader�����Ĭ�ϵ�shader
		}
		shader->use();
		// ����model����modelBuffer���� ��������ɫ����
		(*shader)["model"] = obj->transform.getMatrix();
		(*shader)["modelBuffer"] = obj->getModelBufferMatrix();
		// ���������Ϣ ��Ƭ����ɫ����
		shader->loadAttribute(obj->getAttribute());

		// ��ͼ
		glBindVertexArray(obj->getVAO());
		glDrawElements(GL_TRIANGLES, obj->getVAOLength(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void render(Arrow* obj) {
		render(obj->getArrow());
		render(obj->getBody());
	}

	void render(Axis* obj) {
		render(obj->getAxis_x());
		render(obj->getAxis_y());
		render(obj->getAxis_z());
	}

	void render(Bone* obj) {
		// ���ַ���������Ⱦһ�������ڵ㣬����Ⱦ�ù����������ӹ����ڵ㡣������ʱ��ѡ���һ�ַ���
		// Bone����Ⱦ����ֱ�ӵ���render(Geometry*)����ΪBone��Ҫ�������и������ı任
		// ���⣬Bone�����̳���Geometry��Bone��Geometry��������Ϲ�ϵ

		if (this->shader == nullptr) {
			bindShader(DefaultShader::getDefaultShader());	// ���û��Ԥ��bindShader�����Ĭ�ϵ�shader
		}
		shader->use();
		// ����model����modelBuffer���� ��������ɫ����
		(*shader)["model"] = obj->getTransMatrix() * obj->getObj()->transform.getMatrix();		// modelʹ�õ���Bone��transMatrix(���и������任)��Geometry��model(����ֻ����תû��λ�Ƶı任)����ĳ˻�
		(*shader)["modelBuffer"] = obj->getObj()->getModelBufferMatrix();									// modelBufferʹ�õ���Bone��Geometry��Ա�����
		// ���������Ϣ ��Ƭ����ɫ����
		shader->loadAttribute(obj->getObj()->getAttribute());

		// ��ͼ
		glBindVertexArray(obj->getObj()->getVAO());
		glDrawElements(GL_TRIANGLES, obj->getObj()->getVAOLength(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	void render(Skeleton* obj) {
		// ��������Ⱦ���й����ڵ�
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
};