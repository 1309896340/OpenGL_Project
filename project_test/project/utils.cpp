#include "utils.h"

std::string readSource(std::string filename) {
	std::ifstream ifs(filename);
	std::stringstream ss;
	ss << ifs.rdbuf();
	return ss.str();
}

//void updateUniformMatrix4fv(GLuint program, const std::string& varname, const glm::mat4& m) {
//	GLint location = glGetUniformLocation(program, varname.c_str());
//	glUseProgram(program);
//	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m));
//	glUseProgram(0);
//}
//
//void updateUniformVector3fv(GLuint program, const std::string& varname, const glm::vec3& v) {
//	GLint location = glGetUniformLocation(program, varname.c_str());
//	glUseProgram(program);
//	glUniform3fv(location, 1, glm::value_ptr(v));
//	glUseProgram(0);
//}
//
//void updateUniformVector4fv(GLuint program, const std::string& varname, const glm::vec4& v) {
//	GLint location = glGetUniformLocation(program, varname.c_str());
//	glUseProgram(program);
//	glUniform4fv(location, 1, glm::value_ptr(v));
//	glUseProgram(0);
//}

GLuint prepareVAO(const std::vector<vec3>& vertex, const std::vector<vec3>& normal, const std::vector<GLuint>& index) {
	GLuint VAO, vbo_pos, vbo_norm, vbo_idx;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &vbo_pos);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_pos);
	glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(vec3), vertex.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	if (!normal.empty()) {
		glGenBuffers(1, &vbo_norm);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_norm);
		glBufferData(GL_ARRAY_BUFFER, normal.size() * sizeof(vec3), normal.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
		glEnableVertexAttribArray(1);
	}

	glGenBuffers(1, &vbo_idx);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_idx);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index.size() * sizeof(GLuint), index.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}

void loadShader(GLuint shader, std::string source) {
	const char* csource = source.c_str();
	glShaderSource(shader, 1, &csource, NULL);
	glCompileShader(shader);
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
		exit(4);
	}
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