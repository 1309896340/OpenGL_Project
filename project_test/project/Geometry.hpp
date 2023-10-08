#include "proj.h"
#include "utils.h"
#include "Shader.hpp"

#pragma once

typedef struct {
	vec3 begin;
	vec3 end;
	vec4 color;
	float width;
} Line;

typedef struct {
	std::vector<Line> lines;	// ����������
	GLuint vbo_line;
	Shader* shader;
}LineStructure;


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

class Geometry {
private:
	glm::mat4 transMatrix;
	glm::mat4 modelBuffer;
protected:
	GLuint VAO{ 0 }, VBO[3]{ 0,0,0 };
	GLsizei index_size{ 0 };
	uniformTable attribute;
public:
	Transform transform;

	Geometry(glm::vec3 position) : modelBuffer(glm::mat4(1.0f)), transMatrix(glm::mat4(1.0f)) {}
	Geometry() : modelBuffer(glm::mat4(1.0f)), transMatrix(glm::mat4(1.0f)) {}
	Geometry(const std::vector<vec3>& vertex, const std::vector<vec3>& normal, const std::vector<GLuint>& index) :
		modelBuffer(glm::mat4(1.0f)), transMatrix(glm::mat4(1.0f)) {
		prepareVAO(vertex, normal, index, &VAO, VBO, &index_size);
	}
	~Geometry() {}

	glm::mat4 getModelBufferMatrix() {
		return modelBuffer;
	}
	void setColor(glm::vec4 color) {
		attribute.autoColor = false;
		attribute.color = color;
	}
	void setAutoColor() {
		attribute.autoColor = true;
	}
	void applyTransform() {
		modelBuffer = (transform.getMatrix()) * modelBuffer;
		transform.reset();
	}
	void setTransform(const Transform& trans) {
		transform = trans;
	}
	void setTransformMatrix(const glm::mat4& trans) {
		transMatrix = trans;
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

	uniformTable& getAttribute() {
		return attribute;
	}
	GLuint getVAO() {
		return VAO;
	}
	GLsizei getVAOLength() {
		return index_size;
	}
};

class Cube : public Geometry {
private:
	int xSliceNum, ySliceNum, zSliceNum;
	float xLength, yLength, zLength;
public:
	Cube(float xLength, float yLength, float zLength, int xSliceNum=10, int ySliceNum=10, int zSliceNum=10) :
		Geometry(), xSliceNum(xSliceNum), ySliceNum(ySliceNum), zSliceNum(zSliceNum),
		xLength(xLength), yLength(yLength), zLength(zLength) {

		float dx, dy, dz;
		dx = xLength / xSliceNum;
		dy = yLength / ySliceNum;
		dz = zLength / zSliceNum;

		int xyNum = xSliceNum * ySliceNum;
		int zyNum = ySliceNum * zSliceNum;
		int xzNum = xSliceNum * zSliceNum;

		int xyNum1 = (xSliceNum + 1) * (ySliceNum + 1);
		int zyNum1 = (ySliceNum + 1) * (zSliceNum + 1);
		int xzNum1 = (xSliceNum + 1) * (zSliceNum + 1);

		std::vector<vec3> vertex(2 * (xyNum1 + zyNum1 + xzNum1), { 0.0f,0.0f,0.0f });
		std::vector<vec3> normal(2 * (xyNum1 + zyNum1 + xzNum1), { 0.0f,0.0f,0.0f });
		std::vector<GLuint> index(2 * (xyNum + zyNum + xzNum) * 6, 0);

		int baseVert = 0, baseIdx = 0;
		for (int i = 0; i <= xSliceNum; i++) {
			for (int j = 0; j <= ySliceNum; j++) {
				vertex[baseVert + i * (ySliceNum + 1) + j] = { -xLength / 2 + i * dx, -yLength / 2 + j * dy, -zLength / 2 };
				vertex[baseVert + xyNum1 + i * (ySliceNum + 1) + j] = { -xLength / 2 + i * dx, -yLength / 2 + j * dy, zLength / 2 };
				normal[baseVert + i * (ySliceNum + 1) + j] = { 0.0f,0.0f,-1.0f };
				normal[baseVert + xyNum1 + i * (ySliceNum + 1) + j] = { 0.0f,0.0f,1.0f };
				if (i < xSliceNum && j < ySliceNum) {
					for (int k = 0; k < 2; k++) {
						unsigned int* ptr = &index[baseIdx + (k * xyNum + i * ySliceNum + j) * 6];
						*ptr++ = baseVert + k * xyNum1 + i * (ySliceNum + 1) + j;
						*ptr++ = baseVert + k * xyNum1 + i * (ySliceNum + 1) + j + 1;
						*ptr++ = baseVert + k * xyNum1 + (i + 1) * (ySliceNum + 1) + j + 1;
						*ptr++ = baseVert + k * xyNum1 + i * (ySliceNum + 1) + j;
						*ptr++ = baseVert + k * xyNum1 + (i + 1) * (ySliceNum + 1) + j + 1;
						*ptr++ = baseVert + k * xyNum1 + (i + 1) * (ySliceNum + 1) + j;
					}
				}
			}
		}
		baseVert += 2 * xyNum1;
		baseIdx += 2 * xyNum * 6;
		for (int i = 0; i <= zSliceNum; i++) {
			for (int j = 0; j <= ySliceNum; j++) {
				vertex[baseVert + i * (ySliceNum + 1) + j] = { -xLength / 2, -yLength / 2 + j * dy, -zLength / 2 + i * dz };
				vertex[baseVert + zyNum1 + i * (ySliceNum + 1) + j] = { xLength / 2, -yLength / 2 + j * dy, -zLength / 2 + i * dz };
				normal[baseVert + i * (ySliceNum + 1) + j] = { -1.0f,0.0f,0.0f };
				normal[baseVert + zyNum1 + i * (ySliceNum + 1) + j] = { 1.0f,0.0f,0.0f };
				if (i < zSliceNum && j < ySliceNum) {
					for (int k = 0; k < 2; k++) {
						unsigned int* ptr = &index[baseIdx + (k * zyNum + i * ySliceNum + j) * 6];
						*ptr++ = baseVert + k * zyNum1 + i * (ySliceNum + 1) + j;
						*ptr++ = baseVert + k * zyNum1 + i * (ySliceNum + 1) + j + 1;
						*ptr++ = baseVert + k * zyNum1 + (i + 1) * (ySliceNum + 1) + j + 1;
						*ptr++ = baseVert + k * zyNum1 + i * (ySliceNum + 1) + j;
						*ptr++ = baseVert + k * zyNum1 + (i + 1) * (ySliceNum + 1) + j + 1;
						*ptr++ = baseVert + k * zyNum1 + (i + 1) * (ySliceNum + 1) + j;
					}
				}
			}
		}
		baseVert += 2 * zyNum1;
		baseIdx += 2 * zyNum * 6;
		for (int i = 0; i <= xSliceNum; i++) {
			for (int j = 0; j <= zSliceNum; j++) {
				vertex[baseVert + i * (zSliceNum + 1) + j] = { -xLength / 2 + i * dx, -yLength / 2, -zLength / 2 + j * dz };
				vertex[baseVert + xzNum1 + i * (zSliceNum + 1) + j] = { -xLength / 2 + i * dx, yLength / 2, -zLength / 2 + j * dz };
				normal[baseVert + i * (zSliceNum + 1) + j] = { 0.0f,-1.0f,0.0f };
				normal[baseVert + xzNum1 + i * (zSliceNum + 1) + j] = { 0.0f,1.0f,0.0f };
				if (i < xSliceNum && j < zSliceNum) {
					for (int k = 0; k < 2; k++) {
						unsigned int* ptr = &index[baseIdx + (k * xzNum + i * zSliceNum + j) * 6];
						*ptr++ = baseVert + k * xzNum1 + i * (zSliceNum + 1) + j;
						*ptr++ = baseVert + k * xzNum1 + i * (zSliceNum + 1) + j + 1;
						*ptr++ = baseVert + k * xzNum1 + (i + 1) * (zSliceNum + 1) + j + 1;
						*ptr++ = baseVert + k * xzNum1 + i * (zSliceNum + 1) + j;
						*ptr++ = baseVert + k * xzNum1 + (i + 1) * (zSliceNum + 1) + j + 1;
						*ptr++ = baseVert + k * xzNum1 + (i + 1) * (zSliceNum + 1) + j;
					}
				}
			}
		}
		prepareVAO(vertex, normal, index, &VAO, VBO, &index_size);
	}
};

class Sphere : public Geometry {
private:
	float radius;
	int lonSliceNum;
	int latSliceNum;

public:
	Sphere(float radius, unsigned int lonSliceNum=36, unsigned int latSliceNum=20) :Geometry(), radius(radius), lonSliceNum(lonSliceNum), latSliceNum(latSliceNum) {
		float lonStep = 2 * PI / lonSliceNum;
		float latStep = PI / latSliceNum;

		unsigned int lonlatNum = lonSliceNum * latSliceNum;
		unsigned int lonlatNum1 = (latSliceNum + 1) * (lonSliceNum + 1);

		std::vector<vec3> vertex(lonlatNum1, { 0.0f,0.0f,0.0f });
		std::vector<vec3> normal(lonlatNum1, { 0.0f,0.0f,0.0f });
		std::vector<GLuint> index(lonlatNum * 6, 0);

		for (unsigned int i = 0; i <= lonSliceNum; i++) {
			for (unsigned int j = 0; j <= latSliceNum; j++) {
				float lon = -PI + i * lonStep;
				float lat = -PI / 2 + j * latStep;
				vertex[i * (latSliceNum + 1) + j] = { radius * cos(lat) * cos(lon), radius * cos(lat) * sin(lon), radius * sin(lat) };
				normal[i * (latSliceNum + 1) + j] = { cos(lat) * cos(lon), cos(lat) * sin(lon), sin(lat) };
				if (i < lonSliceNum && j < latSliceNum) {
					unsigned int* ptr = &index[(i * latSliceNum + j) * 6];
					*ptr++ = i * (latSliceNum + 1) + j;
					*ptr++ = i * (latSliceNum + 1) + j + 1;
					*ptr++ = (i + 1) * (latSliceNum + 1) + j + 1;
					*ptr++ = i * (latSliceNum + 1) + j;
					*ptr++ = (i + 1) * (latSliceNum + 1) + j + 1;
					*ptr++ = (i + 1) * (latSliceNum + 1) + j;
				}
			}
		}
		prepareVAO(vertex, normal, index, &VAO, VBO, &index_size);
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
	Cylinder(float radius, float height, unsigned int rSliceNum=10, unsigned int hSliceNum=20, unsigned int lonSliceNum=36) :Geometry(), radius(radius), height(height), rSliceNum(rSliceNum), hSliceNum(hSliceNum), lonSliceNum(lonSliceNum) {
		float rStep = radius / rSliceNum;
		float hStep = height / hSliceNum;
		float lonStep = 2 * PI / lonSliceNum;

		unsigned int rlonNum = rSliceNum * lonSliceNum;
		unsigned int hlonNum = hSliceNum * lonSliceNum;
		unsigned int rlonNum1 = (rSliceNum + 1) * (lonSliceNum + 1);
		unsigned int hlonNum1 = (hSliceNum + 1) * (lonSliceNum + 1);

		float lon_tmp, r_tmp, h_tmp;

		int baseVert = 0, baseIdx = 0;

		std::vector<vec3> vertex(2 * rlonNum1 + hlonNum1, { 0.0f,0.0f,0.0f });
		std::vector<vec3> normal(2 * rlonNum1 + hlonNum1, { 0.0f,0.0f,0.0f });
		std::vector<GLuint> index((2 * rlonNum + hlonNum) * 6, 0);

		for (unsigned int i = 0; i <= lonSliceNum; i++) {
			for (unsigned int j = 0; j <= rSliceNum; j++) {
				lon_tmp = -PI + i * lonStep;
				r_tmp = j * rStep;
				vertex[baseVert + i * (rSliceNum + 1) + j] = { r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), -height / 2 };
				vertex[baseVert + rlonNum1 + i * (rSliceNum + 1) + j] = { r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), height / 2 };
				normal[baseVert + i * (rSliceNum + 1) + j] = { 0.0f,0.0f,-1.0f };
				normal[baseVert + rlonNum1 + i * (rSliceNum + 1) + j] = { 0.0f,0.0f,1.0f };
				if (i < lonSliceNum && j < rSliceNum) {
					for (int k = 0; k < 2; k++) {
						unsigned int* ptr = &index[baseIdx + (k * rlonNum + i * rSliceNum + j) * 6];
						*ptr++ = baseVert + k * rlonNum1 + i * (rSliceNum + 1) + j;
						*ptr++ = baseVert + k * rlonNum1 + i * (rSliceNum + 1) + j + 1;
						*ptr++ = baseVert + k * rlonNum1 + (i + 1) * (rSliceNum + 1) + j + 1;
						*ptr++ = baseVert + k * rlonNum1 + i * (rSliceNum + 1) + j;
						*ptr++ = baseVert + k * rlonNum1 + (i + 1) * (rSliceNum + 1) + j + 1;
						*ptr++ = baseVert + k * rlonNum1 + (i + 1) * (rSliceNum + 1) + j;
					}
				}
			}
		}
		baseVert += 2 * rlonNum1;
		baseIdx += 2 * rlonNum * 6;
		for (unsigned int i = 0; i <= hSliceNum; i++) {
			for (unsigned int j = 0; j <= lonSliceNum; j++) {
				h_tmp = -height / 2 + i * hStep;
				lon_tmp = -PI + j * lonStep;
				vertex[baseVert + i * (lonSliceNum + 1) + j] = { radius * cos(lon_tmp),radius * sin(lon_tmp) , h_tmp };
				normal[baseVert + i * (lonSliceNum + 1) + j] = { cos(lon_tmp), sin(lon_tmp), 0.0f };
				if (i < hSliceNum && j < lonSliceNum) {
					unsigned int* ptr = &index[baseIdx + (i * lonSliceNum + j) * 6];
					*ptr++ = baseVert + i * (lonSliceNum + 1) + j;
					*ptr++ = baseVert + i * (lonSliceNum + 1) + j + 1;
					*ptr++ = baseVert + (i + 1) * (lonSliceNum + 1) + j + 1;
					*ptr++ = baseVert + i * (lonSliceNum + 1) + j;
					*ptr++ = baseVert + (i + 1) * (lonSliceNum + 1) + j + 1;
					*ptr++ = baseVert + (i + 1) * (lonSliceNum + 1) + j;
				}
			}
		}
		prepareVAO(vertex, normal, index, &VAO, VBO, &index_size);
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
	Cone(float radius, float height, unsigned int rSliceNum=10, unsigned int hSliceNum=20, unsigned int lonSliceNum=36) :Geometry(), radius(radius), height(height), rSliceNum(rSliceNum), hSliceNum(hSliceNum), lonSliceNum(lonSliceNum) {
		float rStep = radius / rSliceNum;
		float hStep = height / hSliceNum;
		float lonStep = 2 * PI / lonSliceNum;

		unsigned int rlonNum = rSliceNum * lonSliceNum;
		unsigned int hlonNum = hSliceNum * lonSliceNum;
		unsigned int rlonNum1 = (rSliceNum + 1) * (lonSliceNum + 1);
		unsigned int hlonNum1 = (hSliceNum + 1) * (lonSliceNum + 1);

		float lon_tmp, r_tmp, h_tmp;

		int baseVert = 0, baseIdx = 0;

		std::vector<vec3> vertex(rlonNum1 + hlonNum1, { 0.0f,0.0f,0.0f });
		std::vector<vec3> normal(rlonNum1 + hlonNum1, { 0.0f,0.0f,0.0f });
		std::vector<GLuint> index((rlonNum + hlonNum) * 6, 0);

		for (unsigned int i = 0; i <= lonSliceNum; i++) {
			for (unsigned int j = 0; j <= rSliceNum; j++) {
				lon_tmp = -PI + i * lonStep;
				r_tmp = j * rStep;
				vertex[baseVert + i * (rSliceNum + 1) + j] = { r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), -height / 2 };
				normal[baseVert + i * (rSliceNum + 1) + j] = { 0.0f,0.0f,-1.0f };
				if (i < lonSliceNum && j < rSliceNum) {
					unsigned int* ptr = &index[baseIdx + (i * rSliceNum + j) * 6];
					*ptr++ = baseVert + i * (rSliceNum + 1) + j;
					*ptr++ = baseVert + i * (rSliceNum + 1) + j + 1;
					*ptr++ = baseVert + (i + 1) * (rSliceNum + 1) + j + 1;
					*ptr++ = baseVert + i * (rSliceNum + 1) + j;
					*ptr++ = baseVert + (i + 1) * (rSliceNum + 1) + j + 1;
					*ptr++ = baseVert + (i + 1) * (rSliceNum + 1) + j;
				}
			}
		}
		baseVert += rlonNum1;
		baseIdx += rlonNum * 6;
		for (unsigned int i = 0; i <= hSliceNum; i++) {
			for (unsigned int j = 0; j <= lonSliceNum; j++) {
				h_tmp = -height / 2 + i * hStep;
				lon_tmp = -PI + j * lonStep;
				r_tmp = radius * (1 - i * hStep / height);
				vertex[baseVert + i * (lonSliceNum + 1) + j] = { r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp) , h_tmp };
				float tmp = sqrt(radius * radius + height * height);
				normal[baseVert + i * (lonSliceNum + 1) + j] = { height * cos(lon_tmp) / tmp,height * sin(lon_tmp) / tmp, radius / tmp };
				if (i < hSliceNum && j < lonSliceNum) {
					unsigned int* ptr = &index[baseIdx + (i * lonSliceNum + j) * 6];
					*ptr++ = baseVert + i * (lonSliceNum + 1) + j;
					*ptr++ = baseVert + i * (lonSliceNum + 1) + j + 1;
					*ptr++ = baseVert + (i + 1) * (lonSliceNum + 1) + j + 1;
					*ptr++ = baseVert + i * (lonSliceNum + 1) + j;
					*ptr++ = baseVert + (i + 1) * (lonSliceNum + 1) + j + 1;
					*ptr++ = baseVert + (i + 1) * (lonSliceNum + 1) + j;
				}
			}
		}
		prepareVAO(vertex, normal, index, &VAO, VBO, &index_size);
	}
};


typedef float (*zFunc)(float x, float y);
typedef vec3(*zGrad)(float x, float y, float z);

class Surface : public Geometry {
private:
	int xSliceNum;
	int ySliceNum;
	float xStart, xEnd;
	float yStart, yEnd;
	zFunc func;
	zGrad grad;
public:
	Surface(float xStart, float xEnd, float yStart, float yEnd, zFunc func, zGrad grad, unsigned int xSliceNum, unsigned int ySliceNum) :
		Geometry(), xStart(xStart), xEnd(xEnd), yStart(yStart), yEnd(yEnd), func(func), grad(grad), xSliceNum(xSliceNum), ySliceNum(ySliceNum) {
		float dx = (xEnd - xStart) / xSliceNum;
		float dy = (yEnd - yStart) / ySliceNum;
		float x, y, z;
		vec3 _grad;

		unsigned int lonlatNum = xSliceNum * ySliceNum;
		unsigned int lonlatNum1 = (xSliceNum + 1) * (ySliceNum + 1);

		std::vector<vec3> vertex(lonlatNum1, { 0.0f,0.0f,0.0f });
		std::vector<vec3> normal(lonlatNum1, { 0.0f,0.0f,0.0f });
		std::vector<GLuint> index(lonlatNum * 6, 0);

		for (unsigned int i = 0; i <= xSliceNum; i++) {
			for (unsigned int j = 0; j <= ySliceNum; j++) {
				x = xStart + i * dx;
				y = yStart + j * dy;
				z = func(x, y);
				_grad = grad(x, y, z);

				vertex[i * (ySliceNum + 1) + j] = { x,y,z };
				normal[i * (ySliceNum + 1) + j] = { _grad.x, _grad.y, _grad.z };
				if (i < xSliceNum && j < ySliceNum) {
					unsigned int* ptr = &index[(i * ySliceNum + j) * 6];
					*ptr++ = i * (ySliceNum + 1) + j;
					*ptr++ = i * (ySliceNum + 1) + j + 1;
					*ptr++ = (i + 1) * (ySliceNum + 1) + j + 1;
					*ptr++ = i * (ySliceNum + 1) + j;
					*ptr++ = (i + 1) * (ySliceNum + 1) + j + 1;
					*ptr++ = (i + 1) * (ySliceNum + 1) + j;
				}
			}
		}
		prepareVAO(vertex, normal, index, &VAO, VBO, &index_size);
	}
};


LineStructure lineManager;

void initLineDrawing(Shader* shader) {
	lineManager.shader = shader;
	glGenBuffers(1, &lineManager.vbo_line);
	glBindBuffer(GL_ARRAY_BUFFER, lineManager.vbo_line);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), NULL, GL_STATIC_DRAW);
}
void showLines() {
	Shader& shader = *lineManager.shader;
	shader.use();
	shader["model"] = glm::mat4(1.0f);
	shader["modelBuffer"] = glm::mat4(1.0f);
	shader["isAuto"] = false;

	while (!lineManager.lines.empty()) {
		Line a = lineManager.lines.back();
		glBindBuffer(GL_ARRAY_BUFFER, lineManager.vbo_line);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, &a.begin, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);
		glEnableVertexAttribArray(0);
		shader["ncolor"] = glm::vec4(a.color.x, a.color.y, a.color.z, a.color.w);
		glLineWidth(a.width);
		glDrawArrays(GL_LINES, 0, 2);
		glLineWidth(DEFAULT_LINE_WIDTH);
		lineManager.lines.pop_back();
	}
}
void drawLine(glm::vec3 begin, glm::vec3 end, glm::vec3 color, float width, Shader* shader) {
	Line a;
	a.begin = { begin.x,begin.y,begin.z };
	a.end = { end.x,end.y,end.z };
	a.color = { color.x,color.y,color.z,1.0f };
	a.width = width;

	lineManager.lines.push_back(a);
}


class Combination : public Geometry {
	// Combination��������϶��Geometry������̳���Geometry�࣬����translate��rotate��scale��draw�ȷ���
private:
	std::vector<Geometry*> objs;
	std::vector<glm::mat4> objModel;
public:
	// CombinationӦ�������Լ���shader������ʹ��objs�и���obj�Լ���shader
	// VAO��VAO_lengthҲӦ��ʹ��obj�����
	Combination() :Geometry() {}
	void add(Geometry* obj) {
		// CombinationĬ�ϵ�������ԭ�㴦
		// �����obj�Ὣ��ǰ�����ģ�ͱ任model��ΪCombination��objModel������������modelΪ��λ��
		objs.push_back(obj);
		objModel.push_back(obj->transform.getMatrix());
		obj->resetTransform();
	}

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
};

class Bone {
private:
	Bone* child{ 0 };			// ��ʱֻ����һ���ӹ���
	Bone* parent{ 0 };

	glm::vec3 position{ 0.0f,0.0f,0.0f };			// ���λ�ã���������λ������Ч�ģ����������ӹ������ߵ�λ���ɸ�����ȷ��
	glm::vec3 vec{ 0.0f,0.0f,0.0f };			// �����������������С��ʾ�����ĳ��ȣ������ʾ�����ķ���

	// �����õ�Բ������Ϊ����ʵ��
	Geometry* obj = nullptr;

	// ÿ��Bone��Ҫ������ֹλ�ã�����һ������һ������С�ķ�������
	// ������ͨ���ӹ�������ʼλ�úͷ���������ȷ���Լ���λ�ã�ͨ�������һ��λ�ƾ���
public:
	Transform transform; // ��ʾ��ǰ�������ӹ�������ڵ�ǰ�����ı任
	Bone(float length = 1.0f) : vec(length* _up) {
		obj = new Cylinder(0.04f, length, 4, 20, 36);
		glm::mat4 transMatrix(1.0f);
		obj->rotate(glm::radians(-90.0f), _right);
		obj->translateTo(glm::vec3(0.0f, length / 2, 0.0f));
		obj->applyTransform();	// �����ĵ��ƶ���Բ�����¶˵�

		transform.translateTo(vec);	// �任����Ϊ����ĩ������ڹ�����ʼ���ƽ�ƾ���
	}
	~Bone() {
		if (child) {
			delete child;
		}
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
	Bone* getChild() {	// ��ʱֻ����һ���ӹ���
		return child;
	}
	void addChild(Bone* b) {	// ��ʱֻ����һ���ӹ���
		if (child)
			child->addChild(b);
		else {
			child = b;
			b->parent = this;
			child->position = position + vec;
		}
	}
	// ���һЩ�Ե�����������ת�������������Ǹı�vec�ķ���(���ı��С)���������ӹ�����λ��
	void rotate(float angle, glm::vec3 axis) {
		vec = glm::rotate(glm::mat4(1.0f), angle, axis) * glm::vec4(vec, 1.0f);
		if (child) {
			child->position = position + vec;
		}
		transform.rotate(angle, axis);	// �����ӹ����ı任������Լ�Ҫ������ת��Ҫ����λ��
		transform.translateTo(vec);

		obj->rotate(angle, axis);	// �Ե�ǰ���ƵĹ�������ֻ����תû��λ��
	}
	Geometry* getObj() {
		return obj;
	}
	//virtual void draw() {	// �����õĻ��ƺ���
	//	obj->setTransformMatrix(getTransMatrix());
	//	obj->draw();
	//}
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

		arrow->setColor(arrowColor);
		body->setColor(bodyColor);
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
	Axis() {
		axis_x = new Arrow(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.06f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		axis_y = new Arrow(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.f, 0.0f), 0.06f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		axis_z = new Arrow(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.06f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
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
	float width, height, theta;
	unsigned int wSliceNum, hSliceNum;

	const unsigned int u_degree = 3, v_degree = 2;		// uΪ���ȷָvΪ��ȷָ�
	const float k = 10.0f, SLAngle = PI / 6.0f;			// kΪҶƬ�����̶ȣ�SLAngleΪҶƬ�����Ƕ�

	float wFunc(float h) {
		// ����ҶƬ��ȹ��ڳ��ȵĺ���
		return (-1.0f / powf(height, 2) * powf(h, 2) + 2 * theta / height * h + (1 - 2 * theta)) * width / powf(1 - theta, 2);
	}
	float veinFunc(float x) {
		// ����Ҷ�����ڳ��ȵĺ���
		return -k * width / height * x * x + tanf(PI / 2 - SLAngle) * x;
	}
public:
	Leaf(float width, float height, unsigned int wSliceNum, unsigned int hSliceNum) :
		Geometry(), width(width), height(height), wSliceNum(wSliceNum), hSliceNum(hSliceNum), theta(0.35f) {

		if (wSliceNum < v_degree + 1 || hSliceNum < u_degree + 1) {
			std::cout << "�ڵ�ָ���̫��" << std::endl;
			exit(0);
		}

		tinynurbs::array2<glm::vec3> ctrl_pts(hSliceNum, wSliceNum, glm::vec3(0.0f));
		tinynurbs::array2<float> weight(hSliceNum, wSliceNum, 1.0f);

		// ���ɽڵ�
		std::vector<float> knots_u(hSliceNum + u_degree + 1, 0), knots_v(wSliceNum + v_degree + 1, 0);
		for (unsigned int i = 0; i <= u_degree; i++) {
			knots_u[i] = 0.0f;
			knots_u[hSliceNum + u_degree - i] = 1.0f;
		}
		for (unsigned int i = u_degree + 1; i < hSliceNum; i++) {
			knots_u[i] = (float)(i - u_degree) / (hSliceNum - u_degree);
		}
		for (unsigned int i = 0; i <= v_degree; i++) {
			knots_v[i] = 0.0f;
			knots_v[wSliceNum + v_degree - i] = 1.0f;
		}
		for (unsigned int i = v_degree + 1; i < wSliceNum; i++) {
			knots_v[i] = (float)(i - v_degree) / (wSliceNum - v_degree);
		}
		// ���ɿ��Ƶ㣨Ҷ�ʡ�Ҷ���ߡ�Ҷ����
		//float x;
		//for (int i = 0; i < hSliceNum; i++) {
		//	for (int j = 0; j < wSliceNum; j++) {
		//		x = (float)i / (hSliceNum - 1) * height;
		//		ctrl_pts(i, j) = glm::vec3(
		//			x,
		//			veinFunc(x),
		//			(j - (wSliceNum - 1) / 2.0f) / ((wSliceNum - 1) / 2.0f) * wFunc(x) / 2.0f
		//		);
		//	}
		//}

		//tinynurbs::RationalSurface3f srf(
		//	u_degree, v_degree, knots_u, knots_v, ctrl_pts, weight
		//);

		//assert(tinynurbs::surfaceIsValid(srf));


		// �Ȳ�ʹ��NURBS���棬ֱ������
		int hwNum = hSliceNum * wSliceNum, hwNum1 = (hSliceNum + 1) * (wSliceNum + 1);

		std::vector<vec3> vertex(hwNum1, { 0 });
		std::vector<vec3> normal(hwNum1, { 0 });	// û�����ɷ�����
		std::vector<GLuint> index(hwNum1 * 6, 0);

		float x, y;
		for (unsigned int i = 0; i <= hSliceNum; i++) {
			x = (float)i / hSliceNum * height;
			for (unsigned int j = 0; j <= wSliceNum; j++) {
				y = (j - wSliceNum / 2.0f) / (wSliceNum / 2.0f) * wFunc(x);
				vertex[i * (wSliceNum + 1) + j] = { x,veinFunc(x),y };
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
		}
		// Ŀǰû�д����Ҷ�����̣�Ҳû��ʹ��NURBS������������ʱ��֪����δ���
		// Leaf����Ӧ�����м������º�����Ӧ��ʱ����VBO��ֵ
		// 1.����Ҷ�������ȵ�k
		// 2. ��Ҷ�нǵ�theta

		//assert(wSliceNum % 2 == 0);	// �ٶ�wSliceNum����Ϊż��
		//unsigned int veinIdx = (unsigned int)(wSliceNum / 2);
		//x = 0.0f;
		//y = 0.0f;
		//float a = -k * width / height, b = tanf(PI / 2 - SLAngle), tmp1, tmp2;
		//for (int i = 0; i <= hSliceNum; i++) {
		//	//tmp1 = 2 * a * x + b;
		//	//tmp2 = height / (hSliceNum + 1) / sqrtf(1 + powf(tmp1, 2));
		//	//x = x + tmp2;
		//	//y = y + tmp1 * tmp2;
		//	//vertex[i * (wSliceNum + 1) + veinIdx] = { x,y,0 };
		//	x = vertex[i * (wSliceNum + 1) + veinIdx].x;
		//	tmp1 = 2 * a * x + b;
		//	tmp2 = height / (hSliceNum + 1) / sqrtf(1 + powf(tmp1, 2));
		//	y = y + tmp1 * tmp2;
		//	vertex[i * (wSliceNum + 1) + veinIdx].y = y;
		//}

		prepareVAO(vertex, normal, index, &VAO, VBO, &index_size);
	}
};




