#ifndef __WIND_GEOMETRY
#define __WIND_GEOMETRY

#include "proj.h"

#include "stb_image.h"

//extern Shader* defaultShader;			// Ĭ����ɫ����������demo.cpp��
typedef enum {
	DEFAULT,
	LEAF
}GeometryType;


class Transform {
private:
	vec3 position;
	quat rotation;
	vec3 _scale;
public:
	Transform() :position(vec3(0.0f)), _scale(vec3(1.0f)), rotation(identity<quat>()) {}
	Transform(vec3 position) :position(position), _scale(vec3(1.0f)), rotation(identity<quat>()) {}
	Transform(vec3 position, float angle, vec3 axis) :position(position), _scale(vec3(1.0f)), rotation(identity<quat>()) {
		rotate(angle, axis);
	}
	void scale(vec3 xyz) {
		_scale *= xyz;
	}
	void scaleTo(vec3 xyz) {
		_scale = xyz;
	}
	void rotate(float angle, vec3 axis) {
		rotation = angleAxis(radians(angle), normalize(axis)) * rotation;
	}
	void translate(vec3 dxyz) {
		position = position + dxyz;
	}
	void translateTo(vec3 dxyz) {
		position = dxyz;
	}
	void reset() {
		position = vec3(0.0f);
		_scale = vec3(1.0f);
		rotation = identity<quat>();
	}
	quat getRotation() {
		return rotation;
	}
	void setRotation(quat q) {
		rotation = q;
	}
	vec3 getPosition() {
		return position;
	}
	mat4 getMatrix() {
		mat4 Tmat(1.0f);
		Tmat = glm::translate(Tmat, position);
		Tmat = Tmat * mat4_cast(rotation);
		Tmat = glm::scale(Tmat, _scale);
		return Tmat;
	}
};
//class Drawable {
//protected:
//	Shader* shader{ nullptr };
//public:
//	Transform model;		// ģ�;���
//public:
//	virtual void draw(Shader* shader) = 0;
//};

typedef struct _Triangle {
	Vertex vertex[3];
}Triangle;

class Mesh {
private:
	Vertex* vertexPtr{ nullptr };		// ������ʱ�������ݵ�ָ��
	unsigned int* indexPtr{ nullptr };
	Triangle* trianglePtr{ nullptr };		// ���ڱ���ͼԪ
protected:
	// ����������ص�����
	unsigned int uSize{ 0 }, vSize{ 0 };							// uΪ�ڼ��У�vΪ�ڼ��С�ָ�ָ�ķ��������Ƕ�������������ΪuSize+1��vSize+1
	Vertex** vertex{ nullptr };										// ��ά���飬�洢��������
	unsigned int*** index{ nullptr };								// ��ά���飬�洢��������

	// ��Ⱦ��ص�����
	//GLuint VAO{ 0 }, VBO[4]{ 0,0,0,0 }, texture{ 0 };			// VBO�ֱ�Ϊposition��normal��index��texture
public:
	Mesh(unsigned int uSize = 2, unsigned int vSize = 2) :uSize(uSize), vSize(vSize) {
		index = new unsigned int** [vSize] {nullptr};
		for (unsigned int v = 0; v < vSize; v++) {
			index[v] = new unsigned int* [uSize] {nullptr};
			for (unsigned int u = 0; u < uSize; u++)
				index[v][u] = new unsigned int[6] {0};
		}

		vertex = new Vertex * [vSize + 1] {nullptr};
		for (unsigned int v = 0; v <= vSize; v++) {
			vertex[v] = new Vertex[uSize + 1];
			for (unsigned int u = 0; u <= uSize; u++) {
				vertex[v][u].position = vec3(0.0f);
				vertex[v][u].normal = vec3(0.0f);
				vertex[v][u].color = vec4(0.0f);
			}
		}
	}
	~Mesh() {
		delete[] index;
		for (unsigned int v = 0; v <= vSize; v++)
			delete[] vertex[v];
		delete[] vertex;
	}
	void connect() {
		// Ĭ�������������ӣ�ָ��1�������ʼ��2�У�
		for (unsigned int v = 0; v < vSize; v++) {
			for (unsigned int u = 0; u < uSize; u++) {
				unsigned int* ptr = index[v][u];		// ������ʱ������
				ptr[0] = v * (uSize + 1) + u;						// ����
				ptr[1] = (v + 1) * (uSize + 1) + u;			// ����
				ptr[2] = (v + 1) * (uSize + 1) + u + 1;		// ����
				ptr[3] = v * (uSize + 1) + u;						// ����
				ptr[4] = (v + 1) * (uSize + 1) + u + 1;		// ����
				ptr[5] = v * (uSize + 1) + u + 1;				// ����
			}
		}
	}
	//bool isTexture() {
	//	return glIsTexture(texture) == GL_TRUE;
	//}
	void updateVertexPositionByFunc(std::function<vec3(unsigned int, unsigned int)> func) {
		for (unsigned int v = 0; v <= vSize; v++) {
			for (unsigned int u = 0; u <= uSize; u++) {
				vertex[v][u].position = func(u, v);
			}
		}
	}
	void updateVertexNormalByFunc(std::function<vec3(unsigned int, unsigned int)> func) {
		for (unsigned int v = 0; v <= vSize; v++) {
			for (unsigned int u = 0; u <= uSize; u++) {
				vertex[v][u].normal = func(u, v);
			}
		}
	}
	void updateVertexNormalByFunc(std::function<vec3(float, float, float)> func) {
		vec3 pos;
		for (unsigned int v = 0; v <= vSize; v++) {
			for (unsigned int u = 0; u <= uSize; u++) {
				pos = vertex[v][u].position;
				vertex[v][u].normal = func(pos.x, pos.y, pos.z);
			}
		}
	}
	Vertex** getVertexPtr() {
		return vertex;
	}
	Vertex* mapVertexData() {		// ������ָ��洢�Ķ�������ӳ�䵽һά�����ռ䣬ֻ��
		vertexPtr = new Vertex[(uSize + 1) * (vSize + 1)];
		for (unsigned int v = 0; v <= vSize; v++) {
			for (unsigned int u = 0; u <= uSize; u++) {
				vertexPtr[v * (uSize + 1) + u] = vertex[v][u];
			}
		}
		return vertexPtr;
	}
	void unmapVertexData() {
		delete[] vertexPtr;
		vertexPtr = nullptr;
	}
	unsigned int* mapIndexData() {
		indexPtr = new unsigned int[uSize * vSize * 6];
		for (unsigned int v = 0; v < vSize; v++)
			for (unsigned int u = 0; u < uSize; u++)
				for (unsigned int idx = 0; idx < 6; idx++) {
					indexPtr[(v * uSize + u) * 6 + idx] = index[v][u][idx];
				}
		return indexPtr;
	}
	void unmapIndexData() {
		delete[] indexPtr;
		indexPtr = nullptr;
	}
	Triangle* mapAllTriangles() {
		trianglePtr = new Triangle[uSize * vSize * 2];
		Vertex* vtx = mapVertexData();
		for (unsigned int v = 0; v < vSize; v++) {
			for (unsigned int u = 0; u < uSize; u++) {
				trianglePtr[(v * uSize + u) * 2] = { vtx[index[v][u][0]],vtx[index[v][u][1]] ,vtx[index[v][u][2]] };
				trianglePtr[(v * uSize + u) * 2 + 1] = { vtx[index[v][u][3]],vtx[index[v][u][4]] ,vtx[index[v][u][5]] };
			}
		}
		unmapVertexData();
		return trianglePtr;
	}
	void unmapAllTriangles() {
		delete [] trianglePtr;
		trianglePtr = nullptr;
	}
	unsigned int getVertexSize() {
		return (uSize + 1) * (vSize + 1);
	}
	unsigned int getTriangleSize() {
		return uSize * vSize * 2;
	}
	unsigned int getIndexSize() {
		return uSize * vSize * 6;
	}
	//virtual void draw(Shader* sd) {		// Mesh������Ե������ƣ�����û��model��modelBuffer������λ����ص��Զ���ɫ
	//	sd->use();
	//	(*sd)["modelBuffer"] = mat4(1.0f);
	//	(*sd)["model"] = mat4(1.0f);
	//	(*sd)["isAuto"] = true;
	//	(*sd)["ncolor"] = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	//	glBindVertexArray(VAO);
	//	glDrawElements(GL_TRIANGLES, getIndexSize(), GL_UNSIGNED_INT, 0);
	//	glBindVertexArray(0);
	//}
};
class Geometry {
	// Geometry����û����������
private:
protected:
	mat4 modelBuffer{ mat4(1.0f) };
	Geometry* parent{ nullptr };
	vector<Geometry*> children;
	vector<Mesh*> meshes;
public:
	Transform model;		// ģ�;���
	Transform offset;			// ƫ�ƾ�����addChildʱ��¼�ӽڵ��뵱ǰ�ڵ��ƫ����(����λ�á���ת)�����ڸ��ӽڵ�������ϵ�任���ڴ�֮�ϵ���model�任
	GeometryType type{ DEFAULT };
	Geometry() {}
	~Geometry() {		// ��ɾ����ǰ���󣬻���ɾ�������Ӷ���ѡ�����
		for (auto& child : children)
			delete child;		// �����ݹ�ɾ�������Ӷ���
		for (auto& mesh : meshes) {
			delete mesh;
		}
		children.clear();
		meshes.clear();
	}
	vector<Geometry*>& getChildren() {
		return children;
	}
	vector<Mesh*>& getMeshes() {
		return meshes;
	}
	virtual void pose() = 0;
	// Ԥ�任
	mat4 getModelBufferMatrix() {
		return modelBuffer;
	}
	void applyTransform() {
		modelBuffer = (model.getMatrix()) * modelBuffer;
		model.reset();
	}
	// ��̬�任
	void translate(vec3 dxyz) {
		model.translate(dxyz);
	}
	void translateTo(vec3 dxyz) {
		model.translateTo(dxyz);
	}
	void rotate(float angle, vec3 axis) {
		model.rotate(angle, axis);
		//offset.rotate(angle, axis);
	}
	void scale(vec3 xyz) {
		model.scale(xyz);
	}
	void scaleTo(vec3 xyz) {
		model.scaleTo(xyz);
	}
	// ���ṹ����
	void addChild(Geometry* obj, const Transform& offset = Transform()) {		// ��ӵ�ʱ����Ҫָ���ӽڵ�(pose���)���λ������ڸ��ڵ�����pose����ϵ��λ��ƫ��
		if (obj != nullptr) {
			obj->setParent(this);
			obj->offset = offset;
			children.push_back(obj);
		}
	}
	void setParent(Geometry* obj) {
		this->parent = obj;
	}
	mat4 getFinalOffset() {	 // �������и��ڵ���㵱ǰ�ڵ��ƫ�ƾ���
		mat4 offsetMatrix(offset.getMatrix());
		Geometry* cur = this;
		while (cur->parent) {
			cur = cur->parent;
			offsetMatrix = (cur->offset.getMatrix()) * (cur->model.getMatrix()) * offsetMatrix;
		}
		return offsetMatrix;
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
		meshes.push_back(new Mesh(xSliceNum, ySliceNum));
		meshes.push_back(new Mesh(xSliceNum, ySliceNum));
		meshes.push_back(new Mesh(xSliceNum, zSliceNum));
		meshes.push_back(new Mesh(xSliceNum, zSliceNum));
		meshes.push_back(new Mesh(ySliceNum, zSliceNum));
		meshes.push_back(new Mesh(ySliceNum, zSliceNum));

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

		meshes.push_back(new Mesh(latSliceNum, lonSliceNum));
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

		meshes.push_back(new Mesh(rSliceNum, lonSliceNum));
		meshes.push_back(new Mesh(rSliceNum, lonSliceNum));
		meshes.push_back(new Mesh(hSliceNum, lonSliceNum));

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
		model.translateTo(vec3(0.0f, height / 2.0f, 0.0f));
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

		meshes.push_back(new Mesh(rSliceNum, lonSliceNum));
		meshes.push_back(new Mesh(hSliceNum, lonSliceNum));

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
		model.translateTo(vec3(0.0f, height / 2.0f, 0.0f));
		applyTransform();
	}
};
class Arrow : public Geometry {
private:
	vec3 _begin{ vec3(0.0f,0.0f,0.0f) }, _end{ vec3(0.0f,1.0f,0.0f) };
	float width{ 0.03f };
	vec3 arrowColor{ vec3(1.0f,1.0f,0.0f) }, bodyColor{ vec3(1.0f,0.0f,0.0f) };
	Geometry* arrow, * body;

	float arrowLength = 0.2f;
	const float arrowLengthRatio = 0.2f;
	const float arrowRadiusRatio = 2.5f;
public:
	// Ŀǰû��ʵ�ֶ�̬�ı��ͷ���ȵĹ��ܣ�ֻ���ڹ��캯����ָ��
	// �ı�ָ��λ��ֻ��ͨ���任ʵ�֣��һ��ԭ�ȼ�ͷ�ĳ�������Ӱ��
	Arrow(vec3 _begin, vec3 _end, vec3 bodyColor) :_begin(_begin), _end(_end), bodyColor(bodyColor) {
		float len = length(_end - _begin);
		vec3 direction = normalize(_end - _begin);

		if (len <= arrowLength) {
			std::cout << "���ȹ��̣�������0.2�������Ų��ԣ���Ӱ���ͷ�����" << std::endl;
			arrowLength = len * arrowLengthRatio;
		}

		arrow = new Cone(width * arrowRadiusRatio, arrowLength);
		body = new Cylinder(width, len - arrowLength);
		body->addChild(arrow, Transform(vec3(0.0f, len - arrowLength, 0.0f)));
		this->addChild(body, Transform());		// �������������this�У�������this�ı任��ͬʱ��������������

		model.reset();
		pose();

		// ��ͷԭʼ��̬Ϊ��(0,0,0)ָ��(0,height,0)��ͨ���任����ӳ��Ϊ��_begin��_end
		vec3 rotate_axis = cross(_up, direction);
		float rotate_radian = acos(dot(vec3(0.0f, 1.0f, 0.0f), direction));
		if (abs(rotate_radian) < MEPS) {		// ��ȫƽ��ʱrotate_axis�ӽ�ȫ��ᵼ��rotate�������ó���
			rotate_axis = vec3(0.0f, 0.0f, 1.0f);
			rotate_radian = 0.0f;
		}
		else if (abs(rotate_radian - PI) < MEPS) {
			rotate_axis = vec3(0.0f, 0.0f, 1.0f);
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
		axis_z = new Arrow(_origin, _front, _front);
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

//class Plane : public Drawable {
//	// ��ƽ�棬���̳���Geometry�������������������
//protected:
//	GLuint VAO{ 0 }, VBO[2]{ 0,0 };		// ֻ��Ҫ4�����㣬ʹ�ó���Ϊ6���������壬����Ҫ������
//	vec4 color{ 0.5f,0.5f,0.5f,0.2f };
//public:
//	Plane(Shader* shader, vec3 p[4]) {
//		this->shader = shader;
//
//		glGenVertexArrays(1, &VAO);
//		glBindVertexArray(VAO);
//		glGenBuffers(2, VBO);
//
//		glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
//		glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(vec3), p, GL_DYNAMIC_DRAW);
//		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
//		glEnableVertexAttribArray(0);
//
//		unsigned int indices[6] = { 0,1,2,0,2,3 };
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[1]);
//		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW);
//
//		glBindVertexArray(0);
//	}
//};
//class Line :public Drawable {
//	// ��������Ҫ����������������û���Ǽ̳���Geometry
//protected:
//	GLuint VAO, VBO;		// ����ֻ��Ҫ�������㣬����Ҫ����������Ҳ����Ҫ������
//	vec4 color{ 1.0f,0.0f,0.0f,1.0f };
//public:
//	Line(Shader* lineShader, vec3 start = { 0.0f,0.0f,0.0f }, vec3 end = { 0.0f,0.0f,0.0f }) {
//		shader = lineShader;	// ��������ר�õ���ɫ��
//
//		// ��ʼ�������˵�VBO
//		glGenVertexArrays(1, &VAO);
//		glBindVertexArray(VAO);
//		glGenBuffers(1, &VBO);
//		glBindBuffer(GL_ARRAY_BUFFER, VBO);
//		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * 2, NULL, GL_DYNAMIC_DRAW);
//		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
//		glEnableVertexAttribArray(0);
//		glBindVertexArray(0);
//		// ���������˵�VBO
//		setStartPoint(start);
//		setEndPoint(end);
//	}
//	void setColor(vec4 color) {
//		this->color = toGlm(color);
//	}
//	void setStartPoint(vec3 start) {
//		glNamedBufferSubData(VBO, 0, sizeof(vec3), &start);
//	}
//	void setEndPoint(vec3 end) {
//		glNamedBufferSubData(VBO, sizeof(vec3), sizeof(vec3), &end);
//	}
//
//	virtual void draw(Shader* sd) {	// ��ʹ�ò�����shader������ʹ�����ڲ���shader
//		shader->use();
//		(*shader)["color"] = color;
//		glBindVertexArray(VAO);
//		glDrawArrays(GL_LINES, 0, 2);
//		glBindVertexArray(0);
//	}
//};

#endif
