#include "proj.h"

#ifndef __WIND_SCENE
#define __WIND_SCENE

#include "Wheat.hpp"
#include "Geometry.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include "Light.hpp"

#ifdef TEST_OPENGL
typedef struct _MeshRenderInfo {
	GLuint VAO{ 0 };
	GLuint VBO{ 0 };
	GLuint EBO{ 0 };
	GLuint elementNum{ 0 };		// ����������Ƶ����
	unsigned int id{ 0 };				// ��������Geometry��meshes�е�����

	// ÿ��MeshӦ���и��Ե�shader����������չ
	GLuint fluxBuffer{ 0 };			// ���ڴ洢����ͨ����SSBO���������ڼ���Sceneʱ��ʼ��
}MeshRenderInfo;

typedef struct _GeometryRenderInfo {
	vector<MeshRenderInfo> meshesInfo;
	unsigned int id{ 0 };				// �ü�������Scene�е�Ψһ���
	//GeometryType gtype{ GeometryType::DEFAULT };			// �����������
	float flux{ 0.0f };			// ������ķ���ͨ��
} GeometryRenderInfo;

typedef struct _LightRenderInfo {
	GLuint UBO{ 0 };			// ���ڴ洢��Դ��Ϣ��uniform���������ڼ���Sceneʱ��ʼ��

	// ������Ⱦ���ͼ��֡������
	GLuint FBO_depth{ 0 };
	GLuint texture_depth{ 0 };

	// ���ڿ��ӻ����ߵĶ��㻺����������glDrawArrays(GL_LINES, 0, 2)����
	GLuint VAO{ 0 };
	GLuint VBO{ 0 };
}LightRenderInfo;


class Scene {
	// Scene����OpenGL�߶���ϣ�����Ҫ�����ǹ������е����ж��󣬰��������塢��Դ���������ɫ��
private:
	map<Geometry*, GeometryRenderInfo> objs;		// ��addʱ��һ���µ�GeometryRenderInfo������ʼ�����㻺��
	map<Light*, LightRenderInfo> lights;		// ��Դ

	Shader* shader{ nullptr };		// ��ǰʹ�õ�shader����Ⱦ�����п��ܻ��л�shader
	Camera* camera{ 0 };				// ��ǰ�����
	GLuint ubo{ 0 };

	float lastTime{ 0 }, currentTime{ (float)glfwGetTime() }, deltaTime{ 0 };

	const GLuint matrixBindPoint = 0;		// projection��view�ӿڿ�󶨵�

public:
	map<std::string, Shader*> shaders;

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
		shaders["depthmap"] = new Shader("shader/depthmap.gvs", "shader/depthmap.gfs");		// ��Ⱦ���ͼ����ɫ��

		shaders["radiantFlux"] = new ComputeShader("shader/radiantFlux.gcs");
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

	void addLight(Light* light) {
		// ����lightʱ��Ϊ�䴴��һ��LightInfo���洢����Ⱦ��Ϣ
		// LightInfoӦ�����ڴ洢���ͼ����Light���������(��λ�á����򡢷ֱ���)�����仯ʱ��Ӧ�������������ͼ
		LightRenderInfo info;
		unsigned int wNum, hNum;
		light->getResolution(&wNum, &hNum);
		{// ����uniform������
			glGenBuffers(1, &info.UBO);
			glBindBuffer(GL_UNIFORM_BUFFER, info.UBO);
			glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(vec4) + sizeof(mat4), nullptr, GL_DYNAMIC_COPY);
			glBindBufferBase(GL_UNIFORM_BUFFER, 3, info.UBO);
			// ���uniform buffer�Ľṹ
			// ������ܴ��ںܴ�����⣬��Ҫ���ֽڶ��룬��Ҫʹ��renderdoc��һ����֤
		}
		{// �������ͼ
			// ����������
			glGenTextures(1, &info.texture_depth);
			glBindTexture(GL_TEXTURE_2D, info.texture_depth);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, wNum, hNum, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);		// �����յ��������
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// ����֡������󣬽�������������ȥ
			glGenFramebuffers(1, &info.FBO_depth);
			glBindFramebuffer(GL_FRAMEBUFFER, info.FBO_depth);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, info.texture_depth, 0);
			glDrawBuffer(GL_NONE);	// ��ʹ����ɫ������
			glReadBuffer(GL_NONE);		// ��ʹ����ɫ������
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
#ifdef TEST_RAY_TRACE
		//���ӻ����ߣ����ڵ��ԣ�
		{
			// �������㻺����
			glGenVertexArrays(1, &info.VAO);
			glBindVertexArray(info.VAO);
			glGenBuffers(1, &info.VBO);
			glBindBuffer(GL_ARRAY_BUFFER, info.VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * (wNum * hNum * 2), nullptr, GL_DYNAMIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, NULL, nullptr);
			glEnableVertexAttribArray(0);
		}
#endif
		lights[light] = info;
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
			// �����Ҫ�������ͨ�������ʼ���洢�������Ļ�����
			if (obj->isNeedCalFlux()) {
				glGenBuffers(1, &mInfo.fluxBuffer);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, mInfo.fluxBuffer);
				glBufferData(GL_SHADER_STORAGE_BUFFER, mesh->getIndexSize() / 3 * sizeof(float), nullptr, GL_DYNAMIC_COPY);
			}
			// ��¼������������
			mInfo.elementNum = mesh->getIndexSize();
			mInfo.id = k;
			// ���뵽gInfo��
			gInfo.meshesInfo.push_back(mInfo);
		}
		gInfo.id = (unsigned int)objs.size();		// ��û����ö���ǰ��objs��������Ϊ�ö����id
		objs[obj] = gInfo;
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

	// �÷�����ΪScene.render()���ã�����Ⱦ����������Geometry���󡣲�����Geometry�ӽڵ㣬ֻ���Ƶ�ǰGeometry
	void renderOne(Geometry* obj) {
		(*shader)["model"] = obj->getFinalOffset() * obj->model.getMatrix();
		(*shader)["modelBuffer"] = obj->getModelBufferMatrix();
		// �������Ƿ���ڣ�����������ӡ���һ���ǲ�ͨ��Scene.render()���õĶ������뵽���
		if (objs.count(obj) <= 0) {
			cout << "Geometry���󲻴��ڣ��ѽ������" << endl;
			add(obj);
		}
		GeometryRenderInfo& gInfo = objs[obj];
		for (auto& meshinfo : gInfo.meshesInfo) {
			// ������Mesh
			Mesh* mesh = obj->getMeshes()[meshinfo.id];
			if (mesh->isChanged()) {
				mesh->updateVertex();
				glNamedBufferSubData(meshinfo.VBO, 0, mesh->getVertexSize() * sizeof(Vertex), mesh->getVertexPtr());
				mesh->resetChangeFlag();		// ���ñ�־λ
			}
			glBindVertexArray(meshinfo.VAO);
			glDrawElements(GL_TRIANGLES, meshinfo.elementNum, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}

#ifdef TEST_RAY_TRACE
	void visualizeRay() {
		// ��������Light�Ĺ����Ƿ���ȷ
		// ͨ������Light����ָ���ֱ���ͼ���ϵ�ÿ�����أ������߶Σ����ӷ������뱻�����棬���п��ӻ�

		// ����ֻ���ǵ�һ��Light����
		Light* light = lights.begin()->first;
		unsigned int wNum, hNum;
		float width, height, near, far;
		light->getResolution(&wNum, &hNum);
		light->getFieldSize(&width, &height, &near, &far);
		float dw = width / wNum, dh = height / hNum;

		vec3 lightDir = light->getDirection();
		vec3 lightPos = light->getPosition();
		vec3 lightRight = normalize(cross(lightDir, _up));
		vec3 lightUp = normalize(cross(lightRight, lightDir));

		float* depthDataPtr = new float[wNum * hNum];

		// �Ӱ���Ȼ��帽����֡�����ж�ȡ�������
		//glBindFramebuffer(GL_FRAMEBUFFER, lights[light].FBO_depth);
		//glReadPixels(0, 0, wNum, hNum, GL_DEPTH_COMPONENT, GL_FLOAT, depthDataPtr);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// ����������ж�ȡ�������
		glBindTexture(GL_TEXTURE_2D, lights[light].texture_depth);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, depthDataPtr);
		//glGetTextureImage(lights[light].texture_depth, 0, GL_DEPTH_COMPONENT, GL_FLOAT, hNum * wNum, depthDataPtr);		// ��֪��Ϊʲô�������

		//mat4 trans = light->getProjectionViewMatrix();
		vec3 lineSeg[2];
		for (unsigned int h = 0; h < hNum; h++) {
			for (unsigned int w = 0; w < wNum; w++) {
				float depth = depthDataPtr[h * wNum + w] * (far - near) + near;
				//depth = 2.0f * near * far / (far + near - depth * (far - near));
				lineSeg[0] = lightPos + lightRight * (-width / 2.0f + dw / 2.0f + dw * w) + lightUp * (-height / 2.0f + dh / 2.0f + dh * h);
				lineSeg[1] = lineSeg[0] + lightDir * depth;
				if (depth > 13.0f)
					lineSeg[1] = lineSeg[0] + lightDir * 0.001f;
				//lineSeg[1] = vec3(0.0f, 0.0f, 0.0f);
				//glBindBuffer(GL_ARRAY_BUFFER, lights[light].VBO);
				//glBufferSubData(GL_ARRAY_BUFFER, (h * wNum + w) * 2 * sizeof(vec3), sizeof(vec3) * 2, lineSeg);
				glNamedBufferSubData(lights[light].VBO, (h * wNum + w) * 2 * sizeof(vec3), sizeof(vec3) * 2, lineSeg);
			}
		}
		delete[] depthDataPtr;
		(*shader)["color"] = vec4(1.0f, 0.0f, 0.0f, 1.0f);	// ���ƺ���
		glBindVertexArray(lights[light].VAO);
		glDrawArrays(GL_LINES, 0, wNum * hNum * 2);
	}
#endif
	// Ϊÿ����Դ���������ͼ
	void updateDepthMap() {
		// ��Ⱦ�����й�Դ�����ͼ
		// ���ﲻ��shaders["default"]����ΪĬ��shaderʹ��ubo���洢����ӽǵ�view��projection����
		// ��shaders["depthmap"]��ֱ��ʹ�ù�Դ��ȡ�ı任������б任����ubo�޹�
		shader = shaders["depthmap"];
		shader->use();
		for (auto& elem : lights) {	// ��������Light����
			Light* light = elem.first;

			assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);		// ��֤FBO�Ƿ�����

			glBindFramebuffer(GL_FRAMEBUFFER, lights[light].FBO_depth);
			// ����֡������ӿڣ�������ͼ�ķֱ����йأ������Ȼ���
			unsigned int wNum, hNum;
			light->getResolution(&wNum, &hNum);
			glViewport(0, 0, wNum, hNum);
			glClear(GL_DEPTH_BUFFER_BIT);
			// ���ù�Դ�ı任����
			(*shader)["lightSpaceMatrix"] = light->getProjectionViewMatrix();		// ��ͬ�Ĺ�Դ�в�ͬ���ӽ�
			// �������������м����壬��Ⱦ���ͼ
			for (auto& elem : objs)
				renderOne(elem.first);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);		// �ָ�Ĭ��֡����(����ѭ����swap��֡���壿)

#ifdef TEST_RAY_TRACE
		// ͨ�����ɵ������ͼ�����ӻ����߹켣
		shader = shaders["line"];
		shader->use();
		glViewport(0, 0, WIDTH, HEIGHT);
		visualizeRay();
#endif
	}

	// �ݹ����renderOne���Ի���objs�����ж���
	void render() {
		// ��Ⱦ�����еļ�����
		shader = shaders["default"];
		shader->use();
		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		for (auto& elem : objs)
			renderOne(elem.first);
	}

	// ���㳡���������ܵ��ķ���ͨ��
	float computeRadiantFlux() {
		// ������Ҫ������Щ����Ҫ�����������������壬��Ҫ�ڹ���������ʱ��ʹ��ϱ�ǩ
		// ��Щ����ֻ��Ϊ��ͨ�������壬��ǽ����ڵ�����ǲ���Ҫ������������������Ҫ�����������ͼ
		// ��Ҫ�������ǩ��¼ΪGeometry��һ������
		ComputeShader* sd = static_cast<ComputeShader*>(shaders["radiantFlux"]);
		sd->use();
		// �����Դ��Ϣ������ֻ���ǵ�һ����Դ�����Ҷ����г����е�������ԣ���Դ����ͬ�ģ�
		Light* light = lights.begin()->first;
		// ����ʹ����uniform�飬���ﲻ����glUniformXXX()�ķ�ʽ���������ݣ�����Ҫ��glBufferSubData()�ķ�ʽ
		//(*sd)["lightPos"] = light->getPosition();
		//(*sd)["lightDir"] = light->getDirection();
		//(*sd)["lightColor"] = vec3(1.0f, 1.0f, 1.0f);
		//(*sd)["lightIntensity"] = light->getIntensity();
		//(*sd)["lightViewProjectionMatrix"] = light->getProjectionViewMatrix();
		// ��uniform���д�������
		glNamedBufferSubData(lights[light].UBO, 0, sizeof(vec3), value_ptr(light->getPosition()));
		glNamedBufferSubData(lights[light].UBO, sizeof(vec4), sizeof(vec3), value_ptr(light->getDirection()));
		glNamedBufferSubData(lights[light].UBO, 2 * sizeof(vec4), sizeof(vec3), value_ptr(light->getColor()));
		float intensity = light->getIntensity();
		glNamedBufferSubData(lights[light].UBO, 2 * sizeof(vec4) + sizeof(vec3), sizeof(float), &intensity);
		glNamedBufferSubData(lights[light].UBO, 3 * sizeof(vec4), sizeof(mat4), value_ptr(light->getProjectionViewMatrix()));

		// ��light�������ͼ�󶨵�����Ԫ0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, lights[light].texture_depth);

		float fluxSum = 0.0f;
		// �������м�����
		for (auto& elem : objs) {
			Geometry* obj = elem.first;
			if (!(obj->isNeedCalFlux()))
				continue;
			// ���뵱ǰ�������model����(����modelBuffer)
			(*sd)["meshModelMatrix"] = obj->getFinalOffset() * obj->model.getMatrix() * obj->getModelBufferMatrix();
			GeometryRenderInfo& ginfo = elem.second;
			ginfo.flux = 0.0f;
			for (unsigned int i = 0; i < ginfo.meshesInfo.size(); i++) {
				MeshRenderInfo& meshinfo = ginfo.meshesInfo[i];
				Mesh* mesh = obj->getMeshes()[meshinfo.id];
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, meshinfo.VBO);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, meshinfo.EBO);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, meshinfo.fluxBuffer);

				glDispatchCompute(2, mesh->getUSize(), mesh->getVSize());
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshinfo.fluxBuffer);
				float* fluxPtr = (float*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
				float tmp = 0.0f;
				for (unsigned int k = 0; k < mesh->getIndexSize() / 3; k++) {
					tmp += fluxPtr[k];
				}
				// ������Ե���ĳ��Geometry��ĳ��Mesh�ķ���ͨ����tmp
				printf("Goemetry id: %d  Mesh id: %d  flux: %.2f \n", ginfo.id, meshinfo.id, tmp);

				ginfo.flux += tmp;
				glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
			}
			fluxSum += ginfo.flux;
		}
		return fluxSum;
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
