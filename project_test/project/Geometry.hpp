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
	// ͨ���ڴ�ӳ��ֱ�Ӳ���VBO��EBO
private:
	unsigned int uSize{ 0 }, vSize{ 0 };		// uΪ�ڼ��У�vΪ�ڼ���
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
	// Geometryû���������񣬵��Ѿ�ʵ����draw()��ֱ�ӵ��ûᱨ��
	// ���������๹�캯��������
private:
protected:
	glm::mat4 modelBuffer{ glm::mat4(1.0f) };		// ���ڽ���һ��Ԥ�任
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
		// ����һ��Ĭ�ϵ���̬����������������ʵ�ֲ�ͬ����̬
		transform.rotate(glm::radians(-90.0f), _right);
		applyTransform();
	}

	// Ԥ�任���
	glm::mat4 getModelBufferMatrix() {
		return modelBuffer;
	}
	void applyTransform() {
		modelBuffer = (transform.getMatrix()) * modelBuffer;
		transform.reset();
	}

	// transform ����
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
		// ʹ����ɫ��������uniform����
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

		// ���������6���棬�����6��Meshʵ��
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

	}
};



//class Combination : public Geometry {
//	// Combination��������϶��Geometry������̳���Geometry�࣬����translate��rotate��scale��draw�ȷ���
//private:
//	std::vector<Geometry*> children;
//	std::vector<glm::mat4> childModel;
//public:
//	// CombinationӦ�������Լ���shader������ʹ��objs�и���obj�Լ���shader
//	// VAO��VAO_lengthҲӦ��ʹ��obj�����
//	Combination() :Geometry() {}
//	void add(Geometry* obj) {
//		// CombinationĬ�ϵ�������ԭ�㴦
//		// �����obj�Ὣ��ǰ�����ģ�ͱ任model��ΪCombination��objModel������������modelΪ��λ��
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
	//	// ��ҪӦ�����б任�����￼�Ǽ������
	//	// 1. Combination����ı任����Combination��model
	//	// 2. Combination�е�obj�ı任����objModel
	//	// 3. Combination�е�obj����ı任����obj->getModelMatrix(),�������obj��model��modelBuffer


	//	// ע�⣺Ŀǰ���ǵ�objֻ������Geometry�࣬��������ʵ��objΪCombination�����
	//	// ��ʱ����model����ļ�����ܻ��漰���ݹ�

	//	for (int i = 0; i < objs.size(); i++) {
	//		// ����ʹ��Combination��model��modelBuffer��objModel[i]�����˱任������(���)��objs[i]��getModelMatrix()�ϣ������������ú�ԭ����draw����һ��
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
	//Bone* child{ 0 };			// ��ʱֻ����һ���ӹ���
	std::vector<Bone*> children;
	Bone* parent{ 0 };

	//glm::vec3 position{ 0.0f,0.0f,0.0f };			// ���λ�ã���������λ������Ч�ģ����������ӹ��������λ���ɸ�����ȷ��  // ������������Ƕ���ģ�����任�����Ѿ������������Ϣ
	glm::vec3 vec{ 0.0f,0.0f,0.0f };			// �����������������С��ʾ�����ĳ��ȣ������ʾ�����ķ���

	// �������Ի��Ƶ�Բ���壨��ȥ����
	Geometry* obj = nullptr;

	// ÿ��Bone��Ҫ������ֹλ�ã�����һ������һ������С�ķ�������
	// ������ͨ���ӹ�������ʼλ�úͷ���������ȷ���Լ���λ�ã�ͨ�������һ��λ�ƾ���
public:
	Transform transform; // ��ʾ��ǰ�������ӹ�������ڵ�ǰ�����ı任
	Bone(float length = 1.0f) : vec(length* _up) {
		transform.translateTo(vec);	// �任����Ϊ����ĩ������ڹ�����ʼ���ƽ�ƾ���

		// �������Ի��Ƶ�Բ���壨��ȥ����
		obj = new Cylinder(0.04f, length, 4, 20, 36);
		obj->rotate(glm::radians(-90.0f), _right);
		obj->translateTo(glm::vec3(0.0f, length / 2, 0.0f));
		obj->applyTransform();	// �����ĵ��ƶ���Բ�����¶˵�
	}
	~Bone() {
		for (auto& child : children) {
			delete child;
		}
		children.clear();
	}
	glm::mat4 getTransMatrix() {		// �������и����������㵱ǰ��������ϵ�任����
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
		transform.rotate(angle, axis);	// �����ӹ����ı任������Լ�Ҫ������ת��Ҫ����λ��
		transform.translateTo(vec);

		// �������Ի��Ƶ�Բ���壨��ȥ����
		obj->rotate(angle, axis);	// �Ե�ǰ���ƵĹ�������ֻ����תû��λ��
	}
	// �������Ի��Ƶ�Բ���壨��ȥ����
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

class Arrow {		// �Ƚϴֱ��������ʵ��
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

		// �������
		arrow->scale(glm::vec3(1.0f, length, 1.0f));	// ���ھֲ�����ϵ�����尴��y�᷽����ƣ�����ֻ��Ҫ����������Ͻ������ţ����ܸı䳤��
		body->scale(glm::vec3(1.0f, length, 1.0f));
		arrow->rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		arrow->translate(glm::vec3(0, (1 - arrowLengthRatio) / 2.0f * length, 0));
		body->rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		body->translate(glm::vec3(0, -arrowLengthRatio / 2.0f * length, 0));
		// Ӧ�ñ任
		arrow->applyTransform();
		body->applyTransform();
		// ��ʱ���������һ�𣬹�����ԭ��Ϊ�е�ļ�ͷ

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
		if (abs(rotate_angle) < EPS) {		// ��ȫƽ��ʱrotate_axis�ӽ�ȫ��ᵼ��rotate�������ó���
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

	float k = 4.4f, SLAngle = 30.0f, MVAngle = 0.0f, theta = 0.2f;				// kΪҶƬ�����̶ȣ�SLAngleΪҶƬ�����Ƕȣ�thetaΪҶ������
	bool isChanged = false;												// ���ҶƬ�����Ƿ񱻸ı�

	const unsigned int u_degree = 3, v_degree = 2;		// uΪ���ȷָvΪ��ȷָ�

	float wFunc(float h) {
		// ����ҶƬ��ȹ��ڳ��ȵĺ���
		return (-1.0f / powf(height, 2) * powf(h, 2) + 2 * theta / height * h + (1 - 2 * theta)) * width / powf(1 - theta, 2);
	}

public:

	void updateMesh() {		// ��scene��render(Leaf)���ж�isChangedΪtrue����ô˺����������񣬺�isChanged��Ϊfalse
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
				// ����������ת
				tp = toVec(trans * glm::vec4(x_accum, y_accum, z, 1.0f));
				vertex[i * (wSliceNum + 1) + j] = { tp.x, tp.y, tp.z };
			}
			// ����x��y��������
			tmp = 2.0f * a * x + b;
			frac = sqrtf(1.0f + tmp * tmp);
			costheta = 1.0f / frac;
			sintheta = tmp / frac;
			x_accum += ds * costheta;
			y_accum += ds * sintheta;
			// ��ͬ���ȷ�λ�㡢��ͬ��ȷ�λ��Ľڵ㣬ʹ����ͬ����ת������б任����֪�������Ƿ�Ӧ����compute shaderʵ�ָ���
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

		// ��ʹ��NURBS���棬ֱ������
		int hwNum = hSliceNum * wSliceNum, hwNum1 = (hSliceNum + 1) * (wSliceNum + 1);

		std::vector<vec3> vertex(hwNum1, { 0 });
		std::vector<vec3> normal(hwNum1, { 0 });	// û�����ɷ�����
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
			// ����x��y��������  figure out new x and y 
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
