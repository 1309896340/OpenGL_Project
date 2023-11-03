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
	GLuint elementNum{ 0 };		// 网格所需绘制点个数
	bool isChanged{ false };		// 是否需要更新顶点缓冲
	unsigned int id{ 0 };				// 该网格在Geometry的meshes中的索引
	// 每个Mesh应当有各自的shader，后续会扩展
}MeshRenderInfo;

typedef struct _GeometryRenderInfo {
	vector<MeshRenderInfo> meshesInfo;
	GeometryType gtype{ GeometryType::DEFAULT };			// 几何体的类型
} GeometryRenderInfo;


class Scene {
private:
	std::map<Geometry*, GeometryRenderInfo> objs;		// 在add时绑定一个新的GeometryRenderInfo，并初始化顶点缓冲
	Camera* camera{ 0 }; // 当前主相机
	GLuint ubo{ 0 };

	float lastTime{ 0 }, currentTime{ (float)glfwGetTime() }, deltaTime{ 0 };

	const GLuint matrixBindPoint = 0;		// projection和view接口块绑定点

public:
	std::map<std::string, Shader*> shaders;

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
			// 记录其他网格属性
			mInfo.elementNum = mesh->getIndexSize();
			mInfo.isChanged = true;
			mInfo.id = k;
			// 加入到gInfo中
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

	void renderOne(Geometry* obj) {
		// 不考虑Geometry子节点，只绘制当前Geometry
		Shader& shader = *shaders["default"];
		shader.use();
		shader["model"] = obj->getFinalOffset() * obj->model.getMatrix();
		shader["modelBuffer"] = obj->getModelBufferMatrix();
		// 检查对象是否存在，不存在则添加
		if (objs.count(obj) <= 0) {
			cout << "Geometry对象不存在，已进行添加" << endl;
			add(obj);
		}
		GeometryRenderInfo& gInfo = objs[obj];
		//if (obj->type == LEAF) {										// 检索不同Geometry类型，更新其顶点数据
		//	Leaf* leaf_a = dynamic_cast<Leaf*>(obj);
		//	if (leaf_a->isMeshChanged()) {
		//		leaf_a->updateVertex();
		//		// 更新VBO
		//		Mesh* m = leaf_a->getMeshes()[0];
		//		glNamedBufferSubData(gInfo.meshesInfo[0].VBO, 0, m->getVertexSize() * sizeof(Vertex), m->getVertexPtr());
		//	}
		//}
		for (auto& meshInfo : gInfo.meshesInfo) {
			if (meshInfo.isChanged) {
				// 检查更新网格
				Mesh* mesh = obj->getMeshes()[meshInfo.id];
				//Vertex* vptr = mesh->getVertexPtr();
				//unsigned int uSize = mesh->getUSize(), vSize = mesh->getVSize();
				//mesh->upda
				
				// 由于没有定义Mesh的自动更新函数，但其实目前除了Leaf类以外其他对象的网格都是静态的，两者具有完全不同的控制逻辑
				// 有必要进行一层抽象，将Mesh的更新函数抽象出来


				meshInfo.isChanged = false;	// 更新完毕
			}
			// 物体在加入场景中时需要将顶点数据等载入显存，同时建立其缓冲区ID与对象之间的联系，使用map存储
			glBindVertexArray(meshInfo.VAO);
			glDrawElements(GL_TRIANGLES, meshInfo.elementNum, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}

	void render(Geometry* obj) {		// 绘制obj及其子对象
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

	void render() {	// 绘制objs中所有对象
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
