#include "utils.h"

std::string readSource(std::string filename) {
	std::ifstream ifs(filename);
	std::stringstream ss;
	ss << ifs.rdbuf();
	return ss.str();
}


void prepareVAO(const std::vector<vec3>& vertex, const std::vector<vec3>& normal, const std::vector<GLuint>& index, GLuint* VAO, GLuint* VBO, GLsizei* length) {
	// 默认VBO[0]为顶点坐标，VBO[1]为法向量，VBO[2]为索引
	glGenVertexArrays(1, VAO);
	glBindVertexArray(*VAO);
	*length = (GLsizei)index.size();

	glGenBuffers(1, VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(vec3), vertex.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	if (!normal.empty()) {
		glGenBuffers(1, VBO + 1);
		glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
		glBufferData(GL_ARRAY_BUFFER, normal.size() * sizeof(vec3), normal.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
		glEnableVertexAttribArray(1);
	}

	glGenBuffers(1, VBO + 2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(GLuint), index.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void updateVertexPosition(GLuint VAO, GLuint VBO, std::vector<vec3> vertex) {
	// 更新VBO[0]数据(顶点坐标)
	glBindVertexArray(VAO);
	glNamedBufferData(VBO, vertex.size() * sizeof(vec3), vertex.data(), GL_DYNAMIC_DRAW);
	glBindVertexArray(0);
}

void loadShader(GLuint shader, std::string source) {
	const char* csource = source.c_str();
	glShaderSource(shader, 1, &csource, NULL);
	glCompileShader(shader);
}


GLuint loadProgramFromFile(std::string vertexSourcePath, std::string fragmentSourcePath) {
	std::string vertexShader = readSource(vertexSourcePath);
	std::string fragmentShader = readSource(fragmentSourcePath);
	return loadProgram(vertexShader, fragmentShader);
}

GLuint loadProgram(std::string vertexSource, std::string fragmentSource) {
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER), fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	loadShader(vertexShader, vertexSource);
	loadShader(fragmentShader, fragmentSource);
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char info[512];
		GLsizei length;
		glGetProgramInfoLog(program, 512, &length, info);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info << std::endl;
		exit(5);
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return program;
}


std::ostream& operator<<(std::ostream& os, const glm::vec3& v) {
	os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return os;
}