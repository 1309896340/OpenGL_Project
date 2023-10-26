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
	GLuint elementNum;	// 网格所需绘制点个数
	// 每个Mesh应当有各自的shader，后续会扩展
}MeshRenderInfo;

typedef struct {
	vector<MeshRenderInfo> meshesInfo;
	//每个Mesh应当还有其他属性，后续会扩展
} GeometryRenderInfo;


class Scene {
	// 目前来看 Scene类是一个管理器，管理所有的Shader、Geometry、Camera
	// 但它不是单向依赖的，Geometry需要Scene来获取一个默认shader
	// 那就干脆把这个特殊的shader拿出来声明为一个全局变量，这样Geometry就不需要依赖Scene了
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
		GeometryRenderInfo gInfo;
		vector<Mesh*> meshes = obj->getMeshes();
		for (auto& mesh : meshes) {
			MeshRenderInfo mInfo;
			// 创建并绑定VAO
			glGenVertexArrays(1, &mInfo.VAO);
			glBindVertexArray(mInfo.VAO);
			// 创建并绑定VBO
			glGenBuffers(1, &mInfo.VBO);
			glBindBuffer(GL_ARRAY_BUFFER, mInfo.VBO);
			glBufferData(GL_ARRAY_BUFFER, mesh->getVertexSize() * sizeof(Vertex), mesh->mapVertexData(), GL_DYNAMIC_DRAW);
			mesh->unmapVertexData();
			// 配置VBO的属性组
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);								// 位置属性
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(vec3)));			// 法向量属性
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(2*sizeof(vec3)));			// 颜色属性
			glEnableVertexAttribArray(2);
			// 创建并绑定EBO
			glGenBuffers(1, &mInfo.EBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mInfo.EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->getIndexSize() * sizeof(GLuint), mesh->getIndexPtr(), GL_STATIC_DRAW);
			// 设置顶点索引数
			mInfo.elementNum = mesh->getIndexSize();
			// 加入到gInfo中
			gInfo.meshesInfo.push_back(mInfo);
		}
		objs[obj] = gInfo;		// 存入map
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
		if (obj->type == LEAF) {										// 检索不同Geometry类型，更新其顶点数据
			Leaf* leaf_a = dynamic_cast<Leaf*>(obj);
			if (leaf_a->isMeshChanged()) {
				leaf_a->updateVertex();
				// 更新VBO
				Mesh* m = leaf_a->getMeshes()[0];
				glNamedBufferSubData(gInfo.meshesInfo[0].VBO, 0, m->getVertexSize() * sizeof(Vertex), m->mapVertexData());
				m->unmapVertexData();
			}
		}
		for (auto& meshInfo : gInfo.meshesInfo) {
			// 物体在加入场景中时需要将顶点数据等载入显存，同时建立其缓冲区ID与对象之间的联系，使用map存储
			glBindVertexArray(meshInfo.VAO);
			glDrawElements(GL_TRIANGLES, meshInfo.elementNum, GL_UNSIGNED_INT,0);
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
		for(auto &obj : objs)
			renderOne(obj.first);
	}
};

#endif
