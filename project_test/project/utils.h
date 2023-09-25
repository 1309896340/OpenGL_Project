#pragma once
#include "proj.h"


//void updateUniformMatrix4fv(GLuint program, const std::string& varname, const glm::mat4& m);
//void updateUniformVector3fv(GLuint program, const std::string& varname, const glm::vec3& v);
//void updateUniformVector4fv(GLuint program, const std::string& varname, const glm::vec4& v);

std::string readSource(std::string filename);
void loadShader(GLuint shader, std::string source);
GLuint loadProgram(std::string vertexSource, std::string fragmentSource);
GLuint loadProgramFromFile(std::string vertexSourcePath, std::string fragmentSourcePath);

std::ostream& operator<<(std::ostream& os, const glm::vec3& v);

GLuint prepareVAO(const std::vector<vec3> &vertex, const std::vector<vec3> &normal, const std::vector<GLuint> &index);
