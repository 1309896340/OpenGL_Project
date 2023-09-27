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
	std::vector<Line> lines;	// 线条缓冲区
	GLuint vbo_line;
	Shader* shader;
}LineStructure;


class Drawable {
public:
	virtual void draw() = 0;
};

class Geometry :public Drawable {
private:
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 _scale;

	glm::mat4 modelBuffer;

protected:
	GLuint VAO;
	std::vector<vec3> vertex;
	std::vector<vec3> normal;
	std::vector<unsigned int> index;

	Shader* shader;
	glm::vec4 color;
	bool autoColor;

public:

	Geometry(glm::vec3 position, Shader* shader) :autoColor(true), color(glm::vec4(0.0f)), shader(shader), modelBuffer(glm::mat4(1.0f)),
		_scale(1.0f), rotation(glm::identity<glm::quat>()), VAO(0), position(glm::vec3(0.0f)) {
		vertex.clear();
		index.clear();
		normal.clear();
	}
	Geometry(Shader* shader) :position(glm::vec3(0.0f)), shader(shader), modelBuffer(glm::mat4(1.0f)),
		_scale(1.0f), rotation(glm::identity<glm::quat>()), VAO(0), autoColor(true), color(glm::vec4(0.0f)) {
		vertex.clear();
		index.clear();
		normal.clear();
	}
	Geometry(const std::vector<vec3>& vertex, const std::vector<vec3>& normal, const std::vector<GLuint>& index, Shader* shader) :position(glm::vec3(0.0f)), shader(shader), modelBuffer(glm::mat4(1.0f)),
		_scale(1.0f), rotation(glm::identity<glm::quat>()), VAO(0), vertex(vertex), normal(normal), index(index), autoColor(true), color(glm::vec4(0.0f)) {
		VAO = prepareVAO(vertex, normal, index);
	}

	void setColor(glm::vec4 color) {
		autoColor = false;
		this->color = color;
	}
	void setColor(bool isAuto) {
		if (isAuto)
			autoColor = true;
	}
	void rotate(float angle, glm::vec3 axis) {
		rotation = glm::angleAxis(angle, axis) * rotation;
	}
	void rotateTo(glm::vec3 direction) {
		//尚未实现的功能：旋转到指定方向(绝对姿态)，rotate是基于当前姿态的相对旋转
	}
	void translate(glm::vec3 dxyz) {
		position = position + dxyz;
	}
	void moveTo(glm::vec3 dxyz) {
		position = dxyz;
	}
	void applyTransform() {
		modelBuffer = getModelMatrix() * modelBuffer;

		position = glm::vec3(0.0f);
		rotation = glm::identity<glm::quat>();
		_scale = glm::vec3(1.0f);
	}
	void scale(glm::vec3 xyz) {
		_scale *= xyz;
	}
	glm::mat4 getModelMatrix() {
		glm::mat4 transformation(1.0f);
		transformation = glm::translate(transformation, position);
		transformation = transformation * glm::mat4_cast(rotation);
		transformation = glm::scale(transformation, _scale);
		return transformation;
	}
	virtual void draw() {
		Shader& sd = *shader;
		sd.use();
		if (autoColor) {
			sd["isAuto"] = true;
		}
		else {
			sd["isAuto"] = false;
			sd["ncolor"] = color;
		}
		sd["model"] = getModelMatrix();
		sd["modelBuffer"] = modelBuffer;
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)index.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
};

class Cube : public Geometry {
private:
	int xSliceNum, ySliceNum, zSliceNum;
	float xLength, yLength, zLength;
public:
	Cube(float xLength, float yLength, float zLength, int xSliceNum, int ySliceNum, int zSliceNum, Shader* shader) :
		Geometry(glm::vec3(0.0), shader), xSliceNum(xSliceNum), ySliceNum(ySliceNum), zSliceNum(zSliceNum),
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

		vertex.resize(2 * (xyNum1 + zyNum1 + xzNum1), { 0.0f,0.0f,0.0f });
		index.resize(2 * (xyNum + zyNum + xzNum) * 6, 0);
		normal.resize(2 * (xyNum1 + zyNum1 + xzNum1), { 0.0f,0.0f,0.0f });

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
		VAO = prepareVAO(vertex, normal, index);
	}
};

class Sphere : public Geometry {
private:
	float radius;
	int lonSliceNum;
	int latSliceNum;

public:
	Sphere(float radius, unsigned int lonSliceNum, unsigned int latSliceNum, Shader* shader) :Geometry(glm::vec3(0.0), shader), radius(radius), lonSliceNum(lonSliceNum), latSliceNum(latSliceNum) {
		float lonStep = 2 * PI / lonSliceNum;
		float latStep = PI / latSliceNum;

		int lonlatNum = lonSliceNum * latSliceNum;
		int lonlatNum1 = (latSliceNum + 1) * (lonSliceNum + 1);

		vertex.resize(lonlatNum1, { 0.0f,0.0f,0.0f });
		index.resize(lonlatNum * 6, 0);
		normal.resize(lonlatNum1, { 0.0f,0.0f,0.0f });

		for (int i = 0; i <= lonSliceNum; i++) {
			for (int j = 0; j <= latSliceNum; j++) {
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
		VAO = prepareVAO(vertex, normal, index);
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
	Cylinder(float radius, float height, int rSliceNum, int hSliceNum, int lonSliceNum, Shader* shader) :Geometry(glm::vec3(0.0), shader), radius(radius), height(height), rSliceNum(rSliceNum), hSliceNum(hSliceNum), lonSliceNum(lonSliceNum) {
		float rStep = radius / rSliceNum;
		float hStep = height / hSliceNum;
		float lonStep = 2 * PI / lonSliceNum;

		int rlonNum = rSliceNum * lonSliceNum;
		int hlonNum = hSliceNum * lonSliceNum;
		int rlonNum1 = (rSliceNum + 1) * (lonSliceNum + 1);
		int hlonNum1 = (hSliceNum + 1) * (lonSliceNum + 1);

		float lon_tmp, r_tmp, h_tmp;

		int baseVert = 0, baseIdx = 0;
		vertex.resize(2 * rlonNum1 + hlonNum1, { 0.0f,0.0f,0.0f });
		index.resize((2 * rlonNum + hlonNum) * 6, 0);
		normal.resize(2 * rlonNum1 + hlonNum1, { 0.0f,0.0f,0.0f });
		for (int i = 0; i <= lonSliceNum; i++) {
			for (int j = 0; j <= rSliceNum; j++) {
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
		for (int i = 0; i <= hSliceNum; i++) {
			for (int j = 0; j <= lonSliceNum; j++) {
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
		VAO = prepareVAO(vertex, normal, index);
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
	Cone(float radius, float height, int rSliceNum, int hSliceNum, int lonSliceNum, Shader* shader) :Geometry(glm::vec3(0.0), shader), radius(radius), height(height), rSliceNum(rSliceNum), hSliceNum(hSliceNum), lonSliceNum(lonSliceNum) {
		float rStep = radius / rSliceNum;
		float hStep = height / hSliceNum;
		float lonStep = 2 * PI / lonSliceNum;

		int rlonNum = rSliceNum * lonSliceNum;
		int hlonNum = hSliceNum * lonSliceNum;
		int rlonNum1 = (rSliceNum + 1) * (lonSliceNum + 1);
		int hlonNum1 = (hSliceNum + 1) * (lonSliceNum + 1);

		float lon_tmp, r_tmp, h_tmp;

		int baseVert = 0, baseIdx = 0;
		vertex.resize(rlonNum1 + hlonNum1, { 0.0f,0.0f,0.0f });
		index.resize((rlonNum + hlonNum) * 6, 0);
		normal.resize(rlonNum1 + hlonNum1, { 0.0f,0.0f,0.0f });

		for (int i = 0; i <= lonSliceNum; i++) {
			for (int j = 0; j <= rSliceNum; j++) {
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
		for (int i = 0; i <= hSliceNum; i++) {
			for (int j = 0; j <= lonSliceNum; j++) {
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
		VAO = prepareVAO(vertex, normal, index);
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
	Surface(float xStart, float xEnd, float yStart, float yEnd, zFunc func, zGrad grad, unsigned int xSliceNum, unsigned int ySliceNum, Shader* shader) :
		Geometry(glm::vec3(0.0), shader), xStart(xStart), xEnd(xEnd), yStart(yStart), yEnd(yEnd), func(func), grad(grad), xSliceNum(xSliceNum), ySliceNum(ySliceNum) {
		float dx = (xEnd - xStart) / xSliceNum;
		float dy = (yEnd - yStart) / ySliceNum;
		float x, y, z;
		vec3 _grad;

		int lonlatNum = xSliceNum * ySliceNum;
		int lonlatNum1 = (xSliceNum + 1) * (ySliceNum + 1);

		vertex.resize(lonlatNum1, { 0.0f,0.0f,0.0f });
		index.resize(lonlatNum * 6, 0);
		normal.resize(lonlatNum1, { 0.0f,0.0f,0.0f });

		for (int i = 0; i <= xSliceNum; i++) {
			for (int j = 0; j <= ySliceNum; j++) {
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
		VAO = prepareVAO(vertex, normal, index);
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

class Arrow :public Drawable {		// 比较粗暴的组合体实现
private:
	glm::vec3 _begin;
	glm::vec3 _end;
	float width;
	float length;
	glm::vec4 arrowColor;
	glm::vec4 bodyColor;

	const float arrowLengthRatio = 0.2f;
	const float arrowRadiusRatio = 2.5f;

	Geometry* arrow, * body;
public:
	Arrow(glm::vec3 _begin, glm::vec3 _end, float width, glm::vec4 arrowColor, glm::vec4 bodyColor, Shader* shader) : _begin(_begin), _end(_end), width(width), arrowColor(arrowColor), bodyColor(bodyColor) {
		length = glm::length(_end - _begin);
		arrow = new Cone(arrowRadiusRatio * width / 2.0f, arrowLengthRatio * length, 3, 4, 18, shader);
		body = new Cylinder(width / 2.0f, (1 - arrowLengthRatio) * length, 2, (int)(length * 10), 18, shader);

		arrow->setColor(arrowColor);
		body->setColor(bodyColor);
		// 进行组合
		arrow->rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		body->rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		arrow->translate(glm::vec3(0, (1 - arrowLengthRatio) * length / 2.0f, 0));
		body->translate(glm::vec3(0, -arrowLengthRatio / 2.0f * length, 0));
		// 应用变换
		arrow->applyTransform();
		body->applyTransform();

		// 变换到指定位置
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

		arrow->rotate(rotate_angle, rotate_axis);
		body->rotate(rotate_angle, rotate_axis);
		arrow->moveTo((_end + _begin) / 2.0f);
		body->moveTo((_end + _begin) / 2.0f);
	}
	void draw() {
		arrow->draw();
		body->draw();
	}
};

class Axis :public Drawable {
private:
	Arrow* axis_x, * axis_y, * axis_z;
	Shader* shader;
public:
	Axis(Shader* shader) :shader(shader) {
		axis_x = new Arrow(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.06f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), shader);
		axis_y = new Arrow(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.f, 0.0f), 0.06f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), shader);
		axis_z = new Arrow(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.06f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), shader);
	}

	void draw() {
		axis_x->draw();
		axis_y->draw();
		axis_z->draw();
	}
};


class Leaf :public Geometry {
private:
	float width, height, theta;
	unsigned int wSliceNum, hSliceNum;

	const unsigned int u_degree = 3, v_degree = 2;		// u为长度分割，v为宽度分割
	const float k = 10.0f, SLAngle = PI / 6.0f;			// k为叶片弯曲程度，SLAngle为叶片弯曲角度

	float wFunc(float h) {
		// 生成叶片宽度关于长度的函数
		return (-1.0f / powf(height, 2) * powf(h, 2) + 2 * theta / height * h + (1 - 2 * theta)) * width / powf(1 - theta, 2);
	}
	float veinFunc(float x) {
		// 生成叶脉关于长度的函数
		return -k * width / height * x * x + tanf(PI / 2 - SLAngle) * x;
	}
public:
	Leaf(float width, float height, unsigned int wSliceNum, unsigned int hSliceNum, Shader* shader) :
		Geometry(shader), width(width), height(height), wSliceNum(wSliceNum), hSliceNum(hSliceNum), theta(0.35f) {

		if (wSliceNum < v_degree + 1 || hSliceNum < u_degree + 1) {
			std::cout << "节点分割数太少" << std::endl;
			exit(0);
		}

		tinynurbs::array2<glm::vec3> ctrl_pts(hSliceNum, wSliceNum, glm::vec3(0.0f));
		tinynurbs::array2<float> weight(hSliceNum, wSliceNum, 1.0f);

		// 生成节点
		std::vector<float> knots_u(hSliceNum + u_degree + 1, 0), knots_v(wSliceNum + v_degree + 1, 0);
		for (int i = 0; i <= u_degree; i++) {
			knots_u[i] = 0.0f;
			knots_u[hSliceNum + u_degree - i] = 1.0f;
		}
		for (int i = u_degree + 1; i < hSliceNum; i++) {
			knots_u[i] = (float)(i - u_degree) / (hSliceNum - u_degree);
		}
		for (int i = 0; i <= v_degree; i++) {
			knots_v[i] = 0.0f;
			knots_v[wSliceNum + v_degree - i] = 1.0f;
		}
		for (int i = v_degree + 1; i < wSliceNum; i++) {
			knots_v[i] = (float)(i - v_degree) / (wSliceNum - v_degree);
		}
		// 生成控制点（叶鞘、叶两边、叶脉）
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


		// 先不使用NURBS曲面，直接连线
		int hwNum = hSliceNum * wSliceNum, hwNum1 = (hSliceNum + 1) * (wSliceNum + 1);
		vertex.resize(hwNum1, { 0 });
		normal.resize(hwNum1, { 0 });	// 没有生成法向量
		index.resize(hwNum * 6, 0);
		float x, y;
		for (int i = 0; i <= hSliceNum; i++) {
			x = (float)i / hSliceNum * height;
			for (int j = 0; j <= wSliceNum; j++) {
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
		// 目前没有处理好叶脉方程，也没有使用NURBS曲面描述，暂时不知道如何处理
		// Leaf对象应当具有几个更新函数，应及时更新VBO的值
		// 1.更新叶脉弯曲度的k
		// 2. 茎叶夹角的theta

		//assert(wSliceNum % 2 == 0);	// 假定wSliceNum必须为偶数
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

		VAO = prepareVAO(vertex, normal, index);
	}
};
