#ifndef __WIND_SCENE
#define __WIND_SCENE

#include "proj.h"
#include "Camera.hpp"
#include "Geometry.hpp"
#include "Wheat.hpp"

typedef struct {
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	GLuint elementNum;	// ����������Ƶ����
	// ÿ��MeshӦ���и��Ե�shader����������չ
}MeshRenderInfo;

typedef struct {
	vector<MeshRenderInfo> meshesInfo;
	//ÿ��MeshӦ�������������ԣ���������չ
} GeometryRenderInfo;


class Scene {
	// Ŀǰ���� Scene����һ�����������������е�Shader��Geometry��Camera
	// �������ǵ��������ģ�Geometry��ҪScene����ȡһ��Ĭ��shader
	// �Ǿ͸ɴ����������shader�ó�������Ϊһ��ȫ�ֱ���������Geometry�Ͳ���Ҫ����Scene��
private:
	std::map<Geometry*, GeometryRenderInfo> objs;		// ��addʱ��һ���µ�GeometryRenderInfo������ʼ�����㻺��
	Camera* camera{ 0 }; // ��ǰ�����
	GLuint ubo{ 0 };

	float lastTime{ 0 }, currentTime{ (float)glfwGetTime() }, deltaTime{ 0 };

	const GLuint matrixBindPoint = 0;		// projection��view�ӿڿ�󶨵�

public:
	std::map<std::string, Shader*> shaders;

	Scene() : currentTime((float)glfwGetTime()) {
		initShaders();					//  ��ʼ������Shader�����롢����
		initUniformBuffer();			//  ��ʼ��uniform������
	}

	Scene(Camera *camera):Scene(){
		setCamera(camera);
	}

	~Scene() {
		for (auto& shader : shaders) {
			delete shader.second;
		}
		glDeleteBuffers(1, &ubo);
	}

	void initShaders() {
		shaders["default"] = new Shader("shader/shader.gvs", "shader/shader.gfs");		// Ĭ��shader
		shaders["normal"] = new Shader("shader/normVisualize.gvs", "shader/normVisualize.ggs", "shader/normVisualize.gfs");	// ������Ԫ���߿��ӻ�
		shaders["normal_v"] = new Shader("shader/nshader.gvs", "shader/nshader.ggs", "shader/nshader.gfs");				// ���㷨�߿��ӻ�	

		shaders["line"] = new Shader("shader/line.gvs", "shader/line.gfs");				// ���Ƽ�����
		shaders["plane"] = new Shader("shader/plane.gvs", "shader/plane.gfs");		// ���Ƽ�ƽ��

		shaders["leaf"] = new Shader("shader/leaf.gvs", "shader/leaf.gfs");		// ��ȾС��ҶƬ����ɫ�������а�������
	}

	void initUniformBuffer() {
		// ��ʼ��uniform������
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(mat4), NULL, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, ubo);
	}

	float step(float* t = nullptr) { // ��Ⱦѭ����ÿһ�ֵ���һ�Σ�������ͼ�任���󣬸��¼�ʱ��������ʱ�䲽��
		static float t_accum = 0.0f;
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), value_ptr(camera->getViewMatrix()));

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
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), value_ptr(camera->getProjectionMatrix()));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), value_ptr(camera->getViewMatrix()));
		//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void addOne(Geometry* obj) {	// ���һ��Geometry���������Ӷ���
		GeometryRenderInfo gInfo;
		vector<Mesh*> meshes = obj->getMeshes();
		for (auto& mesh : meshes) {
			MeshRenderInfo mInfo;
			// ��������VAO
			glGenVertexArrays(1, &mInfo.VAO);
			glBindVertexArray(mInfo.VAO);
			// ��������VBO
			glGenBuffers(1, &mInfo.VBO);
			glBindBuffer(GL_ARRAY_BUFFER, mInfo.VBO);
			glBufferData(GL_ARRAY_BUFFER, mesh->getVertexSize() * sizeof(Vertex), mesh->mapVertexData(), GL_DYNAMIC_DRAW);
			mesh->unmapVertexData();
			// ����VBO��������
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);								// λ������
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3)));			// ����������
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(vec3)));			// ��ɫ����
			glEnableVertexAttribArray(2);
			// ��������EBO
			glGenBuffers(1, &mInfo.EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mInfo.EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->getIndexSize() * sizeof(GLuint), mesh->getIndexPtr(), GL_STATIC_DRAW);
			// ���ö���������
			mInfo.elementNum = mesh->getIndexSize();
			// ���뵽gInfo��
			gInfo.meshesInfo.push_back(mInfo);
		}
		objs[obj] = gInfo;		// ����map
	}
	//void addOne(Leaf* obj) {
	//	addOne(dynamic_cast<Geometry*>(obj));
	//	objs[obj].type = LEAF;
	//}

	void add(Geometry* obj) {
		deque<Geometry*> buf{ obj };
		Geometry* tmp{ nullptr };
		while (!buf.empty()) {
			tmp = buf.front();
			buf.pop_front();
			for (auto& child : tmp->getChildren())
				buf.push_back(child);
			addOne(tmp);
		}
	}


	void removeOne(Geometry* obj) { // ɾ��һ��Geometry���������Ӷ���
		GeometryRenderInfo& gInfo = objs[obj];
		for (auto& mInfo : gInfo.meshesInfo) {
			glDeleteVertexArrays(1, &mInfo.VAO);
			glDeleteBuffers(1, &mInfo.VBO);
			glDeleteBuffers(1, &mInfo.EBO);
		}
		objs.erase(obj);
	}

	void remove(Geometry* obj) {
		deque<Geometry*> buf{ obj };
		Geometry* tmp{ nullptr };
		while (!buf.empty()) {
			tmp = buf.front();
			buf.pop_front();
			for (auto& child : tmp->getChildren())
				buf.push_back(child);
			removeOne(tmp);
		}
	}

	void renderOne(Geometry* obj) {
		// ������Geometry�ӽڵ㣬ֻ���Ƶ�ǰGeometry
		Shader& shader = *shaders["default"];
		shader.use();
		shader["model"] = obj->getFinalOffset() * obj->model.getMatrix();
		shader["modelBuffer"] = obj->getModelBufferMatrix();
		// �������Ƿ���ڣ������������
		if (objs.count(obj) <= 0) {
			cout << "Geometry���󲻴��ڣ��ѽ������" << endl;
			add(obj);
		}
		GeometryRenderInfo& gInfo = objs[obj];
		if (obj->type == LEAF) {										// ������ͬGeometry���ͣ������䶥������
			Leaf* leaf_a = dynamic_cast<Leaf*>(obj);
			if (leaf_a->isMeshChanged()) {
				leaf_a->updateVertex();
				// ����VBO
				Mesh* m = leaf_a->getMeshes()[0];
				glNamedBufferSubData(gInfo.meshesInfo[0].VBO, 0, m->getVertexSize() * sizeof(Vertex), m->mapVertexData());
				m->unmapVertexData();
			}
		}
		for (auto& meshInfo : gInfo.meshesInfo) {
			// �����ڼ��볡����ʱ��Ҫ���������ݵ������Դ棬ͬʱ�����仺����ID�����֮�����ϵ��ʹ��map�洢
			glBindVertexArray(meshInfo.VAO);
			glDrawElements(GL_TRIANGLES, meshInfo.elementNum, GL_UNSIGNED_INT,0);
			glBindVertexArray(0);
		}
	}

	void render(Geometry* obj) {		// ����obj�����Ӷ���
		deque<Geometry*> buf{ obj };
		Geometry* tmp{ nullptr };
		while (!buf.empty()) {
			tmp = buf.front();
			buf.pop_front();
			for (auto& child : tmp->getChildren())
				buf.push_back(child);
			renderOne(tmp);
		}
	}

	void render() {	// ����objs�����ж���
		for(auto &obj : objs)
			renderOne(obj.first);
	}
};

#endif
