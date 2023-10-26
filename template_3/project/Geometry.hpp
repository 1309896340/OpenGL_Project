#ifndef __WIND_GEOMETRY
#define __WIND_GEOMETRY

#include "utils.h"
#include "proj.h"
#include "Shader.hpp"


#include "stb_image.h"

extern Shader* defaultShader;			// Ĭ����ɫ����������demo.cpp��

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
	Transform model;		// ģ�;���
public:
	virtual void draw(Shader* shader) = 0;
};
class Mesh : public Drawable {
	// ͨ���ڴ�ӳ��ֱ�Ӳ���VBO��EBO
	// ��������Ϣ����Mesh�У�ͬ�������������һͬ����VBO
private:
	unsigned int uSize{ 0 }, vSize{ 0 };		// uΪ�ڼ��У�vΪ�ڼ���
	GLuint VAO{ 0 }, VBO[4]{ 0,0,0,0 }, texture{ 0 };			// VBO�ֱ�Ϊposition��normal��index��texture
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
		// Ĭ�������������ӣ�ָ��1�������ʼ��2�У�
		GLuint* ptr;
		GLuint* index_ptr = (GLuint*)glMapNamedBuffer(VBO[2], GL_WRITE_ONLY);
		assert(index_ptr);
		for (unsigned int i = 0; i < uSize - 1; i++) {
			for (unsigned int j = 0; j < vSize - 1; j++) {
				ptr = index_ptr + (i * (vSize - 1) + j) * 6;	// ������ʱ������
				*ptr++ = i * vSize + j;						// ����
				*ptr++ = (i + 1) * vSize + j;				// ����
				*ptr++ = (i + 1) * vSize + j + 1;		// ����
				*ptr++ = i * vSize + j;						// ����
				*ptr++ = (i + 1) * vSize + j + 1;		// ����
				*ptr++ = i * vSize + j + 1;				// ����
			}
		}
		glUnmapNamedBuffer(VBO[2]);
	}
	bool isTexture() {
		return glIsTexture(texture) == GL_TRUE;
	}
	void setTexture(std::string path) {		// ֻ���ڵ��õ��������ʱ��VBO[3]�Żᱻ����
		int img_width, img_height, img_channels;
		unsigned char* img_ptr = stbi_load(path.c_str(), &img_width, &img_height, &img_channels, 0);

		if (!img_ptr)
			throw std::runtime_error("��������ͼƬʧ�ܣ�" + path);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);		// �����ֽڶ����ʽ��Ĭ��Ϊ4�ֽڶ���
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_ptr);	// �������������
		glGenerateMipmap(GL_TEXTURE_2D);		// Ϊ��ǰ�󶨵������Զ�����������Ҫ�Ķ༶��Զ����

		stbi_image_free(img_ptr);

		// ���°�VAO��Ϊ����������������
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO[3]);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
		glBufferData(GL_ARRAY_BUFFER, getVertexSize() * sizeof(vec2), nullptr, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
		glEnableVertexAttribArray(2);
		glBindVertexArray(0);
		// Ŀǰ�ǽ��������о��Ȳ���ӳ�䵽������
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
			throw "���ܷ��طǷ���texture";
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
	virtual void draw(Shader* sd) {		// Mesh������Ե������ƣ�����û��model��modelBuffer������λ����ص��Զ���ɫ
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
	// Geometry����û����������
private:
protected:
	glm::mat4 modelBuffer{ glm::mat4(1.0f) };
	Geometry* parent{ nullptr };
	std::vector<Geometry*> children;
	std::vector<Mesh*> meshes;
public:
	Transform model;		// ģ�;���
	Transform offset;			// ƫ�ƾ�����addChildʱ��¼�ӽڵ��뵱ǰ�ڵ��ƫ����(����λ�á���ת)�����ڸ��ӽڵ�������ϵ�任���ڴ�֮�ϵ���model�任
	uniformTable attribute;
	Geometry() {}
	~Geometry() {		// ��ɾ����ǰ���󣬻���ɾ�������Ӷ���ѡ�����
		for (auto& child : getChildren())
			delete child;		// �����ݹ�ɾ�������Ӷ���
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
	// Ԥ�任
	glm::mat4 getModelBufferMatrix() {
		return modelBuffer;
	}
	void applyTransform() {
		modelBuffer = (model.getMatrix()) * modelBuffer;
		model.reset();
	}
	// ��̬�任
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
	// ������������
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
	// ���ṹ����
	void addChild(Geometry* obj, const Transform& offset = Transform()) {		// ���ӵ�ʱ����Ҫָ���ӽڵ�(pose���)���λ������ڸ��ڵ�����pose����ϵ��λ��ƫ��
		if (obj != nullptr) {
			obj->setParent(this);
			obj->offset = offset;
			//obj->setShader(shader);		// ��Ӧ����������ӽڵ��shader����Ϊ��ǰ�ڵ��shader����Ӧ�������ӽڵ��shader����
			children.push_back(obj);
		}
	}
	void setParent(Geometry* obj) {
		this->parent = obj;
	}
	glm::mat4 getFinalOffset() {	 // �������и��ڵ���㵱ǰ�ڵ��ƫ�ƾ���
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
	void doDraw(Shader* sd = nullptr) {		// Ϊ�˱���ʹ�ô�Ĭ��ֵ�Ĵ��麯������sdΪnullptrʱʹ��Ĭ��shader������ʹ�ô����shader
		if (sd == nullptr)
			sd = this->shader;
		this->draw(sd);
	}
	virtual void draw(Shader* sd) {			// �������û����дdraw��������ʹ��Ĭ�ϵ�draw����������Ҳ��ζ�ű�����fragment shader�ж���isAuto��ncolor
		sd->use();
		(*sd)["modelBuffer"] = getModelBufferMatrix();
		(*sd)["model"] = getFinalOffset() * model.getMatrix();
		(*sd)["isAuto"] = attribute.autoColor;
		(*sd)["ncolor"] = attribute.color;
		for (auto& mesh : meshes) {
			if (mesh->isTexture())
				glBindTexture(GL_TEXTURE_2D, mesh->getTexture());		// ��������в��ʾͰ󶨣���ʵ����û����ʾ�����ÿ�ʹ�����ĸ�shader
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

		// ������6��������
		meshes.push_back(new Mesh(xSliceNum + 1, ySliceNum + 1));
		meshes.push_back(new Mesh(xSliceNum + 1, ySliceNum + 1));
		meshes.push_back(new Mesh(xSliceNum + 1, zSliceNum + 1));
		meshes.push_back(new Mesh(xSliceNum + 1, zSliceNum + 1));
		meshes.push_back(new Mesh(ySliceNum + 1, zSliceNum + 1));
		meshes.push_back(new Mesh(ySliceNum + 1, zSliceNum + 1));

		// ��������
		for (unsigned int i = 0; i < 6; i++)
			meshes[i]->connect();

		// ����λ��
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

		// ���㷨��
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

		// ����λ��
		meshes[0]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {		// ��Բ��
			float r_tmp = i * rStep;
			float lon_tmp = -PI + j * lonStep;
			return vec3{ r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), -height / 2 };
			});
		meshes[1]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {		// ��Բ��
			float r_tmp = i * rStep;
			float lon_tmp = -PI + j * lonStep;
			return vec3{ r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), height / 2 };
			});
		meshes[2]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {		// ����
			float h_tmp = -height / 2 + i * hStep;
			float lon_tmp = -PI + j * lonStep;
			return vec3{ radius * cos(lon_tmp),radius * sin(lon_tmp) , h_tmp };
			});

		// ���㷨��
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
class Cone : public Geometry { // Բ׶
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

		// ����λ��
		meshes[0]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {	// ��Բ��
			float r_tmp = i * rStep;
			float lon_tmp = -PI + j * lonStep;
			return vec3{ r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), -height / 2 };
			});
		meshes[1]->updateVertexPositionByFunc([=](unsigned int i, unsigned int j) {	// ��б��
			float h_tmp = -height / 2 + i * hStep;
			float lon_tmp = -PI + j * lonStep;
			float r_tmp = radius * (1 - i * hStep / height);
			return vec3{ r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp) , h_tmp };
			});

		// ���㷨��
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
	// Ŀǰû��ʵ�ֶ�̬�ı��ͷ���ȵĹ��ܣ�ֻ���ڹ��캯����ָ��
	// �ı�ָ��λ��ֻ��ͨ���任ʵ�֣��һ��ԭ�ȼ�ͷ�ĳ��������Ӱ��
	Arrow(glm::vec3 _begin, glm::vec3 _end, glm::vec3 bodyColor) :_begin(_begin), _end(_end), bodyColor(bodyColor) {
		float length = glm::length(_end - _begin);
		glm::vec3 direction = glm::normalize(_end - _begin);

		if (length <= arrowLength) {
			std::cout << "���ȹ��̣�������0.2�������Ų��ԣ���Ӱ���ͷ������" << std::endl;
			arrowLength = length * arrowLengthRatio;
		}

		arrow = new Cone(width * arrowRadiusRatio, arrowLength);
		body = new Cylinder(width, length - arrowLength);
		body->addChild(arrow, Transform(glm::vec3(0.0f, length - arrowLength, 0.0f)));
		this->addChild(body, Transform());		// �������������this�У�������this�ı任��ͬʱ��������������

		arrow->attribute = { false, glm::vec4(arrowColor,1.0f) };
		body->attribute = { false, glm::vec4(bodyColor,1.0f) };

		model.reset();
		pose();

		// ��ͷԭʼ��̬Ϊ��(0,0,0)ָ��(0,height,0)��ͨ���任����ӳ��Ϊ��_begin��_end
		glm::vec3 rotate_axis = glm::cross(_up, direction);
		float rotate_radian = glm::acos(glm::dot(glm::vec3(0.0f, 1.0f, 0.0f), direction));
		if (abs(rotate_radian) < EPS) {		// ��ȫƽ��ʱrotate_axis�ӽ�ȫ��ᵼ��rotate�������ó���
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
	// ��������һ���ֲ�����ϵ������������ͷ��ʾ
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
		// ���ڽ�Axis���뵽this�У����������ʱ���Զ�����Geometry�������������ݹ�ɾ�������ӽڵ�
		// �����ظ�ɾ��
	}
	virtual void pose() {}
};

class Plane : public Drawable {
	// ��ƽ�棬���̳���Geometry�������������������
protected:
	GLuint VAO{ 0 }, VBO[2]{ 0,0 };		// ֻ��Ҫ4�����㣬ʹ�ó���Ϊ6���������壬����Ҫ������
	glm::vec4 color{ 0.5f,0.5f,0.5f,0.2f };
public:
	Plane(Shader* shader, vec3 p[4]) {
		this->shader = shader;

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(2, VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(vec3), p, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		glEnableVertexAttribArray(0);

		unsigned int indices[6] = { 0,1,2,0,2,3 };
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW);

		glBindVertexArray(0);
	}
	virtual void draw(Shader* sd) {		// ��ʹ�ò�����shader������ʹ�����ڲ���shader
		shader->use();
		(*shader)["color"] = color;
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
};
class Line :public Drawable {
	// ��������Ҫ����������������û���Ǽ̳���Geometry
protected:
	GLuint VAO, VBO;		// ����ֻ��Ҫ�������㣬����Ҫ����������Ҳ����Ҫ������
	glm::vec4 color{ 1.0f,0.0f,0.0f,1.0f };
public:
	Line(Shader* lineShader, vec3 start = { 0.0f,0.0f,0.0f }, vec3 end = { 0.0f,0.0f,0.0f }) {
		shader = lineShader;	// ��������ר�õ���ɫ��

		// ��ʼ�������˵�VBO
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 2, NULL, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
		// ���������˵�VBO
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

	virtual void draw(Shader* sd) {	// ��ʹ�ò�����shader������ʹ�����ڲ���shader
		shader->use();
		(*shader)["color"] = color;
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, 2);
		glBindVertexArray(0);
	}
};

#endif