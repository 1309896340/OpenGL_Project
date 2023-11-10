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
	GLuint elementNum{ 0 };		// 网格所需绘制点个数
	unsigned int id{ 0 };				// 该网格在Geometry的meshes中的索引

	// 每个Mesh应当有各自的shader，后续会扩展
	GLuint fluxBuffer{ 0 };			// 用于存储辐射通量的SSBO缓冲区，在加入Scene时初始化
}MeshRenderInfo;

typedef struct _GeometryRenderInfo {
	vector<MeshRenderInfo> meshesInfo;
	unsigned int id{ 0 };				// 该几何体在Scene中的唯一编号
	//GeometryType gtype{ GeometryType::DEFAULT };			// 几何体的类型
	float flux{ 0.0f };			// 几何体的辐射通量
} GeometryRenderInfo;

typedef struct _LightRenderInfo {
	GLuint UBO{ 0 };			// 用于存储光源信息的uniform缓冲区，在加入Scene时初始化

	// 用于渲染深度图的帧缓冲区
	GLuint FBO_depth{ 0 };
	GLuint texture_depth{ 0 };

	// 用于可视化光线的顶点缓冲区，采用glDrawArrays(GL_LINES, 0, 2)绘制
	GLuint VAO{ 0 };
	GLuint VBO{ 0 };
}LightRenderInfo;


class Scene {
	// Scene类与OpenGL高度耦合，其主要功能是管理场景中的所有对象，包括几何体、光源、相机、着色器
private:
	map<Geometry*, GeometryRenderInfo> objs;		// 在add时绑定一个新的GeometryRenderInfo，并初始化顶点缓冲
	map<Light*, LightRenderInfo> lights;		// 光源

	Shader* shader{ nullptr };		// 当前使用的shader，渲染过程中可能会切换shader
	Camera* camera{ 0 };				// 当前主相机
	GLuint ubo{ 0 };

	float lastTime{ 0 }, currentTime{ (float)glfwGetTime() }, deltaTime{ 0 };

	const GLuint matrixBindPoint = 0;		// projection和view接口块绑定点

public:
	map<std::string, Shader*> shaders;

	Scene() : currentTime((float)glfwGetTime()) {
		initShaders();					//  初始化所有Shader，编译、链接
		initUniformBuffer();			//  初始化uniform缓冲区
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
		shaders["default"] = new Shader("shader/shader.gvs", "shader/shader.gfs");		// 默认shader
		shaders["normal"] = new Shader("shader/normVisualize.gvs", "shader/normVisualize.ggs", "shader/normVisualize.gfs");	// 三角面元法线可视化
		shaders["normal_v"] = new Shader("shader/nshader.gvs", "shader/nshader.ggs", "shader/nshader.gfs");				// 顶点法线可视化	
		shaders["line"] = new Shader("shader/line.gvs", "shader/line.gfs");				// 绘制简单线条
		shaders["plane"] = new Shader("shader/plane.gvs", "shader/plane.gfs");		// 绘制简单平面
		shaders["leaf"] = new Shader("shader/leaf.gvs", "shader/leaf.gfs");		// 渲染小麦叶片的着色器，其中包含材质
		shaders["depthmap"] = new Shader("shader/depthmap.gvs", "shader/depthmap.gfs");		// 渲染深度图的着色器

		shaders["radiantFlux"] = new ComputeShader("shader/radiantFlux.gcs");
	}

	void initUniformBuffer() {
		// 初始化uniform缓冲区
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(mat4), NULL, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, matrixBindPoint, ubo);
	}

	float step(float* t = nullptr) { // 渲染循环中每一轮调用一次，更新视图变换矩阵，更新计时，并返回时间步长
		static float t_accum = 0.0f;
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), value_ptr(camera->getViewMatrix()));

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
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), value_ptr(camera->getProjectionMatrix()));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), value_ptr(camera->getViewMatrix()));
		//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void addLight(Light* light) {
		// 加入light时，为其创建一个LightInfo来存储其渲染信息
		// LightInfo应当用于存储深度图，当Light对象的属性(如位置、方向、分辨率)发生变化时，应当重新生成深度图
		LightRenderInfo info;
		unsigned int wNum, hNum;
		light->getResolution(&wNum, &hNum);
		{// 创建uniform缓冲区
			glGenBuffers(1, &info.UBO);
			glBindBuffer(GL_UNIFORM_BUFFER, info.UBO);
			glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(vec4) + sizeof(mat4), nullptr, GL_DYNAMIC_COPY);
			glBindBufferBase(GL_UNIFORM_BUFFER, 3, info.UBO);
			// 这个uniform buffer的结构
			// 这里可能存在很大的问题，主要是字节对齐，需要使用renderdoc进一步验证
		}
		{// 生成深度图
			// 创建纹理附件
			glGenTextures(1, &info.texture_depth);
			glBindTexture(GL_TEXTURE_2D, info.texture_depth);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, wNum, hNum, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);		// 创建空的深度纹理
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			// 生成帧缓冲对象，将纹理附件附加上去
			glGenFramebuffers(1, &info.FBO_depth);
			glBindFramebuffer(GL_FRAMEBUFFER, info.FBO_depth);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, info.texture_depth, 0);
			glDrawBuffer(GL_NONE);	// 不使用颜色缓冲区
			glReadBuffer(GL_NONE);		// 不使用颜色缓冲区
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
#ifdef TEST_RAY_TRACE
		//可视化光线（用于调试）
		{
			// 创建顶点缓冲区
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

	void addOne(Geometry* obj) {	// 添加一个Geometry，不考虑子对象
		if (objs.count(obj) > 0)
			return;		// 已经存在，不再添加

		// 在obj第一次加入时，为其创建一个GeometryRenderInfo来存储其渲染信息，初始化顶点缓冲
		GeometryRenderInfo gInfo;
		vector<Mesh*> meshes = obj->getMeshes();
		for (unsigned int k = 0; k < meshes.size(); k++) {
			Mesh* mesh = meshes[k];
			MeshRenderInfo mInfo;
			// 创建并绑定VAO
			glGenVertexArrays(1, &mInfo.VAO);
			glBindVertexArray(mInfo.VAO);
			// 创建并绑定VBO
			glGenBuffers(1, &mInfo.VBO);
			glBindBuffer(GL_ARRAY_BUFFER, mInfo.VBO);
			glBufferData(GL_ARRAY_BUFFER, mesh->getVertexSize() * sizeof(Vertex), mesh->getVertexPtr(), GL_DYNAMIC_DRAW);
			// 配置VBO的属性组
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);								// 位置属性
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3)));			// 法向量属性
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2 * sizeof(vec3)));			// 颜色属性
			glEnableVertexAttribArray(2);
			// 创建并绑定EBO
			glGenBuffers(1, &mInfo.EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mInfo.EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->getIndexSize() * sizeof(GLuint), mesh->getIndexPtr(), GL_STATIC_DRAW);
			// 如果需要计算辐射通量，则初始化存储计算结果的缓冲区
			if (obj->isNeedCalFlux()) {
				glGenBuffers(1, &mInfo.fluxBuffer);
				glBindBuffer(GL_SHADER_STORAGE_BUFFER, mInfo.fluxBuffer);
				glBufferData(GL_SHADER_STORAGE_BUFFER, mesh->getIndexSize() / 3 * sizeof(float), nullptr, GL_DYNAMIC_COPY);
			}
			// 记录其他网格属性
			mInfo.elementNum = mesh->getIndexSize();
			mInfo.id = k;
			// 加入到gInfo中
			gInfo.meshesInfo.push_back(mInfo);
		}
		gInfo.id = (unsigned int)objs.size();		// 将没加入该对象前的objs的数量作为该对象的id
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

	void removeOne(Geometry* obj) { // 删除一个Geometry，不考虑子对象
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

	// 该方法仅为Scene.render()调用，以渲染场景中所有Geometry对象。不考虑Geometry子节点，只绘制当前Geometry
	void renderOne(Geometry* obj) {
		(*shader)["model"] = obj->getFinalOffset() * obj->model.getMatrix();
		(*shader)["modelBuffer"] = obj->getModelBufferMatrix();
		// 检查对象是否存在，不存在则添加。（一般是不通过Scene.render()调用的对象会进入到这里）
		if (objs.count(obj) <= 0) {
			cout << "Geometry对象不存在，已进行添加" << endl;
			add(obj);
		}
		GeometryRenderInfo& gInfo = objs[obj];
		for (auto& meshinfo : gInfo.meshesInfo) {
			// 检查更新Mesh
			Mesh* mesh = obj->getMeshes()[meshinfo.id];
			if (mesh->isChanged()) {
				mesh->updateVertex();
				glNamedBufferSubData(meshinfo.VBO, 0, mesh->getVertexSize() * sizeof(Vertex), mesh->getVertexPtr());
				mesh->resetChangeFlag();		// 重置标志位
			}
			glBindVertexArray(meshinfo.VAO);
			glDrawElements(GL_TRIANGLES, meshinfo.elementNum, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}

#ifdef TEST_RAY_TRACE
	void visualizeRay() {
		// 用来调试Light的光线是否正确
		// 通过遍历Light对象指定分辨率图像上的每个像素，生成线段，连接发射面与被照射面，进行可视化

		// 这里只考虑第一个Light对象
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

		// 从绑定深度缓冲附件的帧缓冲中读取深度数据
		//glBindFramebuffer(GL_FRAMEBUFFER, lights[light].FBO_depth);
		//glReadPixels(0, 0, wNum, hNum, GL_DEPTH_COMPONENT, GL_FLOAT, depthDataPtr);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 从深度纹理中读取深度数据
		glBindTexture(GL_TEXTURE_2D, lights[light].texture_depth);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, depthDataPtr);
		//glGetTextureImage(lights[light].texture_depth, 0, GL_DEPTH_COMPONENT, GL_FLOAT, hNum * wNum, depthDataPtr);		// 不知道为什么这个不行

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
		(*shader)["color"] = vec4(1.0f, 0.0f, 0.0f, 1.0f);	// 绘制红线
		glBindVertexArray(lights[light].VAO);
		glDrawArrays(GL_LINES, 0, wNum * hNum * 2);
	}
#endif
	// 为每个光源生成深度贴图
	void updateDepthMap() {
		// 渲染场景中光源的深度图
		// 这里不用shaders["default"]，因为默认shader使用ubo来存储相机视角的view和projection矩阵
		// 而shaders["depthmap"]中直接使用光源获取的变换矩阵进行变换，和ubo无关
		shader = shaders["depthmap"];
		shader->use();
		for (auto& elem : lights) {	// 遍历所有Light对象
			Light* light = elem.first;

			assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);		// 验证FBO是否完整

			glBindFramebuffer(GL_FRAMEBUFFER, lights[light].FBO_depth);
			// 设置帧缓冲的视口，与该深度图的分辨率有关，清空深度缓冲
			unsigned int wNum, hNum;
			light->getResolution(&wNum, &hNum);
			glViewport(0, 0, wNum, hNum);
			glClear(GL_DEPTH_BUFFER_BIT);
			// 配置光源的变换矩阵
			(*shader)["lightSpaceMatrix"] = light->getProjectionViewMatrix();		// 不同的光源有不同的视角
			// 遍历场景中所有几何体，渲染深度图
			for (auto& elem : objs)
				renderOne(elem.first);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);		// 恢复默认帧缓冲(在主循环中swap的帧缓冲？)

#ifdef TEST_RAY_TRACE
		// 通过生成的深度贴图，可视化光线轨迹
		shader = shaders["line"];
		shader->use();
		glViewport(0, 0, WIDTH, HEIGHT);
		visualizeRay();
#endif
	}

	// 递归调度renderOne，以绘制objs中所有对象
	void render() {
		// 渲染场景中的几何体
		shader = shaders["default"];
		shader->use();
		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		for (auto& elem : objs)
			renderOne(elem.first);
	}

	// 计算场景中物体受到的辐射通量
	float computeRadiantFlux() {
		// 这里需要区分哪些是需要计算辐射接受量的物体，需要在构建场景的时候就打上标签
		// 有些物体只作为普通场景物体，如墙体等遮挡物，它们不需要计算辐射接受量，但是要用来构造深度图
		// 需要将这个标签记录为Geometry的一个属性
		ComputeShader* sd = static_cast<ComputeShader*>(shaders["radiantFlux"]);
		sd->use();
		// 载入光源信息（这里只考虑第一个光源，而且对所有场景中的问题而言，光源是相同的）
		Light* light = lights.begin()->first;
		// 由于使用了uniform块，这里不能用glUniformXXX()的方式来传递数据，而是要用glBufferSubData()的方式
		//(*sd)["lightPos"] = light->getPosition();
		//(*sd)["lightDir"] = light->getDirection();
		//(*sd)["lightColor"] = vec3(1.0f, 1.0f, 1.0f);
		//(*sd)["lightIntensity"] = light->getIntensity();
		//(*sd)["lightViewProjectionMatrix"] = light->getProjectionViewMatrix();
		// 向uniform块中传递数据
		glNamedBufferSubData(lights[light].UBO, 0, sizeof(vec3), value_ptr(light->getPosition()));
		glNamedBufferSubData(lights[light].UBO, sizeof(vec4), sizeof(vec3), value_ptr(light->getDirection()));
		glNamedBufferSubData(lights[light].UBO, 2 * sizeof(vec4), sizeof(vec3), value_ptr(light->getColor()));
		float intensity = light->getIntensity();
		glNamedBufferSubData(lights[light].UBO, 2 * sizeof(vec4) + sizeof(vec3), sizeof(float), &intensity);
		glNamedBufferSubData(lights[light].UBO, 3 * sizeof(vec4), sizeof(mat4), value_ptr(light->getProjectionViewMatrix()));

		// 将light的深度贴图绑定到纹理单元0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, lights[light].texture_depth);

		float fluxSum = 0.0f;
		// 遍历所有几何体
		for (auto& elem : objs) {
			Geometry* obj = elem.first;
			if (!(obj->isNeedCalFlux()))
				continue;
			// 载入当前几何体的model矩阵(包括modelBuffer)
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
				// 这里可以调试某个Geometry的某个Mesh的辐射通量：tmp
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
	set<Geometry*> objs;		// 存储所有Geometry对象（嵌套结构已经展开）
	set<Light*> lights;			// 存储所有Light对象

	Scene() = default;
	Scene(Camera* camera) :camera(camera) {}

	float step() {
		return 0.0f;		// 没有实现计数
	}
	// 坐标变换
	vec2 world2screen(const vec3& v, bool* isCulled) {
		return world2screen(vec4(v, 1.0f), isCulled);
	}
	vec2 world2screen(const vec4& v, bool* isCulled) {
		vec4 pos = camera->getProjectionMatrix() * camera->getViewMatrix() * v;
		// 透视除法
		pos /= pos.w;
		// 裁剪判断
		if (pos.x > 1.0f || pos.x < -1.0f || pos.y>1.0f || pos.y < -1.0f || pos.z>1.0f || pos.z < -1.0f)
			if (isCulled != nullptr)
				*isCulled = true;
		// 视口变换
		pos.x = (pos.x + 1.0f) * WIDTH / 2.0f;
		pos.y = (1.0f - pos.y) * HEIGHT / 2.0f;
		return vec2(pos.x, pos.y);
	}
	void add(Light* obj) {
		obj->setTriangleGetter(dynamic_cast<TriangleGetter*>(this));
		lights.insert(obj);
	}

	void addOne(Geometry* obj) {	// 添加一个Geometry，不考虑子对象
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

	//// 用于遍历所有Geometry的Mesh， 已经变换到世界坐标系
	//vector<Mesh*> mapAllMeshes() {		// 每次调用都会重新计算世界坐标系下的Mesh，效率较低
	//	vector<Mesh*> meshes;
	//	for (auto& obj : objs){
	//		for (auto& mesh : obj->getWorldMeshes())	// 在每个Geometry中计算并生成世界坐标系下的Mesh
	//			meshes.push_back(mesh);
	//	}
	//	return meshes;
	//}
	void getAllTriangles(vector<Triangle>& outTrianglesVector) {		// 遍历Scene中所有三角面元，每个三角形顶点坐标都处于世界坐标系下
		outTrianglesVector.clear();
		for (auto& obj : objs) {
			mat4 local2world = obj->getLocal2WorldMatrix();
			mat4 local2world_normal = transpose(inverse(local2world));
			for (auto& mesh : obj->getMeshes()) {
				Vertex* vtx = mesh->getVertexPtr();
				unsigned int vertexNum = mesh->getVertexSize();
				unsigned int* idx = mesh->getIndexPtr();
				unsigned int triNum = mesh->getIndexSize() / 3;
				// 拷贝一份变换到世界坐标系下的顶点数据
				Vertex* worldVtx = new Vertex[vertexNum];
				for (unsigned int i = 0; i < vertexNum; i++) {
					worldVtx[i].position = vec3(local2world * vec4(vtx[i].position, 1.0f));
					worldVtx[i].normal = vec3(local2world_normal * vec4(vtx[i].normal, 0.0f));
					worldVtx[i].color = vtx[i].color;
				}
				// 加入到三角形列表中
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
			// 遍历mesh中所有三角形
			vector<Triangle> triangles = mesh->getAllTriangles();
			for (auto& triangle : triangles) {
				// 进行MVP变换，然后绘制连接线
				vec4 pos[3];
				Point2f p2f[3];
				bool isCulled = false;
				for (unsigned int k = 0; k < 3; k++) {
					pos[k] = local2worldMatrix * vec4(triangle.vertex[k].position, 1.0f);
					vec2 tmp = world2screen(pos[k], &isCulled);
					p2f[k] = Point2f(tmp.x, tmp.y);		// 转换为opencv的line支持的参数格式
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
	//	// 假设已经生成了光源的深度图
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
	void render(Geometry* obj, Mat& canvas) {		// 绘制obj及其子对象
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
