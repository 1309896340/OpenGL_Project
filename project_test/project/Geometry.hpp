#ifndef __WIND_GEOMETRY
#define __WIND_GEOMETRY

#include "utils.h"
#include "proj.h"
#include "Shader.hpp"

class Transform {
private:
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 _scale;
public:
	Transform() :position(glm::vec3(0.0f)), _scale(glm::vec3(1.0f)), rotation(glm::identity<glm::quat>()) {}
	Transform(glm::vec3 position) :position(position), _scale(glm::vec3(1.0f)), rotation(glm::identity<glm::quat>()) {}

	void scale(glm::vec3 xyz) {
		_scale *= xyz;
	}
	void scaleTo(glm::vec3 xyz) {
		_scale = xyz;
	}
	void rotate(float angle, glm::vec3 axis) {
		rotation = glm::angleAxis(angle, glm::normalize(axis)) * rotation;
	}
	void translate(glm::vec3 dxyz) {
		position = position + dxyz;
	}
	void translateTo(glm::vec3 dxyz) {
		position = dxyz;
	}
	void reset() {
		position = glm::vec3(0.0f);
		_scale = glm::vec3(1.0f);
		rotation = glm::identity<glm::quat>();
	}
	glm::quat getRotation() {
		return rotation;
	}
	glm::mat4 getMatrix() {
		glm::mat4 Tmat(1.0f);
		Tmat = glm::translate(Tmat, position);
		Tmat = Tmat * glm::mat4_cast(rotation);
		Tmat = glm::scale(Tmat, _scale);
		return Tmat;
	}
};

class Drawable {
public:
	virtual void draw() = 0;
};

class Mesh : public Drawable {
	// 通过内存映射直接操作VBO和EBO
private:
	unsigned int uSize{ 0 }, vSize{ 0 };		// u为第几行，v为第几列
	GLuint VAO{ 0 }, VBO[3]{ 0,0,0 };

	Shader* shader{ nullptr };
public:
	Mesh(unsigned int uSize = 2, unsigned int vSize = 2) :uSize(uSize), vSize(vSize) {
		if (!shader)
			shader = DefaultShader::getDefaultShader();

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(3, VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, getVertexSize() * sizeof(vec3), nullptr, GL_DYNAMIC_DRAW);		// position
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
		glBufferData(GL_ARRAY_BUFFER, getVertexSize() * sizeof(vec3), nullptr, GL_DYNAMIC_DRAW);		// normal
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, getIndexSize() * sizeof(GLuint), nullptr, GL_STATIC_DRAW);		// index

		glBindVertexArray(0);
	}
	~Mesh() {
		glDeleteBuffers(3, VBO);
		glDeleteVertexArrays(1, &VAO);
	}
	void connect() {
		// 默认以行优先连接（指第1行连完后开始第2行）
		GLuint* ptr;
		GLuint* index_ptr = (GLuint*)glMapNamedBuffer(VBO[2], GL_WRITE_ONLY);
		assert(index_ptr);
		for (unsigned int i = 0; i < uSize - 1; i++) {
			for (unsigned int j = 0; j < vSize - 1; j++) {
				ptr = index_ptr + (i * (vSize - 1) + j) * 6;	// 采用逆时针绕行
				*ptr++ = i * vSize + j;						// 左上
				*ptr++ = (i + 1) * vSize + j;				// 左下
				*ptr++ = (i + 1) * vSize + j + 1;		// 右下
				*ptr++ = i * vSize + j;						// 左上
				*ptr++ = (i + 1) * vSize + j + 1;		// 右下
				*ptr++ = i * vSize + j + 1;				// 右上
			}
		}
		glUnmapNamedBuffer(VBO[2]);
	}
	void updateVertexPositionByFunc(std::function<vec3(unsigned int, unsigned int)> func) {
		vec3* vbo_ptr = (vec3*)glMapNamedBuffer(VBO[0], GL_WRITE_ONLY);
		assert(vbo_ptr);
		for (unsigned int i = 0; i < uSize; i++) {
			for (unsigned int j = 0; j < vSize; j++) {
				vbo_ptr[i * vSize + j] = func(i, j);
			}
		}
		glUnmapNamedBuffer(VBO[0]);
	}
	void updateVertexNormalByFunc(std::function<vec3(unsigned int, unsigned int)> func) {
		vec3* normal_ptr = (vec3*)glMapNamedBuffer(VBO[1], GL_WRITE_ONLY);
		assert(normal_ptr);
		for (unsigned int i = 0; i < uSize; i++) {
			for (unsigned int j = 0; j < vSize; j++) {
				normal_ptr[i * vSize + j] = func(i, j);
			}
		}
		glUnmapNamedBuffer(VBO[1]);
	}

	void updateVertexNormalByFunc(std::function<vec3(float, float, float)> func) {
		vec3* vbo_ptr = (vec3*)glMapNamedBuffer(VBO[0], GL_READ_ONLY);
		vec3* normal_ptr = (vec3*)glMapNamedBuffer(VBO[1], GL_WRITE_ONLY);
		assert(vbo_ptr);
		assert(normal_ptr);
		vec3 pos;
		for (unsigned int i = 0; i < uSize; i++) {
			for (unsigned int j = 0; j < vSize; j++) {
				pos = vbo_ptr[i * vSize + j];
				normal_ptr[i * vSize + j] = func(pos.x, pos.y, pos.z);
			}
		}
		glUnmapNamedBuffer(VBO[0]);
		glUnmapNamedBuffer(VBO[1]);
	}
	GLuint getVAO() {
		return VAO;
	}
	unsigned int getVertexSize() {
		return uSize * vSize;
	}
	unsigned int getIndexSize() {
		return (uSize - 1) * (vSize - 1) * 6;
	}

	virtual void draw() {
		shader->use();
		(*shader)["modelBuffer"] = glm::mat4(1.0f);
		(*shader)["model"] = glm::mat4(1.0f);
		shader->loadAttribute({ true, glm::vec4(0.0f,0.0f,0.0f,0.0f) });
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, getIndexSize(), GL_UNSIGNED_INT, 0);		//
		glBindVertexArray(0);
	}
};


class Geometry : public Drawable {
	// Geometry没有生成网格，但已经实现了draw()，直接调用会报错
	// 网格在子类构造函数中生成
private:
protected:
	glm::mat4 modelBuffer{ glm::mat4(1.0f) };		// 用于进行一个预变换
	std::vector<Mesh*> meshes;
	Shader* shader{ nullptr };
public:
	Transform transform;
	uniformTable attribute;

	Geometry() :shader(DefaultShader::getDefaultShader()) {}

	~Geometry() {
		for (auto& mesh : meshes) {
			delete mesh;
		}
		meshes.clear();
	}

	virtual void pose() {
		// 设置一个默认的姿态，可以在子类中是实现不同的姿态
		transform.rotate(glm::radians(-90.0f), _right);
		applyTransform();
	}

	// 预变换相关
	glm::mat4 getModelBufferMatrix() {
		return modelBuffer;
	}
	void applyTransform() {
		modelBuffer = (transform.getMatrix()) * modelBuffer;
		transform.reset();
	}

	// transform 适配
	void translate(glm::vec3 dxyz) {
		transform.translate(dxyz);
	}
	void rotate(float angle, glm::vec3 axis) {
		transform.rotate(angle, axis);
	}
	void scale(glm::vec3 xyz) {
		transform.scale(xyz);
	}
	void scaleTo(glm::vec3 xyz) {
		transform.scaleTo(xyz);
	}
	void translateTo(glm::vec3 dxyz) {
		transform.translateTo(dxyz);
	}
	void resetTransform() {
		transform.reset();
	}

	virtual void draw() {
		// 使用着色器，设置uniform变量
		shader->use();
		(*shader)["modelBuffer"] = modelBuffer;
		(*shader)["model"] = transform.getMatrix();
		shader->loadAttribute(attribute);
		for (auto& mesh : meshes) {
			glBindVertexArray(mesh->getVAO());
			glDrawElements(GL_TRIANGLES, mesh->getIndexSize(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
		}
	}
};

class Cube : public Geometry {
private:
	int xSliceNum, ySliceNum, zSliceNum;
	float xLength, yLength, zLength;
public:
	Cube(float xLength, float yLength, float zLength, unsigned  int xSliceNum = 10, unsigned int ySliceNum = 10, unsigned int zSliceNum = 10) :
		Geometry(), xSliceNum(xSliceNum), ySliceNum(ySliceNum), zSliceNum(zSliceNum),
		xLength(xLength), yLength(yLength), zLength(zLength) {
		if (!shader)
			shader = DefaultShader::getDefaultShader();

		float dx, dy, dz;
		dx = xLength / xSliceNum;
		dy = yLength / ySliceNum;
		dz = zLength / zSliceNum;

		// 立方体存在6个面，因此有6个Mesh实例
		meshes.push_back(new Mesh(xSliceNum + 1, ySliceNum + 1));
		meshes.push_back(new Mesh(xSliceNum + 1, ySliceNum + 1));
		meshes.push_back(new Mesh(xSliceNum + 1, zSliceNum + 1));
		meshes.push_back(new Mesh(xSliceNum + 1, zSliceNum + 1));
		meshes.push_back(new Mesh(ySliceNum + 1, zSliceNum + 1));
		meshes.push_back(new Mesh(ySliceNum + 1, zSliceNum + 1));

		// 顶点索引
		for (unsigned int i = 0; i < 6; i++)
			meshes[i]->connect();

		// 顶点位置
		meshes[0]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {
			return vec3{ -xLength / 2 + i * dx, -yLength / 2 + j * dy, -zLength / 2 };
			});
		meshes[1]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {
			return vec3{ -xLength / 2 + i * dx, -yLength / 2 + j * dy, zLength / 2 };
			});
		meshes[2]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {
			return vec3{ -xLength / 2 + i * dx, -yLength / 2 , -zLength / 2 + j * dz };
			});
		meshes[3]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {
			return vec3{ -xLength / 2 + i * dx, yLength / 2 , -zLength / 2 + j * dz };
			});
		meshes[4]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {
			return vec3{ -xLength / 2 , -yLength / 2 + i * dy , -zLength / 2 + j * dz };
			});
		meshes[5]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {
			return vec3{ xLength / 2 , -yLength / 2 + i * dy , -zLength / 2 + j * dz };
			});

		// 顶点法线
		meshes[0]->updateVertexNormalByFunc([=](float x, float y, float z) {
			return vec3{ 0.0f, 0.0f, -1.0f };
			});
		meshes[1]->updateVertexNormalByFunc([=](float x, float y, float z) {
			return vec3{ 0.0f, 0.0f, 1.0f };
			});
		meshes[2]->updateVertexNormalByFunc([=](float x, float y, float z) {
			return vec3{ 0.0f, -1.0f, 0.0f };
			});
		meshes[3]->updateVertexNormalByFunc([=](float x, float y, float z) {
			return vec3{ 0.0f, 1.0f, 0.0f };
			});
		meshes[4]->updateVertexNormalByFunc([=](float x, float y, float z) {
			return vec3{ -1.0f, 0.0f, 0.0f };
			});
		meshes[5]->updateVertexNormalByFunc([=](float x, float y, float z) {
			return vec3{ 1.0f, 0.0f, 0.0f };
			});
	}
};

class Sphere : public Geometry {
private:
	float radius;
	int lonSliceNum;
	int latSliceNum;

public:
	Sphere(float radius, unsigned int lonSliceNum = 36, unsigned int latSliceNum = 20) :Geometry(), radius(radius), lonSliceNum(lonSliceNum), latSliceNum(latSliceNum) {

		float latStep = PI / latSliceNum;
		float lonStep = 2 * PI / lonSliceNum;

		meshes.push_back(new Mesh(latSliceNum + 1, lonSliceNum + 1));
		meshes[0]->connect();
		meshes[0]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {
			float lat = -PI / 2 + i * latStep;
			float lon = -PI + j * lonStep;
			return vec3{ radius * cos(lat) * cos(lon), radius * cos(lat) * sin(lon), radius * sin(lat) };
			});
		meshes[0]->updateVertexNormalByFunc([=](float x, float y, float z) {
			float norm = sqrtf(x * x + y * y + z * z);
			return vec3{ x / norm,y / norm,z / norm };
			});
	}
};

class Cylinder : public Geometry {
private:
	float radius;
	float height;
	int rSliceNum;
	int hSliceNum;
	int lonSliceNum;
public:
	Cylinder(float radius, float height, unsigned int rSliceNum = 10, unsigned int hSliceNum = 20, unsigned int lonSliceNum = 36) :Geometry(), radius(radius), height(height), rSliceNum(rSliceNum), hSliceNum(hSliceNum), lonSliceNum(lonSliceNum) {

		float rStep = radius / rSliceNum;
		float hStep = height / hSliceNum;
		float lonStep = 2 * PI / lonSliceNum;

		meshes.push_back(new Mesh(rSliceNum + 1, lonSliceNum + 1));
		meshes.push_back(new Mesh(rSliceNum + 1, lonSliceNum + 1));
		meshes.push_back(new Mesh(hSliceNum + 1, lonSliceNum + 1));

		for (unsigned int i = 0; i < 3; i++)
			meshes[i]->connect();

		// 顶点位置
		meshes[0]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {		// 下圆面
			float r_tmp = i * rStep;
			float lon_tmp = -PI + j * lonStep;
			return vec3{ r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), -height / 2 };
			});
		meshes[1]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {		// 上圆面
			float r_tmp = i * rStep;
			float lon_tmp = -PI + j * lonStep;
			return vec3{ r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), height / 2 };
			});
		meshes[2]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {		// 侧面
			float h_tmp = -height / 2 + i * hStep;
			float lon_tmp = -PI + j * lonStep;
			return vec3{ radius * cos(lon_tmp),radius * sin(lon_tmp) , h_tmp };
			});

		// 顶点法线
		meshes[0]->updateVertexNormalByFunc([=](float x, float y, float z) {
			return vec3{ 0.0f,-1.0f,0.0f };
			});
		meshes[1]->updateVertexNormalByFunc([=](float x, float y, float z) {
			return vec3{ 0.0f,1.0f,0.0f };
			});
		meshes[2]->updateVertexNormalByFunc([=](float x, float y, float z) {
			float norm = sqrtf(x * x + y * y);
			return vec3{ x / norm,y / norm,0.0f };
			});
	}
};

class Cone : public Geometry { // 圆锥
private:
	float radius;
	float height;

	int rSliceNum;
	int hSliceNum;
	int lonSliceNum;
public:
	Cone(float radius, float height, unsigned int rSliceNum = 10, unsigned int hSliceNum = 20, unsigned int lonSliceNum = 36) :Geometry(), radius(radius), height(height), rSliceNum(rSliceNum), hSliceNum(hSliceNum), lonSliceNum(lonSliceNum) {
		float rStep = radius / rSliceNum;
		float hStep = height / hSliceNum;
		float lonStep = 2 * PI / lonSliceNum;

		meshes.push_back(new Mesh(rSliceNum + 1, lonSliceNum + 1));
		meshes.push_back(new Mesh(hSliceNum + 1, lonSliceNum + 1));

		for (unsigned int i = 0; i < 2; i++)
			meshes[i]->connect();

		// 顶点位置
		meshes[0]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {	// 下圆面
			float r_tmp = i * rStep;
			float lon_tmp = -PI + j * lonStep;
			return vec3{ r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), -height / 2 };
			});
		meshes[1]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {	// 侧斜面
			float h_tmp = -height / 2 + i * hStep;
			float lon_tmp = -PI + j * lonStep;
			float r_tmp = radius * (1 - i * hStep / height);
			return vec3{ r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp) , h_tmp };
			});

		// 顶点法线
		meshes[0]->updateVertexNormalByFunc([=](float x, float y, float z) {
			return vec3{ 0.0f,-1.0f,0.0f };
			});
		meshes[1]->updateVertexNormalByFunc([=](unsigned int i, unsigned int j) {
			float lon_tmp = -PI + j * lonStep;
			float tmp = sqrt(radius * radius + height * height);
			return vec3{ height * cos(lon_tmp) / tmp,height * sin(lon_tmp) / tmp, radius / tmp };
			});

	}
};



//class Combination : public Geometry {
//	// Combination类用于组合多个Geometry对象，其继承自Geometry类，重载translate、rotate、scale、draw等方法
//private:
//	std::vector<Geometry*> children;
//	std::vector<glm::mat4> childModel;
//public:
//	// Combination应当禁用自己的shader，而是使用objs中各个obj自己的shader
//	// VAO和VAO_length也应当使用obj自身的
//	Combination() :Geometry() {}
//	void add(Geometry* obj) {
//		// Combination默认的中心在原点处
//		// 传入的obj会将当前自身的模型变换model作为Combination的objModel，并重置自身model为单位阵
//		children.push_back(obj);
//		childModel.push_back(obj->transform.getMatrix());
//		obj->resetTransform();
//	}
//	std::vector<Geometry*>& getChildren() {
//		return children;
//	}
//	glm::mat4& getChildModel(int i) {
//		return childModel[i];
//	}

	//virtual void draw() {
	//	// 需要应用所有变换，这里考虑几类情况
	//	// 1. Combination自身的变换，即Combination的model
	//	// 2. Combination中的obj的变换，即objModel
	//	// 3. Combination中的obj自身的变换，即obj->getModelMatrix(),这包括了obj的model和modelBuffer


	//	// 注意：目前考虑的obj只可能是Geometry类，后续考虑实现obj为Combination的情况
	//	// 此时最终model矩阵的计算可能会涉及到递归

	//	for (int i = 0; i < objs.size(); i++) {
	//		// 考虑使用Combination的model、modelBuffer和objModel[i]的连乘变换，附加(左乘)在objs[i]的getModelMatrix()上，其他属性设置和原来的draw保持一致
	//		Shader* shader = objs[i]->getShader();
	//		shader->setModel(transform->getMatrix());
	//		shader->setModelBuffer(getModelBufferMatrix() * objModel[i] * (objs[i]->transform->getMatrix()) * (objs[i]->getModelBufferMatrix()));
	//		shader->loadUniform(objs[i]->getUniform());
	//		shader->use();
	//		glBindVertexArray(objs[i]->getVAO());
	//		glDrawElements(GL_TRIANGLES, objs[i]->getVAOLength(), GL_UNSIGNED_INT, 0);
	//		glBindVertexArray(0);
	//	}
	//}
	//};

class Bone {
private:
	//Bone* child{ 0 };			// 暂时只考虑一个子骨骼
	std::vector<Bone*> children;
	Bone* parent{ 0 };

	//glm::vec3 position{ 0.0f,0.0f,0.0f };			// 起点位置，根骨骼的位置是有效的，其他所有子骨骼的起点位置由父骨骼确定  // 后来发现这个是多余的，坐标变换矩阵已经包含了这个信息
	glm::vec3 vec{ 0.0f,0.0f,0.0f };			// 骨骼方向向量，其大小表示骨骼的长度，方向表示骨骼的方向

	// 用来调试绘制的圆柱体（可去除）
	Geometry* obj = nullptr;

	// 每个Bone需要有其起止位置，或者一个起点和一个带大小的方向向量
	// 父骨骼通过子骨骼的起始位置和方向向量来确定自己的位置，通过计算出一个位移矩阵
public:
	Transform transform; // 表示当前骨骼的子骨骼相对于当前骨骼的变换
	Bone(float length = 1.0f) : vec(length* _up) {
		transform.translateTo(vec);	// 变换矩阵为骨骼末端相对于骨骼起始点的平移矩阵

		// 用来调试绘制的圆柱体（可去除）
		obj = new Cylinder(0.04f, length, 4, 20, 36);
		obj->rotate(glm::radians(-90.0f), _right);
		obj->translateTo(glm::vec3(0.0f, length / 2, 0.0f));
		obj->applyTransform();	// 将中心点移动到圆柱的下端点
	}
	~Bone() {
		for (auto& child : children) {
			delete child;
		}
		children.clear();
	}
	glm::mat4 getTransMatrix() {		// 遍历所有父骨骼，计算当前骨骼坐标系变换矩阵
		glm::mat4 transMatrix(1.0f);
		Bone* cur = this;
		while (cur->parent) {
			cur = cur->parent;
			transMatrix = cur->transform.getMatrix() * transMatrix;
		}
		return transMatrix;
	}
	Bone* getParent() {
		return parent;
	}
	Bone* getChild(unsigned int index) {
		return children[index];
	}
	std::vector<Bone*>& getChildren() {
		return children;
	}
	void addChild(Bone* b) {
		if (b != nullptr) {
			b->parent = this;
			children.push_back(b);
		}
	}
	void rotate(float angle, glm::vec3 axis) {
		vec = glm::rotate(glm::mat4(1.0f), angle, axis) * glm::vec4(vec, 1.0f);
		transform.rotate(angle, axis);	// 对其子骨骼的变换矩阵而言既要考虑旋转又要进行位移
		transform.translateTo(vec);

		// 用来调试绘制的圆柱体（可去除）
		obj->rotate(angle, axis);	// 对当前绘制的骨骼而言只有旋转没有位移
	}
	// 用来调试绘制的圆柱体（可去除）
	Geometry* getObj() {
		return obj;
	}
};

class Skeleton {
private:
	Bone* root{ 0 };
public:
	Skeleton() {}
	Skeleton(Bone* root) :root(root) {}
	Bone* getRoot() {
		return root;
	}
};

class Arrow {		// 比较粗暴的组合体实现
private:
	glm::vec3 _begin{ 0.0f,0.0f,0.0f };
	glm::vec3 _end{ 0.0f,0.0f,0.0f };
	float width{ 1.0f };

	const float arrowLengthRatio = 0.2f;
	const float arrowRadiusRatio = 2.5f;

	Geometry* arrow, * body;
public:
	Arrow(glm::vec3 _begin, glm::vec3 _end, float width, glm::vec4 arrowColor, glm::vec4 bodyColor) :
		_begin(_begin), _end(_end), width(width) {
		glm::vec3 dir = _end - _begin;
		float length = glm::length(dir);
		arrow = new Cone(arrowRadiusRatio * width / 2.0f, arrowLengthRatio, 3, 4, 18);
		body = new Cylinder(width / 2.0f, (1 - arrowLengthRatio), 2, (int)(length * 10), 18);

		arrow->attribute.autoColor = false;
		arrow->attribute.color = arrowColor;
		body->attribute.autoColor = false;
		body->attribute.color = bodyColor;

		// 进行组合
		arrow->scale(glm::vec3(1.0f, length, 1.0f));	// 由于局部坐标系中物体按照y轴方向绘制，所以只需要在这个方向上进行缩放，就能改变长度
		body->scale(glm::vec3(1.0f, length, 1.0f));
		arrow->rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		arrow->translate(glm::vec3(0, (1 - arrowLengthRatio) / 2.0f * length, 0));
		body->rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		body->translate(glm::vec3(0, -arrowLengthRatio / 2.0f * length, 0));
		// 应用变换
		arrow->applyTransform();
		body->applyTransform();
		// 此时两者组合在一起，构成以原点为中点的箭头

		update();
	}
	~Arrow() {
		delete arrow;
		delete body;
	}
	Geometry* getArrow() {
		return arrow;
	}
	Geometry* getBody() {
		return body;
	}
	void update() {
		arrow->resetTransform();
		body->resetTransform();

		glm::vec3 dir = glm::normalize(_end - _begin);
		glm::vec3 rotate_axis = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), dir);
		float rotate_angle = glm::acos(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), dir));
		if (abs(rotate_angle) < EPS) {		// 完全平行时rotate_axis接近全零会导致rotate方法调用出错
			rotate_axis = glm::vec3(0.0f, 0.0f, 1.0f);
			rotate_angle = 0.0f;
		}
		else if (abs(rotate_angle - PI) < EPS) {
			rotate_axis = glm::vec3(0.0f, 0.0f, 1.0f);
			rotate_angle = PI;
		}
		else {
			rotate_axis = glm::normalize(rotate_axis);
		}

		float length = glm::length(_end - _begin);
		arrow->scale(glm::vec3(length, length, length));
		body->scale(glm::vec3(length, length, length));

		arrow->rotate(rotate_angle, rotate_axis);
		arrow->translateTo((_end + _begin) / 2.0f);
		body->rotate(rotate_angle, rotate_axis);
		body->translateTo((_end + _begin) / 2.0f);
	}
	void setBegin(glm::vec3 _begin) {
		this->_begin = _begin;
		this->update();
	}
	void setEnd(glm::vec3 _end) {
		this->_end = _end;
		this->update();
	}
};

class Axis {
private:
	Arrow* axis_x, * axis_y, * axis_z;
public:
	Axis(float length = 1.0f) {
		axis_x = new Arrow(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(length, 0.0f, 0.0f), 0.06f * length, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		axis_y = new Arrow(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, length, 0.0f), 0.06f * length, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		axis_z = new Arrow(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, length), 0.06f * length, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	}
	~Axis() {
		delete axis_x;
		delete axis_y;
		delete axis_z;
	}
	Arrow* getAxis_x() {
		return axis_x;
	}
	Arrow* getAxis_y() {
		return axis_y;
	}
	Arrow* getAxis_z() {
		return axis_z;
	}
};


class Leaf :public Geometry {
private:
	float width, height;
	unsigned int wSliceNum, hSliceNum;

	float k = 4.4f, SLAngle = 30.0f, MVAngle = 0.0f, theta = 0.2f;				// k为叶片弯曲程度，SLAngle为叶片弯曲角度，theta为叶宽因子
	bool isChanged = false;												// 标记叶片网格是否被改变

	const unsigned int u_degree = 3, v_degree = 2;		// u为长度分割，v为宽度分割

	float wFunc(float h) {
		// 生成叶片宽度关于长度的函数
		return (-1.0f / powf(height, 2) * powf(h, 2) + 2 * theta / height * h + (1 - 2 * theta)) * width / powf(1 - theta, 2);
	}

public:

	void updateMesh() {		// 在scene的render(Leaf)中判断isChanged为true后调用此函数更新网格，后将isChanged置为false
		int hwNum = hSliceNum * wSliceNum, hwNum1 = (hSliceNum + 1) * (wSliceNum + 1);
		std::vector<vec3> vertex(hwNum1, { 0 });
		float x, z, x_rt;

		float x_accum = 0.0f, y_accum = 0.0f;
		float a = -k * width / height, b = tanf(PI / 2.0f - SLAngle * PI / 180.0f);
		float ds = height / hSliceNum;
		float tmp, frac, costheta, sintheta;
		vec4 tp;
		glm::vec3 offset, axis;
		glm::mat4 trans(1.0f);

		for (unsigned int i = 0; i <= hSliceNum; i++) {
			x_rt = (float)i / hSliceNum;
			x = x_rt * height;
			for (unsigned int j = 0; j <= wSliceNum; j++) {
				z = (j - wSliceNum / 2.0f) / (wSliceNum / 2.0f) * wFunc(x);
				// 进行主脉旋转
				tp = toVec(trans * glm::vec4(x_accum, y_accum, z, 1.0f));
				vertex[i * (wSliceNum + 1) + j] = { tp.x, tp.y, tp.z };
			}
			// 计算x和y的新坐标
			tmp = 2.0f * a * x + b;
			frac = sqrtf(1.0f + tmp * tmp);
			costheta = 1.0f / frac;
			sintheta = tmp / frac;
			x_accum += ds * costheta;
			y_accum += ds * sintheta;
			// 相同长度分位点、不同宽度分位点的节点，使用相同的旋转矩阵进行变换，不知道这里是否应该用compute shader实现更好
			offset = glm::vec3(x_accum, y_accum, 0.0f);
			axis = glm::vec3(costheta, sintheta, 0.0f);
			trans = glm::translate(glm::mat4(1.0f), offset);
			trans = glm::rotate(trans, glm::radians(MVAngle * x_rt), axis);
			trans = glm::translate(trans, -offset);
		}

		//updateVertexPosition(VAO, VBO[0], vertex);
		isChanged = false;
	}

	Leaf(float width, float height, unsigned int wSliceNum, unsigned int hSliceNum) :
		Geometry(), width(width), height(height), hSliceNum(hSliceNum), wSliceNum(wSliceNum), theta(0.35f) {

		// 不使用NURBS曲面，直接连线
		int hwNum = hSliceNum * wSliceNum, hwNum1 = (hSliceNum + 1) * (wSliceNum + 1);

		std::vector<vec3> vertex(hwNum1, { 0 });
		std::vector<vec3> normal(hwNum1, { 0 });	// 没有生成法向量
		std::vector<GLuint> index(hwNum1 * 6, 0);

		float x, z;
		float x_accum = 0.0f, y_accum = 0.0f;
		float a = -k * width / height, b = tanf(PI / 2.0f - SLAngle * PI / 180.0f);
		float ds = height / hSliceNum;
		float tmp, frac, costheta, sintheta;

		for (unsigned int i = 0; i <= hSliceNum; i++) {
			x = (float)i / hSliceNum * height;
			for (unsigned int j = 0; j <= wSliceNum; j++) {
				z = (j - wSliceNum / 2.0f) / (wSliceNum / 2.0f) * wFunc(x);
				vertex[i * (wSliceNum + 1) + j] = { x_accum, y_accum, z };
				if (i < hSliceNum && j < wSliceNum) {
					unsigned int* ptr = &index[(i * wSliceNum + j) * 6];
					*ptr++ = i * (wSliceNum + 1) + j;
					*ptr++ = i * (wSliceNum + 1) + j + 1;
					*ptr++ = (i + 1) * (wSliceNum + 1) + j + 1;
					*ptr++ = i * (wSliceNum + 1) + j;
					*ptr++ = (i + 1) * (wSliceNum + 1) + j + 1;
					*ptr++ = (i + 1) * (wSliceNum + 1) + j;
				}
			}
			// 计算x和y的新坐标  figure out new x and y 
			tmp = 2.0f * a * x + b;
			frac = sqrtf(1.0f + tmp * tmp);
			costheta = 1.0f / frac;
			sintheta = tmp / frac;
			x_accum += ds * costheta;
			y_accum += ds * sintheta;
		}
		//prepareVAO(vertex, normal, index, &VAO, VBO, &index_size);
	}

	void setTheta(float theta) {
		this->theta = theta;
		isChanged = true;
	}
	void addTheta(float delta) {
		theta += delta;
		if (theta > 0.5f - 0.01f) {
			theta = 0.5f - 0.01f;
		}
		else if (theta < 0.0f) {
			theta = 0.0f;
		}
		isChanged = true;
	}
	void setK(float k) {
		this->k = k;
		isChanged = true;
	}
	void addK(float delta) {
		k += delta;
		if (k > 20.0f) {
			k = 20.0f;
		}
		else if (k < 0.0f) {
			k = 0.0f;
		}
		isChanged = true;
	}
	void setSLAngle(float angle) {
		this->SLAngle = angle;
		isChanged = true;
	}
	void addSLAngle(float delta) {
		SLAngle += delta;
		if (SLAngle > 90.0f) {
			SLAngle = 90.0f;
		}
		else if (SLAngle < 0.1f) {
			SLAngle = 0.1f;
		}
		isChanged = true;
	}
	void setMVAngle(float angle) {
		this->MVAngle = angle;
		isChanged = true;
	}
	bool isChangedMesh() {
		return isChanged;
	}
};

#endif
