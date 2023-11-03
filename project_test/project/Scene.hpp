#include "proj.h"

#ifndef __WIND_SCENE
#define __WIND_SCENE

#include "Wheat.hpp"
#include "Geometry.hpp"
#include "Camera.hpp"
#include "Shader.hpp"

#ifdef TEST_OPENGL
typedef struct _MeshRenderInfo {
	GLuint VAO{ 0 };
	GLuint VBO{ 0 };
	GLuint EBO{ 0 };
	GLuint elementNum{ 0 };		// ����������Ƶ����
	bool isChanged{ false };		// �Ƿ���Ҫ���¶��㻺��
	unsigned int id{ 0 };				// ��������Geometry��meshes�е�����
	// ÿ��MeshӦ���и��Ե�shader����������չ
}MeshRenderInfo;

typedef struct _GeometryRenderInfo {
	vector<MeshRenderInfo> meshesInfo;
	GeometryType gtype{ GeometryType::DEFAULT };			// �����������
} GeometryRenderInfo;


class Scene {
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

	Scene(Camera* camera) :Scene() {
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
		if (objs.count(obj) > 0)
			return;		// �Ѿ����ڣ��������

		// ��obj��һ�μ���ʱ��Ϊ�䴴��һ��GeometryRenderInfo���洢����Ⱦ��Ϣ����ʼ�����㻺��
		GeometryRenderInfo gInfo;
		vector<Mesh*> meshes = obj->getMeshes();
		for (unsigned int k = 0; k < meshes.size(); k++) {
			Mesh* mesh = meshes[k];
			MeshRenderInfo mInfo;
			// ��������VAO
			glGenVertexArrays(1, &mInfo.VAO);
			glBindVertexArray(mInfo.VAO);
			// ��������VBO
			glGenBuffers(1, &mInfo.VBO);
			glBindBuffer(GL_ARRAY_BUFFER, mInfo.VBO);
			glBufferData(GL_ARRAY_BUFFER, mesh->getVertexSize() * sizeof(Vertex), mesh->getVertexPtr(), GL_DYNAMIC_DRAW);
			// ����VBO��������
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);								// λ������
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3)));			// ����������
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(vec3)));			// ��ɫ����
			glEnableVertexAttribArray(2);
			// ��������EBO
			glGenBuffers(1, &mInfo.EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mInfo.EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->getIndexSize() * sizeof(GLuint), mesh->getIndexPtr(), GL_STATIC_DRAW);
			// ��¼������������
			mInfo.elementNum = mesh->getIndexSize();
			mInfo.isChanged = true;
			mInfo.id = k;
			// ���뵽gInfo��
			gInfo.meshesInfo.push_back(mInfo);
		}
		objs[obj] = gInfo;
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
		//if (obj->type == LEAF) {										// ������ͬGeometry���ͣ������䶥������
		//	Leaf* leaf_a = dynamic_cast<Leaf*>(obj);
		//	if (leaf_a->isMeshChanged()) {
		//		leaf_a->updateVertex();
		//		// ����VBO
		//		Mesh* m = leaf_a->getMeshes()[0];
		//		glNamedBufferSubData(gInfo.meshesInfo[0].VBO, 0, m->getVertexSize() * sizeof(Vertex), m->getVertexPtr());
		//	}
		//}
		for (auto& meshInfo : gInfo.meshesInfo) {
			if (meshInfo.isChanged) {
				// ����������
				Mesh* mesh = obj->getMeshes()[meshInfo.id];
				//Vertex* vptr = mesh->getVertexPtr();
				//unsigned int uSize = mesh->getUSize(), vSize = mesh->getVSize();
				//mesh->upda
				
				// ����û�ж���Mesh���Զ����º���������ʵĿǰ����Leaf��������������������Ǿ�̬�ģ����߾�����ȫ��ͬ�Ŀ����߼�
				// �б�Ҫ����һ����󣬽�Mesh�ĸ��º����������


				meshInfo.isChanged = false;	// �������
			}
			// �����ڼ��볡����ʱ��Ҫ���������ݵ������Դ棬ͬʱ�����仺����ID�����֮�����ϵ��ʹ��map�洢
			glBindVertexArray(meshInfo.VAO);
			glDrawElements(GL_TRIANGLES, meshInfo.elementNum, GL_UNSIGNED_INT, 0);
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
		for (auto& obj : objs)
			renderOne(obj.first);
	}
};

#endif

#ifdef TEST_SOFT_RASTERIZATION
#include "Camera.hpp"
#include "Geometry.hpp"
#include "Light.hpp"

#include "TriangleGetter.hpp"

class Scene : public TriangleGetter {
private:
	Camera* camera{ nullptr };
public:
	set<Geometry*> objs;		// �洢����Geometry����Ƕ�׽ṹ�Ѿ�չ����
	set<Light*> lights;			// �洢����Light����

	Scene() = default;
	Scene(Camera* camera) :camera(camera) {}

	float step() {
		return 0.0f;		// û��ʵ�ּ���
	}
	// ����任
	vec2 world2screen(const vec3& v, bool* isCulled) {
		return world2screen(vec4(v, 1.0f), isCulled);
	}
	vec2 world2screen(const vec4& v, bool* isCulled) {
		vec4 pos = camera->getProjectionMatrix() * camera->getViewMatrix() * v;
		// ͸�ӳ���
		pos /= pos.w;
		// �ü��ж�
		if (pos.x > 1.0f || pos.x < -1.0f || pos.y>1.0f || pos.y < -1.0f || pos.z>1.0f || pos.z < -1.0f)
			if (isCulled != nullptr)
				*isCulled = true;
		// �ӿڱ任
		pos.x = (pos.x + 1.0f) * WIDTH / 2.0f;
		pos.y = (1.0f - pos.y) * HEIGHT / 2.0f;
		return vec2(pos.x, pos.y);
	}
	void add(Light* obj) {
		obj->setTriangleGetter(dynamic_cast<TriangleGetter*>(this));
		lights.insert(obj);
	}

	void addOne(Geometry* obj) {	// ���һ��Geometry���������Ӷ���
		objs.insert(obj);
	}

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

	//// ���ڱ�������Geometry��Mesh�� �Ѿ��任����������ϵ
	//vector<Mesh*> mapAllMeshes() {		// ÿ�ε��ö������¼�����������ϵ�µ�Mesh��Ч�ʽϵ�
	//	vector<Mesh*> meshes;
	//	for (auto& obj : objs){
	//		for (auto& mesh : obj->getWorldMeshes())	// ��ÿ��Geometry�м��㲢������������ϵ�µ�Mesh
	//			meshes.push_back(mesh);
	//	}
	//	return meshes;
	//}
	void getAllTriangles(vector<Triangle>& outTrianglesVector) {		// ����Scene������������Ԫ��ÿ�������ζ������궼������������ϵ��
		outTrianglesVector.clear();
		for (auto& obj : objs) {
			mat4 local2world = obj->getLocal2WorldMatrix();
			mat4 local2world_normal = transpose(inverse(local2world));
			for (auto& mesh : obj->getMeshes()) {
				Vertex* vtx = mesh->getVertexPtr();
				unsigned int vertexNum = mesh->getVertexSize();
				unsigned int* idx = mesh->getIndexPtr();
				unsigned int triNum = mesh->getIndexSize() / 3;
				// ����һ�ݱ任����������ϵ�µĶ�������
				Vertex* worldVtx = new Vertex[vertexNum];
				for (unsigned int i = 0; i < vertexNum; i++) {
					worldVtx[i].position = vec3(local2world * vec4(vtx[i].position, 1.0f));
					worldVtx[i].normal = vec3(local2world_normal * vec4(vtx[i].normal, 0.0f));
					worldVtx[i].color = vtx[i].color;
				}
				// ���뵽�������б���
				for (unsigned int i = 0; i < triNum; i++) {
					Triangle tt = {
						{worldVtx[idx[i * 3 + 0]], worldVtx[idx[i * 3 + 1]], worldVtx[idx[i * 3 + 2]]}
					};
					outTrianglesVector.push_back(tt);
				}
				delete[] worldVtx;
				worldVtx = nullptr;
			}
		}
	}

	void renderOne(Geometry* obj, Mat& canvas) {
		mat4 local2worldMatrix = obj->getLocal2WorldMatrix();
		for (auto& mesh : obj->getMeshes()) {
			// ����mesh������������
			vector<Triangle> triangles = mesh->getAllTriangles();
			for (auto& triangle : triangles) {
				// ����MVP�任��Ȼ�����������
				vec4 pos[3];
				Point2f p2f[3];
				bool isCulled = false;
				for (unsigned int k = 0; k < 3; k++) {
					pos[k] = local2worldMatrix * vec4(triangle.vertex[k].position, 1.0f);
					vec2 tmp = world2screen(pos[k], &isCulled);
					p2f[k] = Point2f(tmp.x, tmp.y);		// ת��Ϊopencv��line֧�ֵĲ�����ʽ
					if (isCulled)
						break;
				}
				if (isCulled)
					continue;
				line(canvas, p2f[2], p2f[0], Vec3f(0.0f, 0.0f, 1.0f), 1, cv::LINE_AA);
				line(canvas, p2f[0], p2f[1], Vec3f(1.0f, 0.0f, 0.0f), 1, cv::LINE_AA);
				line(canvas, p2f[1], p2f[2], Vec3f(0.0f, 1.0f, 0.0f), 1, cv::LINE_AA);
			}
		}
	}
	void render() {
		for (auto& obj : objs) {
			render(obj, canvas);
		}
		//for (auto& light : lights) {
		//	render(light, canvas);
		//}
	}
	//void render(Light* light, Mat& canvas) {
	//	// �����Ѿ������˹�Դ�����ͼ
	//	//light->genLightSample(8, 48);
	//	//light->genDepthMap();
	//	DepthMap dm = light->getDepthMap();
	//	vec3* lightPos = light->getLightSamplePos();
	//	vec3 lightDir = light->direction;
	//	unsigned int vSize = dm.height, uSize = dm.width;
	//	for (unsigned int v = 0; v < vSize; v++) {
	//		for (unsigned int u = 0; u < uSize; u++) {
	//			float depth = dm.ptr[v * uSize + u];
	//			depth = (depth == FLT_MAX) ? 5.0f : depth;
	//			vec3 p1 = lightPos[v * uSize + u];
	//			vec3 p2 = p1 + lightDir * depth;
	//			Point2f pt1 = toPoint2f(world2screen(p1, nullptr));
	//			Point2f pt2 = toPoint2f(world2screen(p2, nullptr));
	//			if (dm.ptr[v * uSize + u] == FLT_MAX) {
	//				line(canvas, pt1, pt2, Vec3f(0.0f, 1.0f, 0.0f), 1);
	//			}
	//			else {
	//				line(canvas, pt1, pt2, Vec3f(1.0f, 0.0f, 0.0f), 2);
	//			}
	//		}
	//	}

	//}
	void render(Geometry* obj, Mat& canvas) {		// ����obj�����Ӷ���
		deque<Geometry*> buf{ obj };
		Geometry* tmp{ nullptr };
		while (!buf.empty()) {
			tmp = buf.front();
			buf.pop_front();
			for (auto& child : tmp->getChildren())
				buf.push_back(child);
			renderOne(tmp, canvas);
}
	}

};
#endif

#endif
