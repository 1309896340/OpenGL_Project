#include "proj.h"
#include "utils.h"

#include <vector>

typedef struct {
	float x, y, z;
}vec3;

typedef struct {
	float x, y, z, w;
}vec4;

class Geometry {
private:
	glm::mat4 position;
	glm::mat4 rotation;
	float scale;

	glm::mat4 generateModelMatrix() {
		return position * rotation * scale;
	}
protected:
	GLuint VAO;
	GLuint program;
	std::vector<vec3> vertex;
	std::vector<unsigned int> index;
	virtual void generateVertexAndIndex() = 0;  //生成顶点和索引
	virtual void prepareVAO() = 0;  //解析顶点和索引
	virtual void prepareShaderProgram() = 0;  //准备着色器程序
public:

	Geometry(glm::vec3 position) : scale(1.0), rotation(glm::mat4(1.0f)), program(0), VAO(0) {
		this->position = glm::translate(glm::mat4(1.0f), position);
	}

	GLuint getProgram() {
		return program;
	}
	void rotate(float angle, glm::vec3 axis) {
		rotation = glm::rotate(rotation, angle, axis);
		glUseProgram(program);
		updateUniformMatrix4fv(program, "model", generateModelMatrix());
		glUseProgram(0);
	}
	void translate(glm::vec3 dxyz) {
		position = glm::translate(position, dxyz);
		glUseProgram(program);
		updateUniformMatrix4fv(program, "model", generateModelMatrix());
		glUseProgram(0);
	}


	virtual void draw() = 0;	//绘制
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

		// 顶点
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
		// 面元索引
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
	virtual void prepareVAO() {

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		GLuint vbo_pos;
		glGenBuffers(1, &vbo_pos);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
		glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(vec3), vertex.data(), GL_STATIC_DRAW);

		GLuint vbo_idx;
		glGenBuffers(1, &vbo_idx);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_idx);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(GLuint), index.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	virtual void prepareShaderProgram() {
		std::string vertexShaderSource = readSource("vertexShaderSource.txt");
		std::string fragmentShaderSource = readSource("fragmentShaderSource.txt");
		program = loadProgram(vertexShaderSource, fragmentShaderSource);
	}
public:
	Cube(float xLength, float yLength, float zLength, int xSliceNum, int ySliceNum, int zSliceNum) :Geometry(glm::vec3(0.0)), xSliceNum(xSliceNum), ySliceNum(ySliceNum), zSliceNum(zSliceNum), xLength(xLength), yLength(yLength), zLength(zLength) {
		generateVertexAndIndex();
		prepareShaderProgram();
		prepareVAO();
	}

	virtual void draw() {
		glUseProgram(program);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, index.size(), GL_UNSIGNED_INT, 0);
		//glDrawArrays(GL_TRIANGLES, 0, vertex.size());
		glBindVertexArray(0);
		glUseProgram(0);
	}
};
