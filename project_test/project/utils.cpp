#include "utils.h"

std::string readSource(std::string filename) {
	std::ifstream ifs(filename);
	std::stringstream ss;
	ss << ifs.rdbuf();
	return ss.str();
}

void updateUniformMatrix4fv(GLuint program, const std::string& varname, const glm::mat4& m) {
	GLint location = glGetUniformLocation(program, varname.c_str());
	glUseProgram(program);
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m));
	glUseProgram(0);
}

void updateUniformVector3fv(GLuint program, const std::string& varname, const glm::vec3& v) {
	GLint location = glGetUniformLocation(program, varname.c_str());
	glUseProgram(program);
	glUniform3fv(location, 1, glm::value_ptr(v));
	glUseProgram(0);
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
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		exit(4);
	}
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
	//glUseProgram(program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return program;
}
