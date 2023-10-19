#ifndef __WIND_GEOMETRY
#define __WIND_GEOMETRY

#include "utils.h"
#include "proj.h"
#include "Shader.hpp"


#include "stb_image.h"

extern Shader* defaultShader;			// 默认着色器，定义在demo.cpp中

class Transform {
private:
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 _scale;
public:
	Transform() :position(glm::vec3(0.0f)), _scale(glm::vec3(1.0f)), rotation(glm::identity<glm::quat>()) {}
	Transform(glm::vec3 position) :position(position), _scale(glm::vec3(1.0f)), rotation(glm::identity<glm::quat>()) {}
	Transform(glm::vec3 position, float angle, glm::vec3 axis) :position(position), _scale(glm::vec3(1.0f)), rotation(glm::identity<glm::quat>()) {
		rotate(angle, axis);
	}
	void scale(glm::vec3 xyz) {
		_scale *= xyz;
	}
	void scaleTo(glm::vec3 xyz) {
		_scale = xyz;
	}
	void rotate(float angle, glm::vec3 axis) {
		rotation = glm::angleAxis(glm::radians(angle), glm::normalize(axis)) * rotation;
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
	void setRotation(glm::quat q) {
		rotation = q;
	}
	glm::vec3 getPosition() {
		return position;
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
protected:
	Shader* shader{ defaultShader };
public:
	Transform model;		// 模型矩阵
public:
	virtual void draw(Shader* shader) = 0;
};
class Mesh : public Drawable {
	// 通过内存映射直接操作VBO和EBO
	// 将材质信息放在Mesh中，同顶点的其他属性一同生成VBO
private:
	unsigned int uSize{ 0 }, vSize{ 0 };		// u为第几行，v为第几列
	GLuint VAO{ 0 }, VBO[4]{ 0,0,0,0 }, texture{ 0 };			// VBO分别为position、normal、index、texture
public:
	Mesh(unsigned int uSize = 2, unsigned int vSize = 2) :uSize(uSize), vSize(vSize) {

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
	bool isTexture() {
		return glIsTexture(texture) == GL_TRUE;
	}
	void setTexture(std::string path) {		// 只有在调用到这个函数时，VBO[3]才会被创建
		int img_width, img_height, img_channels;
		unsigned char* img_ptr = stbi_load(path.c_str(), &img_width, &img_height, &img_channels, 0);

		if (!img_ptr)
			throw std::runtime_error("载入纹理图片失败：" + path);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);		// 设置字节对齐格式（默认为4字节对齐
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_ptr);	// 填充纹理缓冲区
		glGenerateMipmap(GL_TEXTURE_2D);		// 为当前绑定的纹理自动生成所有需要的多级渐远纹理

		stbi_image_free(img_ptr);

		// 重新绑定VAO并为顶点添加纹理坐标
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO[3]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
		glBufferData(GL_ARRAY_BUFFER, getVertexSize() * sizeof(vec2), nullptr, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
		glEnableVertexAttribArray(2);
		glBindVertexArray(0);
		// 目前是将纹理进行均匀采样映射到网格上
		vec2* texcoord_ptr = (vec2*)glMapNamedBuffer(VBO[3], GL_WRITE_ONLY);
		for (unsigned int u = 0; u < uSize; u++) {
			for (unsigned int v = 0; v < vSize; v++) {
				texcoord_ptr[u * vSize + v] = { (float)v / (vSize - 1), (float)u / (uSize - 1) };
				//texcoord_ptr[u * vSize + v] = { (float)u / (uSize - 1), (float)v / (vSize - 1) };
			}
		}
		glUnmapNamedBuffer(VBO[3]);
	}
	GLuint getTexture() {
		if (glIsTexture(texture) != GL_TRUE)
			throw "不能返回非法的texture";
		return texture;
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
	vec3* getVertexPositionPtr() {
		return (vec3*)glMapNamedBuffer(VBO[0], GL_WRITE_ONLY);
	}
	void closeVertexPositionPtr() {
		glUnmapNamedBuffer(VBO[0]);
	}
	vec3* getVertexNormalPtr() {
		return (vec3*)glMapNamedBuffer(VBO[1], GL_WRITE_ONLY);
	}
	void closeVertexNormalPtr() {
		glUnmapNamedBuffer(VBO[1]);
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
	virtual void draw(Shader* sd) {		// Mesh对象可以单独绘制，但其没有model和modelBuffer，采用位置相关的自动颜色
		sd->use();
		(*sd)["modelBuffer"] = glm::mat4(1.0f);
		(*sd)["model"] = glm::mat4(1.0f);
		(*sd)["isAuto"] = true;
		(*sd)["ncolor"] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, getIndexSize(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
};
class Geometry : public Drawable {
	// Geometry本身没有生成网格
private:
protected:
	glm::mat4 modelBuffer{ glm::mat4(1.0f) };
	Geometry* parent{ nullptr };
	std::vector<Geometry*> children;
	std::vector<Mesh*> meshes;
public:
	Transform model;		// 模型矩阵
	Transform offset;			// 偏移矩阵，在addChild时记录子节点与当前节点的偏移量(包括位置、旋转)，属于父子节点间的坐标系变换，在此之上叠加model变换
	uniformTable attribute;
	Geometry() {}
	~Geometry() {		// 仅删除当前对象，还是删除所有子对象？选择后者
		for (auto& child : getChildren())
			delete child;		// 这里会递归删除所有子对象
		for (auto& mesh : meshes) {
			delete mesh;
		}
		meshes.clear();
	}
	Shader* getShader() {
		return shader;
	}
	void setShader(Shader* shader) {
		assert(shader);
		this->shader = shader;
	}
	std::vector<Geometry*>& getChildren() {
		return children;
	}
	std::vector<Mesh*>& getMeshes() {
		return meshes;
	}
	virtual void pose() = 0;
	// 预变换
	glm::mat4 getModelBufferMatrix() {
		return modelBuffer;
	}
	void applyTransform() {
		modelBuffer = (model.getMatrix()) * modelBuffer;
		model.reset();
	}
	// 姿态变换
	void translate(glm::vec3 dxyz) {
		model.translate(dxyz);
	}
	void translateTo(glm::vec3 dxyz) {
		model.translateTo(dxyz);
	}
	void rotate(float angle, glm::vec3 axis) {
		model.rotate(angle, axis);
	}
	void scale(glm::vec3 xyz) {
		model.scale(xyz);
	}
	void scaleTo(glm::vec3 xyz) {
		model.scaleTo(xyz);
	}
	// 适配其他参数
	void translate(float dx, float dy, float dz) {
		model.translate(glm::vec3(dx, dy, dz));
	}
	void translateTo(float x, float y, float z) {
		model.translateTo(glm::vec3(x, y, z));
	}
	void rotate(float angle, float axis_x, float axis_y, float axis_z) {
		model.rotate(angle, glm::vec3(axis_x, axis_y, axis_z));
	}
	void scale(float axis_x, float axis_y, float axis_z) {
		model.scale(glm::vec3(axis_x, axis_y, axis_z));
	}
	void scaleTo(float axis_x, float axis_y, float axis_z) {
		model.scaleTo(glm::vec3(axis_x, axis_y, axis_z));
	}
	// 树结构操作
	void addChild(Geometry* obj, const Transform& offset = Transform()) {		// 添加的时候需要指明子节点(pose后的)起点位置相对于父节点在其pose坐标系的位置偏移
		if (obj != nullptr) {
			obj->setParent(this);
			obj->offset = offset;
			//obj->setShader(shader);		// 不应当将加入的子节点的shader设置为当前节点的shader，而应当保持子节点的shader不变
			children.push_back(obj);
		}
	}
	void setParent(Geometry* obj) {
		this->parent = obj;
	}
	glm::mat4 getFinalOffset() {	 // 根据所有父节点计算当前节点的偏移矩阵
		glm::mat4 offsetMatrix(offset.getMatrix());
		Geometry* cur = this;
		while (cur->parent) {
			cur = cur->parent;
			offsetMatrix = (cur->offset.getMatrix()) * (cur->model.getMatrix()) * offsetMatrix;
		}
		return offsetMatrix;
	}
	void drawAll() {
		std::deque<Geometry*> buf{ this };
		while (!buf.empty()) {
			Geometry* cur = buf.front();
			buf.pop_front();
			for (auto& child : cur->getChildren())
				buf.push_back(child);
			cur->doDraw();
		}
	}
	void doDraw(Shader* sd = nullptr) {		// 为了避免使用带默认值的纯虚函数，当sd为nullptr时使用默认shader，否则使用传入的shader
		if (sd == nullptr)
			sd = this->shader;
		this->draw(sd);
	}
	virtual void draw(Shader* sd) {			// 如果子类没有重写draw函数，就使用默认的draw函数。但这也意味着必须在fragment shader中定义isAuto和ncolor
		sd->use();
		(*sd)["modelBuffer"] = getModelBufferMatrix();
		(*sd)["model"] = getFinalOffset() * model.getMatrix();
		(*sd)["isAuto"] = attribute.autoColor;
		(*sd)["ncolor"] = attribute.color;
		for (auto& mesh : meshes) {
			if (mesh->isTexture())
				glBindTexture(GL_TEXTURE_2D, mesh->getTexture());		// 如果网格有材质就绑定，但实际有没有显示，还得看使用了哪个shader
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

		float dx, dy, dz;
		dx = xLength / xSliceNum;
		dy = yLength / ySliceNum;
		dz = zLength / zSliceNum;

		// 立方体6个网格面
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
		model.reset();
		pose();
	}
	virtual void pose() {
		model.rotate(-90.0f, _right);
		applyTransform();
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
		model.reset();
		pose();
	}
	virtual void pose() {
		model.rotate(-90.0f, _right);
		applyTransform();
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
		model.reset();
		pose();
	}
	virtual void pose() {
		model.rotate(-90.0f, _right);
		model.translateTo(glm::vec3(0.0f, height / 2.0f, 0.0f));
		applyTransform();
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
		model.reset();
		pose();
	}
	virtual void pose() {
		model.rotate(-90.0f, _right);
		model.translateTo(glm::vec3(0.0f, height / 2.0f, 0.0f));
		applyTransform();
	}
};
class Arrow : public Geometry {
private:
	glm::vec3 _begin{ glm::vec3(0.0f,0.0f,0.0f) }, _end{ glm::vec3(0.0f,1.0f,0.0f) };
	float width{ 0.03f };
	glm::vec3 arrowColor{ glm::vec3(1.0f,1.0f,0.0f) }, bodyColor{ glm::vec3(1.0f,0.0f,0.0f) };
	Geometry* arrow, * body;

	float arrowLength = 0.2f;
	const float arrowLengthRatio = 0.2f;
	const float arrowRadiusRatio = 2.5f;
public:
	// 目前没有实现动态改变箭头长度的功能，只能在构造函数中指定
	// 改变指向位置只能通过变换实现，且会对原先箭头的长宽比造成影响
	Arrow(glm::vec3 _begin, glm::vec3 _end, glm::vec3 bodyColor) :_begin(_begin), _end(_end), bodyColor(bodyColor) {
		float length = glm::length(_end - _begin);
		glm::vec3 direction = glm::normalize(_end - _begin);

		if (length <= arrowLength) {
			std::cout << "长度过短，已启用0.2比例缩放策略，会影响箭头长宽比" << std::endl;
			arrowLength = length * arrowLengthRatio;
		}

		arrow = new Cone(width * arrowRadiusRatio, arrowLength);
		body = new Cylinder(width, length - arrowLength);
		body->addChild(arrow, Transform(glm::vec3(0.0f, length - arrowLength, 0.0f)));
		this->addChild(body, Transform());		// 将两个对象加入this中，这样对this的变换会同时作用于两个对象

		arrow->attribute = { false, glm::vec4(arrowColor,1.0f) };
		body->attribute = { false, glm::vec4(bodyColor,1.0f) };

		model.reset();
		pose();

		// 箭头原始姿态为从(0,0,0)指向(0,height,0)，通过变换将其映射为从_begin到_end
		glm::vec3 rotate_axis = glm::cross(_up, direction);
		float rotate_radian = glm::acos(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), direction));
		if (abs(rotate_radian) < EPS) {		// 完全平行时rotate_axis接近全零会导致rotate方法调用出错
			rotate_axis = glm::vec3(0.0f, 0.0f, 1.0f);
			rotate_radian = 0.0f;
		}
		else if (abs(rotate_radian - PI) < EPS) {
			rotate_axis = glm::vec3(0.0f, 0.0f, 1.0f);
			rotate_radian = PI;
		}
		body->rotate(rotate_radian * 180.0f / PI, rotate_axis);
	}
	virtual void pose() {}
};
class Axis : public Geometry {
	// 用于描述一个局部坐标系，采用三个箭头表示
private:
	Arrow* axis_x, * axis_y, * axis_z;
public:
	Axis() {
		axis_x = new Arrow(_origin, _right, _right);
		axis_y = new Arrow(_origin, _up, _up);
		axis_z = new Arrow(_origin, -_front, -_front);
		addChild(axis_x);
		addChild(axis_y);
		addChild(axis_z);

		model.reset();
		pose();
	}
	~Axis() {
		// 由于将Axis加入到this中，因此在析构时会自动调用Geometry的析构函数，递归删除所有子节点
		// 无需重复删除
	}
	virtual void pose() {}
};

class Line :public Drawable {
	// 线条不需要用网格描述，所以没考虑继承自Geometry
private:
	GLuint VAO, VBO;		// 线条只需要两个顶点，不需要索引缓冲区也不需要法向量
	glm::vec4 color{ 1.0f,0.0f,0.0f,1.0f };
public:
	Line(Shader* lineShader, vec3 start = { 0.0f,0.0f,0.0f }, vec3 end = { 0.0f,0.0f,0.0f }) {
		shader = lineShader;	// 设置线条专用的着色器

		// 初始化线条端点VBO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 2, NULL, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
		// 载入线条端点VBO
		setStartPoint(start);
		setEndPoint(end);
	}
	void setColor(vec4 color) {
		this->color = toGlm(color);
	}
	void setStartPoint(vec3 start) {
		glNamedBufferSubData(VBO, 0, sizeof(vec3), &start);
	}
	void setEndPoint(vec3 end) {
		glNamedBufferSubData(VBO, sizeof(vec3), sizeof(vec3), &end);
	}

	virtual void draw(Shader* shader) {
		if (shader == nullptr)
			shader = this->shader;
		shader->use();
		(*shader)["color"] = color;
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, 2);
		glBindVertexArray(0);
	}
};

#endif
