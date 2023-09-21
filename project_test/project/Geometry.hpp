#include "proj.h"
#include "utils.h"
#include "Shader.hpp"
#pragma once

class Geometry {
private:
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 _scale;

	Shader* shader;

	glm::vec4 color;
	bool autoColor;

	glm::mat4 modelBuffer;

protected:
	GLuint VAO, vbo_pos, vbo_idx;
	std::vector<vec3> vertex;
	std::vector<unsigned int> index;

	virtual void generateVertexAndIndex() = 0;  //生成顶点和索引
	virtual void prepareVAO() {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		glGenBuffers(1, &vbo_pos);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
		glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(vec3), vertex.data(), GL_DYNAMIC_DRAW);

		glGenBuffers(1, &vbo_idx);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_idx);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(GLuint), index.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}  //解析顶点和索引

public:

	Geometry(glm::vec3 position, Shader* shader) :autoColor(true), shader(shader), modelBuffer(glm::mat4(1.0f)), _scale(1.0f), rotation(glm::identity<glm::quat>()), VAO(0), position(glm::vec3(0.0f)) {
		// 没有颜色初始化
	}
	void setColor(glm::vec4 color) {
		autoColor = false;
		this->color = color;
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
		shader->use();
		if (autoColor) {
			shader->setBool("isCustom",false);
		}else{
			shader->setBool("isCustom",true);
			shader->setVec4("CustomColor", color);
		}
		shader->setMat4("model", getModelMatrix());
		shader->setMat4("modelBuffer", modelBuffer);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)index.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		shader->close();
	}
};

class Cube : public Geometry {
private:
	int xSliceNum, ySliceNum, zSliceNum;
	float xLength, yLength, zLength;

protected:
	virtual void generateVertexAndIndex() {

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

		std::vector<vec3> xybuf(2 * (xSliceNum + 1) * (ySliceNum + 1));
		std::vector<vec3> zybuf(2 * (zSliceNum + 1) * (ySliceNum + 1));
		std::vector<vec3> xzbuf(2 * (xSliceNum + 1) * (zSliceNum + 1));

		std::vector<GLuint> xyidx(2 * 6 * xSliceNum * ySliceNum);
		std::vector<GLuint> zyidx(2 * 6 * zSliceNum * ySliceNum);
		std::vector<GLuint> xzidx(2 * 6 * xSliceNum * zSliceNum);

		vertex.resize(2 * (xyNum1 + zyNum1 + xzNum1), { 0.0f,0.0f,0.0f });
		index.clear();

		int base = 0;
		for (int i = 0; i <= xSliceNum; i++) {
			for (int j = 0; j <= ySliceNum; j++) {
				vertex[base + i * (ySliceNum + 1) + j] = { -xLength / 2 + i * dx, -yLength / 2 + j * dy, -zLength / 2 };
				vertex[base + xyNum1 + i * (ySliceNum + 1) + j] = { -xLength / 2 + i * dx, -yLength / 2 + j * dy, zLength / 2 };
			}
		}
		for (int k = 0; k < 2; k++) {
			for (int i = 0; i < xSliceNum; i++) {
				for (int j = 0; j < ySliceNum; j++) {
					index.push_back(base + k * xyNum1 + i * (ySliceNum + 1) + j);
					index.push_back(base + k * xyNum1 + i * (ySliceNum + 1) + j + 1);
					index.push_back(base + k * xyNum1 + (i + 1) * (ySliceNum + 1) + j + 1);
					index.push_back(base + k * xyNum1 + i * (ySliceNum + 1) + j);
					index.push_back(base + k * xyNum1 + (i + 1) * (ySliceNum + 1) + j + 1);
					index.push_back(base + k * xyNum1 + (i + 1) * (ySliceNum + 1) + j);
				}
			}
		}
		base += 2 * xyNum1;
		for (int i = 0; i <= zSliceNum; i++) {
			for (int j = 0; j <= ySliceNum; j++) {
				vertex[base + i * (ySliceNum + 1) + j] = { -xLength / 2, -yLength / 2 + j * dy, -zLength / 2 + i * dz };
				vertex[base + zyNum1 + i * (ySliceNum + 1) + j] = { xLength / 2, -yLength / 2 + j * dy, -zLength / 2 + i * dz };
			}
		}
		for (int k = 0; k < 2; k++) {
			for (int i = 0; i < zSliceNum; i++) {
				for (int j = 0; j < ySliceNum; j++) {
					index.push_back(base + k * zyNum1 + i * (ySliceNum + 1) + j);
					index.push_back(base + k * zyNum1 + i * (ySliceNum + 1) + j + 1);
					index.push_back(base + k * zyNum1 + (i + 1) * (ySliceNum + 1) + j + 1);
					index.push_back(base + k * zyNum1 + i * (ySliceNum + 1) + j);
					index.push_back(base + k * zyNum1 + (i + 1) * (ySliceNum + 1) + j + 1);
					index.push_back(base + k * zyNum1 + (i + 1) * (ySliceNum + 1) + j);
				}
			}
		}
		base += 2 * zyNum1;
		for (int i = 0; i <= xSliceNum; i++) {
			for (int j = 0; j <= zSliceNum; j++) {
				vertex[base + i * (zSliceNum + 1) + j] = { -xLength / 2 + i * dx, -yLength / 2, -zLength / 2 + j * dz };
				vertex[base + xzNum1 + i * (zSliceNum + 1) + j] = { -xLength / 2 + i * dx, yLength / 2, -zLength / 2 + j * dz };
			}
		}
		for (int k = 0; k < 2; k++) {
			for (int i = 0; i < xSliceNum; i++) {
				for (int j = 0; j < zSliceNum; j++) {
					index.push_back(base + k * xzNum1 + i * (zSliceNum + 1) + j);
					index.push_back(base + k * xzNum1 + i * (zSliceNum + 1) + j + 1);
					index.push_back(base + k * xzNum1 + (i + 1) * (zSliceNum + 1) + j + 1);
					index.push_back(base + k * xzNum1 + i * (zSliceNum + 1) + j);
					index.push_back(base + k * xzNum1 + (i + 1) * (zSliceNum + 1) + j + 1);
					index.push_back(base + k * xzNum1 + (i + 1) * (zSliceNum + 1) + j);
				}
			}
		}
	}
public:
	Cube(float xLength, float yLength, float zLength, int xSliceNum, int ySliceNum, int zSliceNum, Shader* shader) :Geometry(glm::vec3(0.0), shader), xSliceNum(xSliceNum), ySliceNum(ySliceNum), zSliceNum(zSliceNum), xLength(xLength), yLength(yLength), zLength(zLength) {
		generateVertexAndIndex();
		prepareVAO();
	}
};

class Sphere : public Geometry {
private:
	float radius;
	int lonSliceNum;
	int latSliceNum;

protected:
	virtual void generateVertexAndIndex() {
		float lonStep = 2 * PI / lonSliceNum;
		float latStep = PI / latSliceNum;

		vertex.clear();
		index.clear();

		for (int i = 0; i <= lonSliceNum; i++) {
			for (int j = 0; j <= latSliceNum; j++) {
				float lon = -PI + i * lonStep;
				float lat = -PI / 2 + j * latStep;
				vertex.push_back({ radius * cos(lat) * cos(lon), radius * cos(lat) * sin(lon), radius * sin(lat) });
				if (i < lonSliceNum && j < latSliceNum) {
					index.push_back(i * (latSliceNum + 1) + j);
					index.push_back(i * (latSliceNum + 1) + j + 1);
					index.push_back((i + 1) * (latSliceNum + 1) + j + 1);
					index.push_back(i * (latSliceNum + 1) + j);
					index.push_back((i + 1) * (latSliceNum + 1) + j + 1);
					index.push_back((i + 1) * (latSliceNum + 1) + j);
				}
			}
		}
	}
public:
	Sphere(float radius, unsigned int lonSliceNum, unsigned int latSliceNum, Shader* shader) :Geometry(glm::vec3(0.0), shader), radius(radius), lonSliceNum(lonSliceNum), latSliceNum(latSliceNum) {
		generateVertexAndIndex();
		prepareVAO();
	}
};

class Cylinder : public Geometry {
private:
	float radius;
	float height;
	int rSliceNum;
	int hSliceNum;
	int lonSliceNum;
protected:
	void generateVertexAndIndex() {
		float rStep = radius / rSliceNum;
		float hStep = height / hSliceNum;
		float lonStep = 2 * PI / lonSliceNum;

		int rlonNum = rSliceNum * lonSliceNum;
		int hlonNum = hSliceNum * lonSliceNum;
		int rlonNum1 = (rSliceNum + 1) * (lonSliceNum + 1);
		int hlonNum1 = (hSliceNum + 1) * (lonSliceNum + 1);

		int base = 0;
		vertex.resize(2 * rlonNum1 + hlonNum1, { 0.0f,0.0f,0.0f });
		index.clear();

		float lon_tmp, r_tmp, h_tmp;
		for (int i = 0; i <= lonSliceNum; i++) {
			for (int j = 0; j <= rSliceNum; j++) {
				lon_tmp = -PI + i * lonStep;
				r_tmp = j * rStep;
				vertex[base + i * (rSliceNum + 1) + j] = { r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), -height / 2 };
				vertex[base + rlonNum1 + i * (rSliceNum + 1) + j] = { r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), height / 2 };
			}
		}
		for (int k = 0; k < 2; k++) {
			for (int i = 0; i < lonSliceNum; i++) {
				for (int j = 0; j < rSliceNum; j++) {
					index.push_back(base + k * rlonNum1 + i * (rSliceNum + 1) + j);
					index.push_back(base + k * rlonNum1 + i * (rSliceNum + 1) + j + 1);
					index.push_back(base + k * rlonNum1 + (i + 1) * (rSliceNum + 1) + j + 1);
					index.push_back(base + k * rlonNum1 + i * (rSliceNum + 1) + j);
					index.push_back(base + k * rlonNum1 + (i + 1) * (rSliceNum + 1) + j + 1);
					index.push_back(base + k * rlonNum1 + (i + 1) * (rSliceNum + 1) + j);
				}
			}
		}
		base += 2 * rlonNum1;
		for (int i = 0; i <= hSliceNum; i++) {
			for (int j = 0; j <= lonSliceNum; j++) {
				h_tmp = -height / 2 + i * hStep;
				lon_tmp = -PI + j * lonStep;
				vertex[base + i * (lonSliceNum + 1) + j] = { radius * cos(lon_tmp),radius * sin(lon_tmp) , h_tmp };
			}
		}
		for (int i = 0; i < hSliceNum; i++) {
			for (int j = 0; j < lonSliceNum; j++) {
				index.push_back(base + i * (lonSliceNum + 1) + j);
				index.push_back(base + i * (lonSliceNum + 1) + j + 1);
				index.push_back(base + (i + 1) * (lonSliceNum + 1) + j + 1);
				index.push_back(base + i * (lonSliceNum + 1) + j);
				index.push_back(base + (i + 1) * (lonSliceNum + 1) + j + 1);
				index.push_back(base + (i + 1) * (lonSliceNum + 1) + j);
			}
		}
	}
public:
	Cylinder(float radius, float height, int rSliceNum, int hSliceNum, int lonSliceNum, Shader* shader) :Geometry(glm::vec3(0.0), shader), radius(radius), height(height), rSliceNum(rSliceNum), hSliceNum(hSliceNum), lonSliceNum(lonSliceNum) {
		generateVertexAndIndex();
		prepareVAO();
	}
};

class Cone : public Geometry { // 圆锥
private:
	float radius;
	float height;

	int rSliceNum;
	int hSliceNum;
	int lonSliceNum;
protected:
	void generateVertexAndIndex() {
		float rStep = radius / rSliceNum;
		float hStep = height / hSliceNum;
		float lonStep = 2 * PI / lonSliceNum;

		int rlonNum = rSliceNum * lonSliceNum;
		int hlonNum = hSliceNum * lonSliceNum;
		int rlonNum1 = (rSliceNum + 1) * (lonSliceNum + 1);
		int hlonNum1 = (hSliceNum + 1) * (lonSliceNum + 1);

		vertex.resize(rlonNum1 + hlonNum1, { 0.0f,0.0f,0.0f });
		index.clear();

		float lon_tmp, r_tmp, h_tmp;

		int base = 0;
		for (int i = 0; i <= lonSliceNum; i++) {
			for (int j = 0; j <= rSliceNum; j++) {
				lon_tmp = -PI + i * lonStep;
				r_tmp = j * rStep;
				vertex[base + i * (rSliceNum + 1) + j] = { r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), -height / 2 };
				if (i < lonSliceNum && j < rSliceNum) {
					index.push_back(base + i * (rSliceNum + 1) + j);
					index.push_back(base + i * (rSliceNum + 1) + j + 1);
					index.push_back(base + (i + 1) * (rSliceNum + 1) + j + 1);
					index.push_back(base + i * (rSliceNum + 1) + j);
					index.push_back(base + (i + 1) * (rSliceNum + 1) + j + 1);
					index.push_back(base + (i + 1) * (rSliceNum + 1) + j);
				}
			}
		}
		base += rlonNum1;
		for (int i = 0; i <= hSliceNum; i++) {
			for (int j = 0; j <= lonSliceNum; j++) {
				h_tmp = -height / 2 + i * hStep;
				lon_tmp = -PI + j * lonStep;
				r_tmp = radius * (1 - i * hStep / height);
				vertex[base + i * (lonSliceNum + 1) + j] = { r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp) , h_tmp };
				if (i < hSliceNum && j < lonSliceNum) {
					index.push_back(base + i * (lonSliceNum + 1) + j);
					index.push_back(base + i * (lonSliceNum + 1) + j + 1);
					index.push_back(base + (i + 1) * (lonSliceNum + 1) + j + 1);
					index.push_back(base + i * (lonSliceNum + 1) + j);
					index.push_back(base + (i + 1) * (lonSliceNum + 1) + j + 1);
					index.push_back(base + (i + 1) * (lonSliceNum + 1) + j);
				}
			}
		}
	}
public:
	Cone(float radius, float height, int rSliceNum, int hSliceNum, int lonSliceNum, Shader* shader) :Geometry(glm::vec3(0.0), shader), radius(radius), height(height), rSliceNum(rSliceNum), hSliceNum(hSliceNum), lonSliceNum(lonSliceNum) {
		generateVertexAndIndex();
		prepareVAO();
	}
};


class Line {
private:
	glm::vec3 _begin;
	glm::vec3 _end;
	float width;
	glm::vec3 color;
protected:
	GLuint program;
	GLuint vao;
public:
	Line(glm::vec3 begin, glm::vec3 end, float width, glm::vec3 color) :_begin(begin), _end(end), width(width), color(color) {
		program = loadProgramFromFile("Line.vert", "fragmentShaderSource.txt");

		GLfloat vertices[] = {
			_begin.x,_begin.y,_begin.z,
			_end.x,_end.y,_end.z
		};

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		GLuint vbo;
		glGenBuffers(1, &vbo);
		glNamedBufferData(vbo, sizeof(GLfloat) * 6, vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, NULL, NULL);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);

		// 初始化颜色的uniform矩阵
		updateUniformVector4fv(program, "CustomColor", glm::vec4(color, 1.0f));
	}

	GLuint getProgram() {
		return program;
	}
	void draw() {
		glUseProgram(program);
		glBindVertexArray(vao);
		glDrawArrays(GL_LINES, 0, 2);
		glBindVertexArray(0);
		glUseProgram(0);
	}
};

class Arrow {
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
