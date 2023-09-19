#include "proj.h"
#include "utils.h"

#include <vector>

#pragma once

typedef struct {
	float x, y, z;
}vec3;

typedef struct {
	float x, y, z, w;
}vec4;

class Geometry {
private:
	glm::vec3 position;
	glm::quat rotation;
	glm::vec3 zoom;

	bool autoColor;

	glm::mat4 generateModelMatrix() {
		glm::mat4 transformation(1.0f);
		transformation = glm::translate(transformation, position);
		transformation = transformation * glm::mat4_cast(rotation);
		transformation = glm::scale(transformation, zoom);
		return transformation;
	}
protected:
	GLuint VAO, vbo_pos, vbo_idx;
	GLuint program;
	std::vector<vec3> vertex;
	std::vector<unsigned int> index;
	void prepareShaderProgram(std::string vertexShaderPath, std::string fragmentShaderPath) {
		program = loadProgram(readSource(vertexShaderPath), readSource(fragmentShaderPath));
		updateModelMatrix();
		updateUniformMatrix4fv(program, "modelBuffer", glm::mat4(1.0f));
	};
	virtual void generateVertexAndIndex() = 0;  //���ɶ��������
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
	}  //�������������

public:

	Geometry(glm::vec3 position) : zoom(1.0f), rotation(glm::identity<glm::quat>()), program(0), VAO(0), position(glm::vec3(0.0f)), autoColor(true) {
		// û����ɫ��ʼ��
		prepareShaderProgram("vertexShaderSource.txt", "fragmentShaderSource.txt");

	}
	Geometry(glm::vec3 position, glm::vec4 color) : zoom(1.0f), rotation(glm::identity<glm::quat>()), program(0), VAO(0), position(glm::vec3(0.0f)), autoColor(false) {
		// ����ɫ��ʼ��
		prepareShaderProgram("vertexShaderSource.txt", "customFragmentShaderSource.txt");
	}
	GLuint getProgram() {
		return program;
	}
	void setColor(glm::vec4 color) {
		if (autoColor) {
			autoColor = false;
			glDeleteProgram(program);
			prepareShaderProgram("vertexShaderSource.txt", "customFragmentShaderSource.txt");
		}
		updateUniformVector4fv(program, "CustomColor", color);
	}
	void rotate(float angle, glm::vec3 axis) {
		rotation = glm::angleAxis(angle, axis) * rotation;
		updateModelMatrix();
	}
	void rotateTo(glm::vec3 direction) {
		//��δʵ�ֵĹ��ܣ���ת��ָ������(������̬)��rotate�ǻ��ڵ�ǰ��̬�������ת
	}
	void translate(glm::vec3 dxyz) {
		position = position + dxyz;
		updateModelMatrix();
	}
	void moveTo(glm::vec3 dxyz) {
		position = dxyz;
		updateModelMatrix();
	}
	void applyTransform() {
		glm::mat4 currentModelBuffer;
		glGetUniformfv(program, glGetUniformLocation(program, "modelBuffer"), glm::value_ptr(currentModelBuffer));
		updateUniformMatrix4fv(program, "modelBuffer", generateModelMatrix() * currentModelBuffer);

		position = glm::vec3(0.0f);
		rotation = glm::identity<glm::quat>();
		zoom = glm::vec3(1.0f);
		updateModelMatrix();
	}
	void scale(glm::vec3 xyz) {
		zoom = zoom * xyz;
		updateModelMatrix();
	}
	void updateModelMatrix() {
		updateUniformMatrix4fv(program, "model", generateModelMatrix());
	}
	virtual void draw() {
		glUseProgram(program);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, (GLsizei)index.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glUseProgram(0);
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

		// ����
		for (int i = 0; i <= xSliceNum; i++) {
			for (int j = 0; j <= ySliceNum; j++) {
				xybuf[i * (ySliceNum + 1) + j] = { -xLength / 2 + i * dx, -yLength / 2 + j * dy, -zLength / 2 };
				xybuf[xyNum1 + i * (ySliceNum + 1) + j] = { -xLength / 2 + i * dx, -yLength / 2 + j * dy, zLength / 2 };
			}
		}
		for (int i = 0; i <= zSliceNum; i++) {
			for (int j = 0; j <= ySliceNum; j++) {
				zybuf[i * (ySliceNum + 1) + j] = { -xLength / 2, -yLength / 2 + j * dy, -zLength / 2 + i * dz };
				zybuf[zyNum1 + i * (ySliceNum + 1) + j] = { xLength / 2, -yLength / 2 + j * dy, -zLength / 2 + i * dz };
			}
		}
		for (int i = 0; i <= xSliceNum; i++) {
			for (int j = 0; j <= zSliceNum; j++) {
				xzbuf[i * (zSliceNum + 1) + j] = { -xLength / 2 + i * dx, -yLength / 2, -zLength / 2 + j * dz };
				xzbuf[xzNum1 + i * (zSliceNum + 1) + j] = { -xLength / 2 + i * dx, yLength / 2, -zLength / 2 + j * dz };
			}
		}
		// ��Ԫ����
		int bias = 0;
		GLuint* ptr;
		for (int i = 0; i < xSliceNum; i++) {
			for (int j = 0; j < ySliceNum; j++) {
				ptr = &xyidx[(i * ySliceNum + j) * 12];
				for (int k = 0; k < 2; k++) {
					*(ptr++) = bias + k * xyNum1 + i * (ySliceNum + 1) + j;
					*(ptr++) = bias + k * xyNum1 + (i + 1) * (ySliceNum + 1) + j;
					*(ptr++) = bias + k * xyNum1 + (i + 1) * (ySliceNum + 1) + (j + 1);
					*(ptr++) = bias + k * xyNum1 + i * (ySliceNum + 1) + j;
					*(ptr++) = bias + k * xyNum1 + (i + 1) * (ySliceNum + 1) + (j + 1);
					*(ptr++) = bias + k * xyNum1 + i * (ySliceNum + 1) + (j + 1);
				}
			}
		}
		bias += xyNum1 * 2;
		for (int i = 0; i < zSliceNum; i++) {
			for (int j = 0; j < ySliceNum; j++) {
				ptr = &zyidx[(i * ySliceNum + j) * 12];
				for (int k = 0; k < 2; k++) {
					*(ptr++) = bias + k * zyNum1 + i * (ySliceNum + 1) + j;
					*(ptr++) = bias + k * zyNum1 + (i + 1) * (ySliceNum + 1) + j;
					*(ptr++) = bias + k * zyNum1 + (i + 1) * (ySliceNum + 1) + (j + 1);
					*(ptr++) = bias + k * zyNum1 + i * (ySliceNum + 1) + j;
					*(ptr++) = bias + k * zyNum1 + (i + 1) * (ySliceNum + 1) + (j + 1);
					*(ptr++) = bias + k * zyNum1 + i * (ySliceNum + 1) + (j + 1);
				}
			}
		}
		bias += zyNum1 * 2;
		for (int i = 0; i < xSliceNum; i++) {
			for (int j = 0; j < zSliceNum; j++) {
				ptr = &xzidx[(i * zSliceNum + j) * 12];
				for (int k = 0; k < 2; k++) {
					*(ptr++) = bias + k * xzNum1 + i * (zSliceNum + 1) + j;
					*(ptr++) = bias + k * xzNum1 + (i + 1) * (zSliceNum + 1) + j;
					*(ptr++) = bias + k * xzNum1 + (i + 1) * (zSliceNum + 1) + (j + 1);
					*(ptr++) = bias + k * xzNum1 + i * (zSliceNum + 1) + j;
					*(ptr++) = bias + k * xzNum1 + (i + 1) * (zSliceNum + 1) + (j + 1);
					*(ptr++) = bias + k * xzNum1 + i * (zSliceNum + 1) + (j + 1);
				}
			}
		}
		vertex.clear();
		vertex.insert(vertex.end(), xybuf.begin(), xybuf.end());
		vertex.insert(vertex.end(), zybuf.begin(), zybuf.end());
		vertex.insert(vertex.end(), xzbuf.begin(), xzbuf.end());
		index.clear();
		index.insert(index.end(), xyidx.begin(), xyidx.end());
		index.insert(index.end(), zyidx.begin(), zyidx.end());
		index.insert(index.end(), xzidx.begin(), xzidx.end());
	}
public:
	Cube(float xLength, float yLength, float zLength, int xSliceNum, int ySliceNum, int zSliceNum) :Geometry(glm::vec3(0.0)), xSliceNum(xSliceNum), ySliceNum(ySliceNum), zSliceNum(zSliceNum), xLength(xLength), yLength(yLength), zLength(zLength) {
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

		for (int i = 0; i <= lonSliceNum; i++) {
			for (int j = 0; j <= latSliceNum; j++) {
				float lon = -PI + i * lonStep;
				float lat = -PI / 2 + j * latStep;
				vertex.push_back({ radius * cos(lat) * cos(lon), radius * cos(lat) * sin(lon), radius * sin(lat) });
			}
		}

		for (int i = 0; i < lonSliceNum; i++) {
			for (int j = 0; j < latSliceNum; j++) {
				index.push_back(i * (latSliceNum + 1) + j);
				index.push_back(i * (latSliceNum + 1) + j + 1);
				index.push_back((i + 1) * (latSliceNum + 1) + j + 1);
				index.push_back(i * (latSliceNum + 1) + j);
				index.push_back((i + 1) * (latSliceNum + 1) + j + 1);
				index.push_back((i + 1) * (latSliceNum + 1) + j);
			}
		}
	}
public:
	Sphere(float radius, unsigned int lonSliceNum, unsigned int latSliceNum) :Geometry(glm::vec3(0.0)), radius(radius), lonSliceNum(lonSliceNum), latSliceNum(latSliceNum) {
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
		vertex.clear();

		float lon_tmp, r_tmp, h_tmp;
		//����Բ��
		vec3* ptr = new vec3[2 * rlonNum1]();
		for (int i = 0; i <= lonSliceNum; i++) {
			for (int j = 0; j <= rSliceNum; j++) {
				lon_tmp = -PI + i * lonStep;
				r_tmp = j * rStep;
				ptr[i * (rSliceNum + 1) + j] = { r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), -height / 2 };
				ptr[rlonNum1 + i * (rSliceNum + 1) + j] = { r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), height / 2 };
			}
		}
		vertex.insert(vertex.end(), ptr, ptr + 2 * rlonNum1);
		delete[] ptr;

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
		//����
		ptr = new vec3[hlonNum1]();
		for (int i = 0; i <= hSliceNum; i++) {
			for (int j = 0; j <= lonSliceNum; j++) {
				h_tmp = -height / 2 + i * hStep;
				lon_tmp = -PI + j * lonStep;
				ptr[i * (lonSliceNum + 1) + j] = { radius * cos(lon_tmp),radius * sin(lon_tmp) , h_tmp };
			}
		}
		vertex.insert(vertex.end(), ptr, ptr + hlonNum1);
		delete[] ptr;
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
	Cylinder(float radius, float height, int rSliceNum, int hSliceNum, int lonSliceNum) :Geometry(glm::vec3(0.0)), radius(radius), height(height), rSliceNum(rSliceNum), hSliceNum(hSliceNum), lonSliceNum(lonSliceNum) {
		generateVertexAndIndex();
		prepareVAO();
	}
};

class Cone : public Geometry { // Բ׶
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
		vertex.clear();

		float lon_tmp, r_tmp, h_tmp;
		//��Բ��
		vec3* ptr = new vec3[rlonNum1]();
		for (int i = 0; i <= lonSliceNum; i++) {
			for (int j = 0; j <= rSliceNum; j++) {
				lon_tmp = -PI + i * lonStep;
				r_tmp = j * rStep;
				ptr[i * (rSliceNum + 1) + j] = { r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp), -height / 2 };
			}
		}
		vertex.insert(vertex.end(), ptr, ptr + rlonNum1);
		delete[] ptr;

		for (int i = 0; i < lonSliceNum; i++) {
			for (int j = 0; j < rSliceNum; j++) {
				index.push_back(base + i * (rSliceNum + 1) + j);
				index.push_back(base + i * (rSliceNum + 1) + j + 1);
				index.push_back(base + (i + 1) * (rSliceNum + 1) + j + 1);
				index.push_back(base + i * (rSliceNum + 1) + j);
				index.push_back(base + (i + 1) * (rSliceNum + 1) + j + 1);
				index.push_back(base + (i + 1) * (rSliceNum + 1) + j);
			}
		}

		base += rlonNum1;
		//����
		ptr = new vec3[hlonNum1]();
		for (int i = 0; i <= hSliceNum; i++) {
			for (int j = 0; j <= lonSliceNum; j++) {
				h_tmp = -height / 2 + i * hStep;
				lon_tmp = -PI + j * lonStep;
				r_tmp = radius * (1 - i * hStep / height);
				ptr[i * (lonSliceNum + 1) + j] = { r_tmp * cos(lon_tmp),r_tmp * sin(lon_tmp) , h_tmp };
			}
		}
		vertex.insert(vertex.end(), ptr, ptr + hlonNum1);
		delete[] ptr;
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
	Cone(float radius, float height, int rSliceNum, int hSliceNum, int lonSliceNum) :Geometry(glm::vec3(0.0)), radius(radius), height(height), rSliceNum(rSliceNum), hSliceNum(hSliceNum), lonSliceNum(lonSliceNum) {
		generateVertexAndIndex();
		prepareVAO();
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
	Arrow(glm::vec3 _begin, glm::vec3 _end, float width, glm::vec4 arrowColor, glm::vec4 bodyColor) : _begin(_begin), _end(_end), width(width), arrowColor(arrowColor), bodyColor(bodyColor) {
		length = glm::length(_end - _begin);
		arrow = new Cone(arrowRadiusRatio * width / 2.0f, arrowLengthRatio * length, 3, 4, 18);
		body = new Cylinder(width / 2.0f, (1 - arrowLengthRatio) * length, 2, (int)(length * 10), 18);

		arrow->setColor(arrowColor);
		body->setColor(bodyColor);
		// �������
		arrow->rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		body->rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		arrow->translate(glm::vec3(0, (1 - arrowLengthRatio) * length / 2.0f, 0));
		body->translate(glm::vec3(0, -arrowLengthRatio / 2.0f * length, 0));
		// Ӧ�ñ任
		arrow->applyTransform();
		body->applyTransform();

		// �任��ָ��λ��
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

		arrow->rotate(rotate_angle, rotate_axis);
		body->rotate(rotate_angle, rotate_axis);
		arrow->moveTo((_end + _begin) / 2.0f);
		body->moveTo((_end + _begin) / 2.0f);
	}
	std::vector<GLuint> getProgramList() {
		std::vector<GLuint> programs;
		programs.push_back(body->getProgram());
		programs.push_back(arrow->getProgram());
		return programs;
	}
	void draw() {
		arrow->draw();
		body->draw();
	}
};
